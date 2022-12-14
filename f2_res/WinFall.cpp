/*
The MIT License (MIT)
Copyright ? 2022 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the ?Software?), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ?AS IS?, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"

#include "WinFall.h"

#include "fixes.h"
#include "memwrite.h"
#include "F_Windows.h"
#include "F_Mapper.h"
#include "configTools.h"
#include "GVX_multibyteText.h"

#include "DD7Draw.h"
#if defined(DX9_ENHANCED)
#include "Dx9Enhanced.h"
#elif !defined(VERSION_LEGACY)
#include "Dx9.h"
#endif


#if WINAPI2003//(_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
#define WM_MOUSEWHEEL                   0x020A

short GET_WHEEL_DELTA_WPARAM(WPARAM wParam) {
	return (short)(wParam >> 16);
};
short GET_KEYSTATE_WPARAM(WPARAM wParam) {
	return (short)(wParam);
};

#endif


struct POINTERstate {
	LONG x;
	LONG y;
	WORD flags;
};
POINTERstate pointerState;


HRESULT(__stdcall* pPal_GetEntries)(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries) = nullptr;


bool isGameMode = true;
bool isMapperExiting = false;


LONG SCR_WIDTH = 640;
LONG SCR_HEIGHT = 480;
DWORD scaler = 0;

DWORD COLOUR_BITS = 8;
DWORD REFRESH_RATE = 0;

RECT* SCRN_RECT;

LONG* pFALL_WIDTH = nullptr;
LONG* pFALL_HEIGHT = nullptr;

int graphicsMode = 0;
bool sfallGraphicsMode = false;
float sfallScaleX = 1.0f;
float sfallScaleY = 1.0f;

HWND* phWinMain = nullptr;
HWND hGameWnd = nullptr;
HINSTANCE* phInstance = nullptr;
HHOOK* keyboardHook = nullptr;

char winFallTitle[] = "Fallout II";

LONG* is_winActive = nullptr;
DWORD F_WIN_DESTROYER = 0;
DWORD F_WIN_ACTIVATOR = 0;

void* F_PROCESS_INPUT = nullptr;
void* F_PROCESS_MAP_MOUSE = nullptr;
void* F_SET_BLENDED_PAL = nullptr;

bool isMapperSizing = false;

bool isWindowed = false;

bool isAltMouseInput = false;

bool mainMenuExit = false;

bool isGrayScale = false;

DWORD keyCode_MiddleButton = 'b';

char winClassName[] = "GNW95 Class";
LRESULT CALLBACK WinProcParent(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


DWORD winStyleMain = WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
DWORD winStyle = WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

int* pPointerX = nullptr;
int* pPointerY = nullptr;
int* pPointerXOffset = nullptr;
int* pPointerYOffset = nullptr;

double* pPointerSensitivity = nullptr;
double PointerSensitivityOne = 1.0f;

LONG* pPointerXTemp = nullptr;
LONG* pPointerYTemp = nullptr;
LONG* pPointerWidth = nullptr;
LONG* pPointerHeight = nullptr;

int (*GraphicsLibSetup)(void) = nullptr;

F_LONG_FUNC_VOID ExitMessageBox;

void** lplpDinputMouseDevice = nullptr;


//_______________________
void ClipAltMouseCursor() {

	if (!isAltMouseInput)
		return;
	RECT rcClient;
	POINT p{ 0,0 };
	ClientToScreen(hGameWnd, &p);
	GetClientRect(hGameWnd, &rcClient);
	rcClient.left += p.x;
	rcClient.top += p.y;
	rcClient.right += p.x;
	rcClient.bottom += p.y;

	ClipCursor(&rcClient);
}


//_________________
bool MouseAcquire() {

	bool retVal = 0;
	__asm {
		mov edx, lplpDinputMouseDevice
		mov edx, dword ptr ds : [edx]
		test edx, edx
		je exitFail
		mov eax, edx
		push eax
		mov edx, dword ptr ds : [edx]
		call dword ptr ds : [edx + 0x1C]
		test eax, eax
		je exitPass
		cmp eax, 1
		jne exitFail
		exitPass :
		mov retVal, 1
		jmp exitFunc
		exitFail :
		mov retVal, 0
		exitFunc :
	}
	return retVal;
}


//___________________
bool MouseUnacquire() {

	bool retVal = 0;
	__asm {
		mov edx, lplpDinputMouseDevice
		mov edx, dword ptr ds : [edx]
		test edx, edx
		je exitFail
		mov eax, edx
		push eax
		mov edx, dword ptr ds : [edx]
		call dword ptr ds : [edx + 0x20]
		test eax, eax
		jne exitFail
		mov retVal, 1
		jmp exitFunc
		exitFail :
		mov retVal, 0
			exitFunc :
	}
	return retVal;
}


//________________________________________
bool MouseSetCooperativeLevel(DWORD flags) {

	if (isAltMouseInput)
		return true;

	bool retVal = 0;

	MouseUnacquire();
	__asm {
		mov edx, lplpDinputMouseDevice
		mov edx, dword ptr ds : [edx]
		test edx, edx
		je exitFail
		mov eax, edx
		push flags
		push hGameWnd
		push eax
		mov edx, dword ptr ds : [edx]
		call dword ptr ds : [edx + 0x34]
		test eax, eax
		jne exitFail
		mov retVal, 1
		jmp exitFunc
		exitFail :
		mov retVal, 0
		exitFunc :
	}
	MouseAcquire();
	return retVal;
}


//____________________________
void wmAppActivator(LONG flag) {

	__asm {
		mov eax, flag
		mov ebx, F_WIN_ACTIVATOR
		call ebx
	}
}


//_______________
int ResetWindow() {

	if (!hGameWnd)
		return -1;
	isMapperSizing = true;
	int oldWidth = SCR_WIDTH;
	int oldHeight = SCR_HEIGHT;
	int CLIENT_WIDTH = 0;
	int CLIENT_HEIGHT = 0;

	WindowVars_Load();
	CLIENT_WIDTH = SCR_WIDTH << scaler;
	CLIENT_HEIGHT = SCR_HEIGHT << scaler;

	if (isWindowed) {
		RestoreDDrawDisplayMode();

		int isWinFull = ConfigReadInt("MAIN", "WINDOWED_FULLSCREEN", 0);
		if (isWinFull)
			winStyle = WS_POPUP;
		else
			winStyle = winStyleMain;

		SetWindowLongPtr(hGameWnd, GWL_EXSTYLE, 0);
		SetWindowLongPtr(hGameWnd, GWL_STYLE, winStyle | WS_VISIBLE);
		WINDOWPLACEMENT winPlace;
		if (ConfigReadWinData("MAIN", "WIN_DATA", &winPlace)) {
			winPlace.showCmd = SW_SHOWNORMAL;
			winPlace.length = sizeof(WINDOWPLACEMENT);
			SetWindowPlacement(hGameWnd, &winPlace);
		}
		else {
			winPlace.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hGameWnd, &winPlace);
		}

		RECT rcWindow;
		GetClientRect(hGameWnd, &rcWindow);
		CLIENT_WIDTH = rcWindow.right - rcWindow.left;
		CLIENT_HEIGHT = rcWindow.bottom - rcWindow.top;
		SCR_WIDTH = CLIENT_WIDTH >> scaler;
		SCR_HEIGHT = CLIENT_HEIGHT >> scaler;

		if (SCR_WIDTH < 640 || SCR_HEIGHT < 480) {
			SCR_WIDTH = 640;
			SCR_HEIGHT = 480;
			CLIENT_WIDTH = SCR_WIDTH << scaler;
			CLIENT_HEIGHT = SCR_HEIGHT << scaler;

			rcWindow.left = 0;
			rcWindow.right = CLIENT_WIDTH;
			rcWindow.top = 0;
			rcWindow.bottom = CLIENT_HEIGHT;
		}
		AdjustWindowRectEx(&rcWindow, winStyle, false, 0);
		DWORD winWidth = rcWindow.right - rcWindow.left;
		DWORD winHeight = rcWindow.bottom - rcWindow.top;
		DWORD vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		DWORD vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

		if (vWidth < winWidth || vHeight < winHeight) {
			if (scaler > 0) {
				SCR_WIDTH = 640;
				CLIENT_WIDTH = SCR_WIDTH << scaler;
				SCR_HEIGHT = 480;
				CLIENT_HEIGHT = SCR_HEIGHT << scaler;

				rcWindow.left = 0;
				rcWindow.right = CLIENT_WIDTH;
				rcWindow.top = 0;
				rcWindow.bottom = CLIENT_HEIGHT;
				AdjustWindowRectEx(&rcWindow, winStyle, false, 0);
				winWidth = rcWindow.right - rcWindow.left;
				winHeight = rcWindow.bottom - rcWindow.top;
			}
			if (vWidth < winWidth || vHeight < winHeight) {
				scaler = 0;
				SCR_WIDTH = 640;
				CLIENT_WIDTH = SCR_WIDTH;
				SCR_HEIGHT = 480;
				CLIENT_HEIGHT = SCR_HEIGHT;

				rcWindow.left = 0;
				rcWindow.right = CLIENT_WIDTH;
				rcWindow.top = 0;
				rcWindow.bottom = CLIENT_HEIGHT;
				AdjustWindowRectEx(&rcWindow, winStyle, false, 0);
			}
		}
		winPlace.rcNormalPosition.right = winPlace.rcNormalPosition.left + (rcWindow.right - rcWindow.left);
		winPlace.rcNormalPosition.bottom = winPlace.rcNormalPosition.top + (rcWindow.bottom - rcWindow.top);

		if (isWinFull)
			winPlace.showCmd = SW_SHOWMAXIMIZED;
		SetWindowPlacement(hGameWnd, &winPlace);

		MouseSetCooperativeLevel(6);
		ReSetDisplayMode();
#if !defined (VERSION_LEGACY)
		ReSetDisplayModeDx();
#endif

		SCRN_RECT->right = SCR_WIDTH - 1;
		SCRN_RECT->bottom = SCR_HEIGHT - 1;
		*pFALL_WIDTH = SCR_WIDTH;
		*pFALL_HEIGHT = SCR_HEIGHT;
		F_ResizeWindows(oldWidth, oldHeight);
		SetWindowTitle(hGameWnd, "");
	}
	else {
		if (SCR_WIDTH < 640 || SCR_HEIGHT < 480) {
			CLIENT_WIDTH = 640;
			CLIENT_HEIGHT = 480;
			SCR_WIDTH = CLIENT_WIDTH;
			SCR_HEIGHT = CLIENT_HEIGHT;
			scaler = 0;
		}

		SetWindowLongPtr(hGameWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
		SetWindowLongPtr(hGameWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(hGameWnd, HWND_TOPMOST, 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT, SWP_FRAMECHANGED);//SWP_FRAMECHANGED|SWP_NOREDRAW);

		MouseSetCooperativeLevel(5);
		ReSetDisplayMode();
#if !defined (VERSION_LEGACY)
		ReSetDisplayModeDx();
#endif
		SCRN_RECT->right = SCR_WIDTH - 1;
		SCRN_RECT->bottom = SCR_HEIGHT - 1;
		*pFALL_WIDTH = SCR_WIDTH;
		*pFALL_HEIGHT = SCR_HEIGHT;
		F_ResizeWindows(oldWidth, oldHeight);
	}
	isMapperSizing = false;
	return 0;
}


//________________________
LONG CheckMainMenuButtons() {

	if (mainMenuExit)
		return 0x1B;

	int keyCode = CheckButtons();
	if (keyCode == 'o' || keyCode == 'O') {
		switch (SettingsMenu()) {
		case 1:
			F_HideMainMenu(0);
			OptionsWindow();
			F_ShowMainMenu(0);
			return -1;
		case 2: {
			F_HideMainMenu(0);
			WindowVars_Save();
			bool settingsChanged = HiResWindow();
			F_ShowMainMenu(0);
			if (settingsChanged && graphicsMode) {
				ResetWindow();
				UpdateWindow(hGameWnd);
			}
			return -1;
		}
		default:
			return -1;
			break;
		}
	}
	return keyCode;
}


//_________________________________________________
void __declspec(naked) check_mainmenu_buttons(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		call CheckMainMenuButtons

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}



//_____________________________________________
void SetWindowTitle(HWND hwnd, const char* msg) {

	char winText[260]{ 0 };
	if (sfallGraphicsMode) {
		RECT rcClient;
		GetClientRect(hGameWnd, &rcClient);
		int winWidth = rcClient.right - rcClient.left;
		int winHeight = rcClient.bottom - rcClient.top;
		if (winWidth != SCR_WIDTH || winHeight != SCR_HEIGHT)
			sprintf(winText, "%s  @%ix%i >> %ix%i   %s", winFallTitle, SCR_WIDTH, SCR_HEIGHT, winWidth, winHeight, msg);
		else
			sprintf(winText, "%s  @%ix%i   %s", winFallTitle, SCR_WIDTH, SCR_HEIGHT, msg);
	}
	else
		sprintf(winText, "%s  @%ix%ix%i   %s", winFallTitle, SCR_WIDTH, SCR_HEIGHT, 1 << scaler, msg);
	SendMessage(hwnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)winText);
}


//____________________
void WindowVars_Save() {

	if (isWindowed && !ConfigReadInt("MAIN", "WINDOWED_FULLSCREEN", 0)) {
		WINDOWPLACEMENT winPlacement{ 0 };
		winPlacement.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hGameWnd, &winPlacement);
		ConfigWriteWinData("MAIN", "WIN_DATA", &winPlacement);
	}

#if defined(DX9_ENHANCED)
	if (graphicsMode > 1) {
		LONG scaleGameInt = (LONG)ScaleX_game;//(LONG)(ScaleX_game*10);
		if (scaleGameInt < 1)
			scaleGameInt = 1;
		if (scaler && ScaleX_game >= 1.0f)
			scaleGameInt++;
		ConfigWriteInt("MAPS", "ZOOM_LEVEL", scaleGameInt);
	}
#endif
	print_mem_errors();
}


//________________________________________
void __declspec(naked) win_vars_save(void) {

	__asm {
		pushad
		call WindowVars_Save
		popad
		ret
	}
}


//____________________
void WindowVars_Load() {

#if defined(DX9_ENHANCED)
	graphicsMode = 2;
	COLOUR_BITS = ConfigReadInt("MAIN", "COLOUR_BITS", 8);
	if (COLOUR_BITS != 8 && COLOUR_BITS != 16 && COLOUR_BITS != 24 && COLOUR_BITS != 32)
		COLOUR_BITS = 8;

	scaler = ConfigReadInt("MAIN", "SCALE_2X", 0);
	if (scaler > 0)
		scaler = 1;
#else
	graphicsMode = ConfigReadInt("MAIN", "GRAPHICS_MODE", 0);

#if defined (VERSION_LEGACY)
	if (graphicsMode > 1)
		graphicsMode = 1;
#endif


	if (SfallReadInt("Graphics", "Mode", 0) || graphicsMode == 0) {
		COLOUR_BITS = 8;
		graphicsMode = 0;
		scaler = 0;
		sfallGraphicsMode = true;
	}
	else {
		COLOUR_BITS = ConfigReadInt("MAIN", "COLOUR_BITS", 8);
		if (COLOUR_BITS != 8 && COLOUR_BITS != 16 && COLOUR_BITS != 24 && COLOUR_BITS != 32)
			COLOUR_BITS = 8;

		scaler = ConfigReadInt("MAIN", "SCALE_2X", 0);
		if (scaler > 0)
			scaler = 1;
	}
#endif

	if (graphicsMode && ConfigReadInt("MAIN", "WINDOWED", 0)) 
		isWindowed = true;
	else {
		isWindowed = false;
		SCR_WIDTH = ConfigReadInt("MAIN", "SCR_WIDTH", 640) >> scaler;
		SCR_HEIGHT = ConfigReadInt("MAIN", "SCR_HEIGHT", 480) >> scaler;
		if (SCR_WIDTH < 640 || SCR_HEIGHT < 480) {
			SCR_WIDTH = 640;
			SCR_HEIGHT = 480;
			scaler = 0;
		}
	}

	REFRESH_RATE = ConfigReadInt("MAIN", "REFRESH_RATE", 0);

	if (ConfigReadInt("EFFECTS", "IS_GRAY_SCALE", 0))
		isGrayScale = true;
	else
		isGrayScale = false;

	SetFadeTime((double)ConfigReadInt("OTHER_SETTINGS", "FADE_TIME_MODIFIER", 60));
}


//________________
void wmDestroyer() {

	__asm {
		pushad
		xor eax, eax
		mov ebx, F_WIN_DESTROYER
		call ebx
		popad
	}
}


//_________________
int CheckMessages() {

	MSG msg;
	MsgWaitForMultipleObjectsEx(0, nullptr, 1, QS_ALLINPUT, 0);
	while (PeekMessage(&msg, nullptr, 0, 0, 0)) {
		if ((GetMessage(&msg, nullptr, 0, 0)) != 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
	}
	return 0;
}


//_____________________________________________________________
void CheckGameWindowSize(HWND hwnd, WINDOWPLACEMENT* pWinPlace) {

	WINDOWINFO windowInfo{ 0 };
	windowInfo.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(hwnd, &windowInfo);

	RECT winRect;
	CopyRect(&winRect, &windowInfo.rcClient);

	int CLIENT_WIDTH = winRect.right - winRect.left;
	int CLIENT_HEIGHT = winRect.bottom - winRect.top;

	if ((CLIENT_WIDTH >> scaler) < 640 || (CLIENT_HEIGHT >> scaler) < 480) {
		if ((CLIENT_WIDTH >> scaler) < 640) {
			SCR_WIDTH = 640;
			windowInfo.rcClient.right = windowInfo.rcClient.left + (SCR_WIDTH << scaler);
		}
		if ((CLIENT_HEIGHT >> scaler) < 480) {
			SCR_HEIGHT = 480;
			windowInfo.rcClient.bottom = windowInfo.rcClient.top + (SCR_HEIGHT << scaler);
		}
		AdjustWindowRectEx(&winRect, windowInfo.dwStyle, false, windowInfo.dwExStyle);
		if (winRect.left < 0) {
			winRect.right -= winRect.left;
			winRect.left = 0;
		}
		if (winRect.top < 0) {
			winRect.bottom -= winRect.top;
			winRect.top = 0;
		}
		CopyRect(&pWinPlace->rcNormalPosition, &winRect);
	}

	int vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	if (vWidth < winRect.right || vHeight < winRect.bottom) {
		if (scaler > 0) {
			SCR_WIDTH = 640;
			CLIENT_WIDTH = SCR_WIDTH << scaler;
			SCR_HEIGHT = 480;
			CLIENT_HEIGHT = SCR_HEIGHT << scaler;

			winRect.left = 0;
			winRect.right = CLIENT_WIDTH;
			winRect.top = 0;
			winRect.bottom = CLIENT_HEIGHT;
			AdjustWindowRectEx(&winRect, windowInfo.dwStyle, false, windowInfo.dwExStyle);
		}
		if (vWidth < winRect.right || vHeight < winRect.bottom) {
			scaler = 0;
			SCR_WIDTH = 640;
			CLIENT_WIDTH = SCR_WIDTH;
			SCR_HEIGHT = 480;
			CLIENT_HEIGHT = SCR_HEIGHT;

			winRect.left = 0;
			winRect.right = CLIENT_WIDTH;
			winRect.top = 0;
			winRect.bottom = CLIENT_HEIGHT;
			AdjustWindowRectEx(&winRect, windowInfo.dwStyle, false, windowInfo.dwExStyle);
		}
		if (winRect.left < 0) {
			winRect.right -= winRect.left;
			winRect.left = 0;
		}
		if (winRect.top < 0) {
			winRect.bottom -= winRect.top;
			winRect.top = 0;
		}
		CopyRect(&pWinPlace->rcNormalPosition, &winRect);
	}
	SetWindowPlacement(hwnd, pWinPlace);
}


//____________________
int CreateMainWindow() {

	if (hGameWnd)
		return 0;

	HINSTANCE hinstance = *phInstance;

	int CLIENT_WIDTH = SCR_WIDTH << scaler;
	int CLIENT_HEIGHT = SCR_HEIGHT << scaler;

	if (isWindowed) {
		*phWinMain = CreateWindowEx(0, winClassName, winFallTitle,
			winStyle | WS_VISIBLE,
			0, 0,
			640, 480, HWND_DESKTOP, nullptr, hinstance, nullptr);

		WINDOWPLACEMENT winPlace;

		if (ConfigReadWinData("MAIN", "WIN_DATA", &winPlace)) {
			if (winPlace.showCmd == SW_MINIMIZE || winPlace.showCmd == SW_SHOWMINIMIZED || winPlace.showCmd == SW_SHOWMINNOACTIVE || winPlace.showCmd == SW_HIDE)
				winPlace.showCmd = SW_SHOWNORMAL;
			SetWindowPlacement(*phWinMain, &winPlace);
		}
		else
			GetWindowPlacement(*phWinMain, &winPlace);

		CheckGameWindowSize(*phWinMain, &winPlace);

		if (ConfigReadInt("MAIN", "WINDOWED_FULLSCREEN", 0)) {
			winStyle = WS_POPUP;
			SetWindowLongPtr(*phWinMain, GWL_STYLE, winStyle | WS_VISIBLE);
			SetWindowPos(*phWinMain, 0, 0, 0, 0, 0, 0);
			ShowWindow(*phWinMain, SW_MAXIMIZE);
		}

	}
	else {
		*phWinMain = CreateWindowEx(WS_EX_TOPMOST, winClassName, winFallTitle,
			WS_POPUP | WS_VISIBLE,
			0, 0,
			CLIENT_WIDTH, CLIENT_HEIGHT, HWND_DESKTOP, nullptr, hinstance, nullptr);
	}

	if (*phWinMain == nullptr) {
		MessageBox(nullptr, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	RECT rcClient;
	GetClientRect(hGameWnd, &rcClient);
	SCR_WIDTH = (rcClient.right - rcClient.left) >> scaler;
	SCR_HEIGHT = (rcClient.bottom - rcClient.top) >> scaler;
	SCRN_RECT->left = 0;
	SCRN_RECT->top = 0;
	SCRN_RECT->right = SCR_WIDTH - 1;
	SCRN_RECT->bottom = SCR_HEIGHT - 1;
	*pFALL_WIDTH = SCR_WIDTH;
	*pFALL_HEIGHT = SCR_HEIGHT;

	if (pGvx)
		pGvx->LoadFontLib(*phWinMain);

	return 0;
}


//_________________________________________
void __declspec(naked) check_messages(void) {

	__asm {
		call CheckMessages
		add esp, 0x20
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//____________________________________________
void __declspec(naked) check_messages_CH(void) {

	__asm {
		call CheckMessages
		mov esp, ebp
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//____________________________________________
void __declspec(naked) check_combat_msgs(void) {

	__asm {
		pushad
		call CheckMessages
		popad
		call F_PROCESS_INPUT
		ret
	}
}


//_____________________________________________
void __declspec(naked) check_combat_msgs2(void) {

	__asm {
		pushad
		call CheckMessages
		popad
		call F_PROCESS_MAP_MOUSE
		ret
	}
}


//_______________________
int CheckMessagesNoWait() {

	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, 0)) {
		if ((GetMessage(&msg, nullptr, 0, 0)) != 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
	}
	return 0;
}


//__________________________________________
void __declspec(naked) check_fade_msgs(void) {

	__asm {
		pushad
		call CheckMessagesNoWait
		popad
		call F_SET_BLENDED_PAL
		ret
	}
}


//________________
int DisplaySetup() {

	if (CreateMainWindow())
		return -1;
	if (GraphicsLibSetup())
		return -1;

	UpdateWindow(*phWinMain);
	SetFocus(*phWinMain);
	return 0;
}


//________________________________________
void __declspec(naked) display_setup(void) {

	__asm {
		push ecx
		push esi
		push edi
		push ebp

		call DisplaySetup

		pop ebp
		pop edi
		pop esi
		pop ecx
		ret
	}
}


//___________________
int CheckClientRect() {

	//check if mouse within client rect.
	if (!isWindowed)
		return 0;

	POINT p{ 0,0 }, m{ 0,0 };
	GetCursorPos(&m);
	ClientToScreen(hGameWnd, &p);

	RECT rcClient;
	GetClientRect(hGameWnd, &rcClient);

	rcClient.left += p.x;
	rcClient.top += p.y;
	rcClient.right += p.x;
	rcClient.bottom += p.y;

	if (m.x < rcClient.left || m.x > rcClient.right)
		return 1;
	if (m.y < rcClient.top || m.y > rcClient.bottom)
		return 1;
	return 0;
}


//________________________
void SetRelativeMousePos() {

	int xPos = *pPointerX + *pPointerXOffset;
	int yPos = *pPointerY + *pPointerYOffset;

	if (isWindowed && !isAltMouseInput) {
		POINT p{ 0,0 }, m{ 0,0 };
		GetCursorPos(&m);
		ClientToScreen(hGameWnd, &p);
		
		xPos = (m.x - p.x) >> scaler;
		yPos = (m.y - p.y) >> scaler;
	}

	if (xPos < SCRN_RECT->left)
		xPos = SCRN_RECT->left;
	else if (xPos > SCRN_RECT->right)
		xPos = SCRN_RECT->right;
	*pPointerX = xPos - *pPointerXOffset;

	if (yPos < SCRN_RECT->top)
		yPos = SCRN_RECT->top;
	else if (yPos > SCRN_RECT->bottom)
		yPos = SCRN_RECT->bottom;
	*pPointerY = yPos - *pPointerYOffset;

	return;
}


//_____________________________________________
void __declspec(naked) set_relative_mouse(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push ebp
		call SetRelativeMousePos
		pop ebp
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//____________________________________
LONG GetWinAtPos(LONG xPos, LONG yPos) {

	LONG winNum = *numWindows - 1;
	if (winNum < 1)
		return pWinArray[winNum]->ref;

	while (winNum > 0) {
#if defined(DX9_ENHANCED)
		if (pWinArray[winNum]->ref == *pWinRef_GameArea) {
			if (xPos >= rcGameTrue.left && xPos <= rcGameTrue.right &&
				yPos >= rcGameTrue.top && yPos <= rcGameTrue.bottom)
				return pWinArray[winNum]->ref;
		}
#endif
		if (xPos >= pWinArray[winNum]->rect.left && xPos <= pWinArray[winNum]->rect.right &&
			yPos >= pWinArray[winNum]->rect.top && yPos <= pWinArray[winNum]->rect.bottom)
			return pWinArray[winNum]->ref;
		winNum--;
	}
	return pWinArray[winNum]->ref;
}


//_________________________________________
void __declspec(naked) get_win_at_pos(void) {
	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call GetWinAtPos
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//________________________________________
void GetMousePos(LONG* pXPos, LONG* pYPos) {

	*pXPos = *pPointerX + *pPointerXOffset;
	*pYPos = *pPointerY + *pPointerYOffset;

#if defined(DX9_ENHANCED)
	xPosTrue = *pXPos;
	yPosTrue = *pYPos;
	if (GetWinAtPos(*pXPos, *pYPos) == *pWinRef_GameArea) {
		WinStruct* win = GetWinStruct(*pWinRef_GameArea);
		if (!(win->flags & F_WIN_HIDDEN)) {
			*pXPos = (LONG)((float)*pXPos / ScaleX_game);
			*pYPos = (LONG)((float)*pYPos / ScaleX_game);
		}
	}
#endif
}


//________________________________________
void __declspec(naked) get_mouse_pos(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call GetMousePos
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//_____________________
LONG GetMouseWinAtPos() {

	LONG xPos = *pPointerX + *pPointerXOffset;
	LONG yPos = *pPointerY + *pPointerYOffset;
	return GetWinAtPos(xPos, yPos);
}


//_______________________________________________
void __declspec(naked) get_mouse_win_at_pos(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp
		call GetMouseWinAtPos
		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//___________________
void LockMouseInWin() {

	LONG xPos = *pPointerX + *pPointerXOffset;
	LONG yPos = *pPointerY + *pPointerYOffset;

#if defined(DX9_ENHANCED)
	if (GetWinAtPos((LONG)((float)xPos * ScaleX_game), (LONG)((float)yPos * ScaleX_game)) == *pWinRef_GameArea) {// && ScaleX_game>=1.0f) {
		WinStruct* win = GetWinStruct(*pWinRef_GameArea);

		if (xPos < win->rect.left)
			xPos = win->rect.left;
		else if (xPos > win->rect.right)
			xPos = win->rect.right;

		if (yPos < win->rect.top)
			yPos = win->rect.top;
		else if (yPos > win->rect.bottom)
			yPos = win->rect.bottom;

		xPos = (LONG)((float)xPos * ScaleX_game);
		yPos = (LONG)((float)yPos * ScaleX_game);

		*pPointerX = xPos - *pPointerXOffset;
		*pPointerY = yPos - *pPointerYOffset;
		return;

	}
#endif
	if (xPos < SCRN_RECT->left)
		xPos = SCRN_RECT->left;
	else if (xPos > SCRN_RECT->right)
		xPos = SCRN_RECT->right;

	if (yPos < SCRN_RECT->top)
		yPos = SCRN_RECT->top;
	else if (yPos > SCRN_RECT->bottom)
		yPos = SCRN_RECT->bottom;


	*pPointerX = xPos - *pPointerXOffset;
	*pPointerY = yPos - *pPointerYOffset;
}


//_____________________________________
void SetMousePos(LONG xPos, LONG yPos) {

	*pPointerX = xPos - *pPointerXOffset;
	*pPointerY = yPos - *pPointerYOffset;
	*pPointerXTemp = *pPointerX;
	*pPointerYTemp = *pPointerY;
	LockMouseInWin();

	if (isWindowed || isAltMouseInput) {
		POINT p{ 0,0 };
		ClientToScreen(hGameWnd, &p);

		LONG mouseX = 0, mouseY = 0;
		if (sfallGraphicsMode && isAltMouseInput) {
			mouseX = ((LONG)((float)*pPointerX * sfallScaleX) << scaler) + p.x;
			mouseY = ((LONG)((float)*pPointerY * sfallScaleY) << scaler) + p.y;
		}
		else {
			mouseX = (*pPointerX << scaler) + p.x;
			mouseY = (*pPointerY << scaler) + p.y;
		}
		if (*is_winActive && !isMapperSizing)
			SetCursorPos(mouseX, mouseY);
	}
}


//________________________________________
void __declspec(naked) set_mouse_pos(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call SetMousePos
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//_______________________________________
LONG GetPointerState(POINTERstate* touch) {

	touch->x = pointerState.x;
	touch->y = pointerState.y;
	touch->flags = pointerState.flags;

	pointerState.x = 0;
	pointerState.y = 0;

	return 1;
}


//____________________________________________
void __declspec(naked) get_pointer_state(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push ebp
		push eax
		call GetPointerState
		add esp, 0x4
		pop ebp
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//______________________________________
void SetWindowPointerPos(POINT* pointer) {

	POINT p{ 0,0 }, m{ 0,0 };

	ClientToScreen(hGameWnd, &p);
	m.x = pointer->x;
	m.x -= p.x;
	m.y = pointer->y;
	m.y -= p.y;

	if (sfallGraphicsMode && isAltMouseInput) {
		m.x = (LONG)((float)m.x / sfallScaleX);
		m.y = (LONG)((float)m.y / sfallScaleY);
	}

	pointerState.x = (m.x >> scaler) - *pPointerX - *pPointerXOffset;
	pointerState.y = (m.y >> scaler) - *pPointerY - *pPointerYOffset;
}


//___________________________________________________________________________________
LRESULT CALLBACK WinProcParent(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

	static bool is_cursor_hidden = true;

	switch (Message) {
	case WM_LBUTTONDOWN:
		pointerState.flags = pointerState.flags | 0x1;
		return 0;
	case WM_RBUTTONDOWN:
		pointerState.flags = pointerState.flags | 0x100;
		return 0;
	case WM_LBUTTONUP:
		pointerState.flags = pointerState.flags & 0xFFFFFFFE;
		return 0;
	case WM_RBUTTONUP:
		pointerState.flags = pointerState.flags & 0xFFFFFEFF;
		return 0;
	case WM_MBUTTONDOWN: {
		if (!isAltMouseInput)
			break;
		if (CheckMouseInImonitorRect(0, false))
			return 0;
		if (keyCode_MiddleButton)
			F_SendDosKey(keyCode_MiddleButton);
		return 0;
	}
	case WM_MOUSEMOVE: {
		POINT point;
		GetCursorPos(&point);
		SetWindowPointerPos(&point);
		return 0;
	}
	case WM_MOUSEWHEEL: {
		if (!isAltMouseInput)
			break;
		int fwKeys = GET_KEYSTATE_WPARAM(wParam);
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		bool scrollPage = false;
		if (fwKeys & MK_MBUTTON)
			scrollPage = true;
		if (CheckMouseInImonitorRect(zDelta, scrollPage))
			return 0;
#if defined(DX9_ENHANCED)
		if (CheckMouseInGameRectScroll(zDelta, scrollPage))
			return 0;
#endif
		int keyCode = 0;
		int numItems = CheckMouseInInvRects(zDelta, &keyCode);
		if (keyCode != -1) {
			if (!(fwKeys & MK_MBUTTON))//if middle button not down, scroll one item
				numItems = 1;
			for (int i = 0; i < numItems; i++)
				F_SendDosKey(keyCode);
		}
		return 0;
	}
	case WM_CREATE:
		hGameWnd = hwnd;
		SetWindowTitle(hwnd, "");
		return 0;
	case WM_WINDOWPOSCHANGING: {
		WINDOWPOS* winpos = (WINDOWPOS*)lParam;
		RECT rcClient{ 0,0, 640 << scaler,480 << scaler };
		AdjustWindowRectEx(&rcClient, winStyle, false, 0);
		int minWidth = rcClient.right - rcClient.left;
		int minHeight = rcClient.bottom - rcClient.top;

		if (winpos->cx < minWidth)
			winpos->cx = minWidth;
		if (winpos->cy < minHeight)
			winpos->cy = minHeight;
		return 0;
	}
	case WM_WINDOWPOSCHANGED: {
		if (!isWindowed)
			break;
		if (IsIconic(hwnd))
			break;
		DWORD old_SCR_WIDTH = SCR_WIDTH;
		DWORD old_SCR_HEIGHT = SCR_HEIGHT;
		RECT rcClient;
		GetClientRect(hGameWnd, &rcClient);
		SCR_WIDTH = (rcClient.right - rcClient.left) >> scaler;
		SCR_HEIGHT = (rcClient.bottom - rcClient.top) >> scaler;
		SCRN_RECT->right = SCR_WIDTH - 1;
		SCRN_RECT->bottom = SCR_HEIGHT - 1;
		*pFALL_WIDTH = SCR_WIDTH;
		*pFALL_HEIGHT = SCR_HEIGHT;
		bool isMapperSizing_temp = isMapperSizing;
		isMapperSizing = true;
		ReSizeDDrawSurface();
#if !defined (VERSION_LEGACY)
		ReSetDisplayModeDx();
#endif
		F_ResizeWindows(old_SCR_WIDTH, old_SCR_HEIGHT);
		isMapperSizing = isMapperSizing_temp;

		SetWindowTitle(hwnd, "");
		return 0;
	}
	case WM_SIZING: {

		if (!isWindowed)
			break;

		RECT* rcWindow = (RECT*)lParam;

		RECT rcClient{ 0,0, 640 << scaler,480 << scaler };

		AdjustWindowRectEx(&rcClient, winStyle, false, 0);
		int minWidth = rcClient.right - rcClient.left;
		int minHeight = rcClient.bottom - rcClient.top;

		if (rcWindow->right - rcWindow->left < minWidth)
			rcWindow->right = rcWindow->left + minWidth;
		if (rcWindow->bottom - rcWindow->top < minHeight)
			rcWindow->bottom = rcWindow->top + minHeight;
		return true;
	}
	case WM_ENTERMENULOOP://allows system menu keys to fuction
		*is_winActive = 0;
		wmAppActivator(*is_winActive);
		break;
	case WM_EXITMENULOOP:
		*is_winActive = 1;
		wmAppActivator(*is_winActive);
		break;
	case WM_SIZE: {
		switch ((wParam)) {
		case SIZE_MINIMIZED:
			//return 0;
			break;
		case SIZE_RESTORED:
			break;
		case SIZE_MAXIMIZED:
			break;
		default:
			break;
		}
		if (sfallGraphicsMode && isAltMouseInput) {
			LONG winWidth = (LOWORD(lParam));
			LONG winHeight = (HIWORD(lParam));
			sfallScaleX = (float)winWidth / (float)SCR_WIDTH;
			sfallScaleY = (float)winHeight / (float)SCR_HEIGHT;
			SetWindowTitle(hwnd, "");
			break;
		}
		if (!isWindowed)
			break;
		if (IsIconic(hwnd))
			break;

		DWORD old_SCR_WIDTH = SCR_WIDTH;
		DWORD old_SCR_HEIGHT = SCR_HEIGHT;

		SCR_WIDTH = (LOWORD(lParam)) >> scaler;
		SCR_HEIGHT = (HIWORD(lParam)) >> scaler;
		SCRN_RECT->right = SCR_WIDTH - 1;
		SCRN_RECT->bottom = SCR_HEIGHT - 1;
		*pFALL_WIDTH = SCR_WIDTH;
		*pFALL_HEIGHT = SCR_HEIGHT;
		bool isMapperSizing_temp = isMapperSizing;
		isMapperSizing = true;
		ReSizeDDrawSurface();
#if !defined (VERSION_LEGACY)
		ReSetDisplayModeDx();
#endif
		F_ResizeWindows(old_SCR_WIDTH, old_SCR_HEIGHT);
		isMapperSizing = isMapperSizing_temp;

		SetWindowTitle(hwnd, "");
		return 0;
	}
	case WM_CLOSE: {
		if (IsMainMenuVisible())
			mainMenuExit = true;
		else {
			if (IsIconic(hwnd)) {
				if (SetForegroundWindow(hwnd))
					ShowWindow(hwnd, SW_RESTORE);
			}
			static bool isClosing = false; // to prevent ExitMesageBox loading multiple times.
			if (!isClosing) {
				isClosing = true;
				if (ExitMessageBox() == 1)
					mainMenuExit = true;
				else isClosing = false;
			}
		}
		return 0;
	}
	case WM_DISPLAYCHANGE:
		if (sfallGraphicsMode && isAltMouseInput) {
			LONG winWidth = (LOWORD(lParam));
			LONG winHeight = (HIWORD(lParam));
			sfallScaleX = (float)winWidth / (float)SCR_WIDTH;
			sfallScaleY = (float)winHeight / (float)SCR_HEIGHT;
			SetWindowTitle(hwnd, "");
			break;
		}
		break;
	case WM_COMMAND:
		break;
	case WM_SYSCOMMAND:
		switch ((wParam & 0xFFF0)) {
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		default:
			break;
		}
		break;
	case WM_SETCURSOR: {
		DWORD currentWinStyle = GetWindowLongPtr(hGameWnd, GWL_STYLE);
		if (!(currentWinStyle & (winStyleMain | WS_OVERLAPPED | WS_BORDER))) {
			ClipAltMouseCursor();
			break;
		}
		if (!isWindowed && !(isAltMouseInput && graphicsMode == 0))
			break;
		WORD ht = LOWORD(lParam);
		if (HTCLIENT == ht) {
			ClipCursor(nullptr);

			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			if (!CheckClientRect()) {
				if (!is_cursor_hidden) {
					is_cursor_hidden = true;
					ShowCursor(false);
				}
			}
			else {
				if (is_cursor_hidden) {
					is_cursor_hidden = false;
					ShowCursor(true);
				}
			}
		}
		else {
			if (is_cursor_hidden) {
				is_cursor_hidden = false;
				ShowCursor(true);
			}
		}
	}
			break;
	case WM_ACTIVATEAPP:
		*is_winActive = wParam;
		wmAppActivator(*is_winActive);
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_DESTROY:
		wmDestroyer();
		return 1;
	case WM_PAINT:
		if (isMapperSizing)
			break;
		if (GetUpdateRect(hwnd, nullptr, 0))
			FDrawWinRect(SCRN_RECT);
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, Message, wParam, lParam);
}


//_______________________________________________________________________
LRESULT CALLBACK KeyboardProcMain(int code, WPARAM wParam, LPARAM lParam) {

	if (code < 0)
		return CallNextHookEx(*keyboardHook, code, wParam, lParam);

	switch (wParam) {
	case VK_DELETE:
		if (!(lParam & KF_MENUMODE))
			break;
	case VK_ESCAPE:
		if (GetAsyncKeyState(VK_CONTROL) & 0x80000000)
			return 0;
		break;
	case VK_TAB:
		if ((lParam & KF_MENUMODE))
			return 0;
		break;
	case VK_F4:
		if ((lParam & KF_MENUMODE) && isWindowed)
			return 0;
		break;
	case VK_SPACE:
		if ((lParam & KF_MENUMODE) && isWindowed) {
			return 0;
		}
		break;

	case VK_NUMLOCK:
	case VK_CAPITAL:
	case VK_SCROLL:
		break;
	default:
		return 1;
		break;
	}

	return CallNextHookEx(*keyboardHook, code, wParam, lParam);
}


//____________________
void WinFallFixes_CH() {

	SCRN_RECT = (RECT*)0x6BCF5C;

	phWinMain = (HWND*)0x52E210;

	phInstance = (HINSTANCE*)0x52E214;

	is_winActive = (LONG*)0x52E220;

	F_WIN_DESTROYER = 0x4F1EFE;

	F_WIN_ACTIVATOR = 0x4CAC7B;


	//COLOR MODE 8 TO 16 BIT
	MemWrite32(0x4CCAC1, 0x8, COLOUR_BITS);
	//default display mode dimensions
	MemWrite32(0x4CC630, 640, SCR_WIDTH);
	MemWrite32(0x4CC62B, 480, SCR_HEIGHT);

	MemWrite32(0x4E6FE6, 0x4E730E, (DWORD)&WinProcParent);

	if (ConfigReadInt("OTHER_SETTINGS", "CPU_USAGE_FIX", 0)) {
		MemWrite8(0x4CAF7B, 0x6A, 0xE9);
		FuncWrite32(0x4CAF7C, 0x6A006A00, (DWORD)&check_messages_CH);
	}

	FuncWrite32(0x480F5B, 0x048361, (DWORD)&check_mainmenu_buttons);

	//fix mouse window lock
	if (isWindowed)
		MemWrite8(0x4EA01A, 0x05, 0x06);

	lplpDinputMouseDevice = (void**)0x52E24C;

	pPointerSensitivity = (double*)FixAddress(0x52E080);

	pPointerX = (int*)0x6BCD14;
	pPointerY = (int*)0x6BCD10;
	pPointerXOffset = (int*)0x6BCD3C;
	pPointerYOffset = (int*)0x6BCD38;
	pPointerXTemp = (LONG*)0x6BCD00;
	pPointerYTemp = (LONG*)0x6BCD08;
	pPointerWidth = (LONG*)0x6BCD30;
	pPointerHeight = (LONG*)0x6BCD04;

	FuncWrite32(0x4CBD99, 0x02D8, (DWORD)&set_relative_mouse);

	ExitMessageBox = (LONG(__cdecl*)(void))0x443928;

	pFALL_WIDTH = (LONG*)0x6832FC;
	pFALL_HEIGHT = (LONG*)0x683308;

	MemWrite16(0x4B7E9E, 0x048B, 0x9090);
	MemWrite8(0x4B7EA0, 0xDD, 0xA1);
	MemWrite32(0x4B7EA1, 0x52DB10, (DWORD)&SCR_HEIGHT);

	MemWrite8(0x4B7EB2, 0x8B, 0x90);
	MemWrite16(0x4B7EB3, 0xDD0C, 0x0D8B);
	MemWrite32(0x4B7EB5, 0x52DB0C, (DWORD)&SCR_WIDTH);

	keyboardHook = (HHOOK*)0x6BCCC8;
	MemWrite32(0x4CAD02, 0x4CADA4, (DWORD)&KeyboardProcMain);

	MemWrite8(0x4DE026, 0x53, 0xE9);
	FuncWrite32(0x4DE027, 0x55575651, (DWORD)&get_mouse_win_at_pos);

	MemWrite8(0x4CBFDC, 0x53, 0xE9);
	FuncWrite32(0x4CBFDD, 0x55575651, (DWORD)&get_mouse_pos);

	MemWrite8(0x4CC021, 0x53, 0xE9);
	FuncWrite32(0x4CC022, 0x55575651, (DWORD)&set_mouse_pos);

	if (!SfallReadInt("Graphics", "Mode", 0) && ConfigReadInt("INPUT", "ALT_MOUSE_INPUT", 0)) {
		isAltMouseInput = true;
		MemWrite8(0x4E9BB8, 0xE8, 0xB8);
		MemWrite32(0x4E9BB9, 0x041B, 0x1);

		//aquire mouse func
		MemWrite16(0x4E9C6B, 0x2874, 0x1FEB);

		//unaquire mouse
		MemWrite16(0x4E9CC7, 0x1D74, 0x14EB);

		MemWrite8(0x4E9CF9, 0x68, 0xE9);
		FuncWrite32(0x4E9CFA, 0x48, (DWORD)&get_pointer_state);

		MemWrite32(0x4CBA4C, 0x52E080, (DWORD)&PointerSensitivityOne);
		MemWrite32(0x4CBA5D, 0x52E080, (DWORD)&PointerSensitivityOne);

		MemWrite32(0x4CC422, 0x52E080, (DWORD)&PointerSensitivityOne);
		MemWrite32(0x4CC42A, 0x52E084, (DWORD)&PointerSensitivityOne + 0x4);
	}


	FuncReplace32(0x44256E, 0x0707AE, (DWORD)&win_vars_save);

	//to prevent windows "NOT RESPONDING" error. add messege checking to loops
	if (isAltMouseInput || ConfigReadInt("INPUT", "EXTRA_WIN_MSG_CHECKS", 0)) {
		F_PROCESS_INPUT = (void*)0x4C9379;
		F_PROCESS_MAP_MOUSE = (void*)0x44ADD4;
		F_SET_BLENDED_PAL = (void*)0x4C7681;
		//npc combat actions
		FuncReplace32(0x4225A6, 0x0A6DCF, (DWORD)&check_combat_msgs);
		//combat open
		FuncReplace32(0x45F14E, 0xFFFEBC82, (DWORD)&check_combat_msgs2);
		//combat close
		FuncReplace32(0x45F2A7, 0xFFFEBB29, (DWORD)&check_combat_msgs2);

		//end/begin turn
		FuncReplace32(0x4605B1, 0xFFFEA81F, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x4605FF, 0xFFFEA7D1, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x460655, 0xFFFEA77B, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x4606AE, 0xFFFEA722, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x46071E, 0xFFFEA6B2, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x4607FA, 0xFFFEA5D6, (DWORD)&check_combat_msgs2);

		FuncReplace32(0x4C73ED, 0x0290, (DWORD)&check_fade_msgs);
	}



	if (ConfigReadInt("MAIN", "GRAPHICS_MODE", 0) == 0)
		return;
	if (SfallReadInt("Graphics", "Mode", 0))
		return;

	MemWrite8(0x4CC9C1, 0x51, 0xE9);
	FuncWrite32(0x4CC9C2, 0x89555756, (DWORD)&DisplaySetup);
}



//___________________
void WinFallFixes_MULTI() {

	SCRN_RECT = (RECT*)FixAddress(0x6AC9F0);

	phWinMain = (HWND*)FixAddress(0x51E434);

	phInstance = (HINSTANCE*)FixAddress(0x51E438);

	is_winActive = (LONG*)FixAddress(0x51E444);

	F_WIN_DESTROYER = FixAddress(0x4E660F);

	F_WIN_ACTIVATOR = FixAddress(0x4C9BB4);

	///COLOR MODE 8 TO 16 BIT
	MemWrite32(0x4CAECE, 0x8, COLOUR_BITS);

	///default display mode dimensions
	MemWrite32(0x4CAD6B, 640, SCR_WIDTH);
	MemWrite32(0x4CAD66, 480, SCR_HEIGHT);

	MemWrite32(0x4DE802, FixAddress(0x4DE9FC), (DWORD)&WinProcParent);//(DWORD)&ParentWinProc);

	if (ConfigReadInt("OTHER_SETTINGS", "CPU_USAGE_FIX", 0)) {
		MemWrite8(0x4C9DA9, 0x53, 0xE9);
		FuncWrite32(0x4C9DAA, 0x8D535353, (DWORD)&check_messages);
	}

	FuncReplace32(0x481B2B, 0x047049, (DWORD)&check_mainmenu_buttons);



	//fix mouse window lock
	if (isWindowed)
		MemWrite8(0x4E072C, 0x05, 0x06);

	lplpDinputMouseDevice = (void**)FixAddress(0x51E45C);
	pPointerSensitivity = (double*)FixAddress(0x51E2A0);

	pPointerX = (int*)FixAddress(0x6AC7A8);
	pPointerY = (int*)FixAddress(0x6AC7A4);
	pPointerXOffset = (int*)FixAddress(0x6AC7D0);
	pPointerYOffset = (int*)FixAddress(0x6AC7CC);
	pPointerXTemp = (LONG*)FixAddress(0x6AC794);
	pPointerYTemp = (LONG*)FixAddress(0x6AC79C);
	pPointerWidth = (LONG*)FixAddress(0x6AC7C4);
	pPointerHeight = (LONG*)FixAddress(0x6AC798);

	FuncReplace32(0x4CA89B, 0x0199, (DWORD)&set_relative_mouse);

	ExitMessageBox = (LONG(__cdecl*)(void))FixAddress(0x4440B8);

	pFALL_WIDTH = (LONG*)FixAddress(0x672D7C);
	pFALL_HEIGHT = (LONG*)FixAddress(0x672D88);

	MemWrite16(0x4B91DA, 0x048B, 0x9090);
	MemWrite8(0x4B91DC, 0xDD, 0xA1);
	MemWrite32(0x4B91DD, FixAddress(0x51DD20), (DWORD)&SCR_HEIGHT);

	MemWrite8(0x4B91EE, 0x8B, 0x90);
	MemWrite16(0x4B91EF, 0xDD0C, 0x0D8B);
	MemWrite32(0x4B91F1, FixAddress(0x51DD1C), (DWORD)&SCR_WIDTH);

	keyboardHook = (HHOOK*)FixAddress(0x6AC758);
	MemWrite32(0x4C9BD9, FixAddress(0x4C9C4C), (DWORD)&KeyboardProcMain);

	MemWrite8(0x4D78CC, 0x53, 0xE9);
	FuncWrite32(0x4D78CD, 0xC1895651, (DWORD)&get_mouse_win_at_pos);

	MemWrite8(0x4CA9DC, 0x53, 0xE9);
	FuncWrite32(0x4CA9DD, 0xC3895651, (DWORD)&get_mouse_pos);

	MemWrite8(0x4CAA04, 0x53, 0x90);
	MemWrite16(0x4CAA05, 0x1D8B, 0xE990);
	FuncWrite32(0x4CAA07, FixAddress(0x6AC7D0), (DWORD)&set_mouse_pos);

	if (ConfigReadInt("INPUT", "ALT_MOUSE_INPUT", 0)) {
		isAltMouseInput = true;
		MemWrite8(0x4E042F, 0xE8, 0xB8);
		MemWrite32(0x4E0430, 0x02D8, 0x1);

		//aquire mouse func
		MemWrite16(0x4E04F2, 0x1974, 0x11EB);
		//unaquire mouse
		MemWrite16(0x4E051E, 0x1474, 0x0CEB);

		MemWrite8(0x4E053C, 0x53, 0xE9);
		FuncWrite32(0x4E053D, 0xEC835251, (DWORD)&get_pointer_state);

		MemWrite32(0x4CA60C, FixAddress(0x51E2A0), (DWORD)&PointerSensitivityOne);
		MemWrite32(0x4CAC6E, FixAddress(0x51E2A0), (DWORD)&PointerSensitivityOne);
	}

	FuncReplace32(0x442CFE, 0x07148E, (DWORD)&win_vars_save);


	//to prevent windows "NOT RESPONDING" error. add messege checking to loops
	if (isAltMouseInput || ConfigReadInt("INPUT", "EXTRA_WIN_MSG_CHECKS", 0)) {
		F_PROCESS_INPUT = (void*)FixAddress(0x4C8BDC);
		F_PROCESS_MAP_MOUSE = (void*)FixAddress(0x44B684);
		F_SET_BLENDED_PAL = (void*)FixAddress(0x4C73E4);
		//npc combat actions
		FuncReplace32(0x4227E6, 0x0A63F2, (DWORD)&check_combat_msgs);
		//combat open
		FuncReplace32(0x45FA4E, 0xFFFEBC32, (DWORD)&check_combat_msgs2);
		//combat close
		FuncReplace32(0x45FBA7, 0xFFFEBAD9, (DWORD)&check_combat_msgs2);

		//end/begin turn
		FuncReplace32(0x460EB1, 0xFFFEA7CF, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x460EFF, 0xFFFEA781, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x460F55, 0xFFFEA72B, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x460FAE, 0xFFFEA6D2, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x46101E, 0xFFFEA662, (DWORD)&check_combat_msgs2);
		FuncReplace32(0x4610FA, 0xFFFEA586, (DWORD)&check_combat_msgs2);

		FuncReplace32(0x4C73B2, 0x2E, (DWORD)&check_fade_msgs);
	}

#if !defined(DX9_ENHANCED)
	if (ConfigReadInt("MAIN", "GRAPHICS_MODE", 0) == 0)
		return;
	if (SfallReadInt("Graphics", "Mode", 0))
		return;
#endif

	MemWrite8(0x4CAE1C, 0x51, 0xE9);
	FuncWrite32(0x4CAE1D, 0x89555756, (DWORD)&display_setup);
}


//___________________________
void WinFallFixes(int region) {

	WindowVars_Load();

	if (region == 4)
		WinFallFixes_CH();
	else
		WinFallFixes_MULTI();

	GVXFixes(region);
}
