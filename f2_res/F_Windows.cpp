/*
The MIT License (MIT)
Copyright © 2022 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"

#include "F_Windows.h"
#include "memwrite.h"
#include "fixes.h"
#include "configTools.h"

#if defined(DX9_ENHANCED)
#include "Dx9Enhanced.h"
#endif

WinStruct** pWinArray = nullptr;
LONG* numWindows = nullptr;

BYTE** lpWinMainBuff = nullptr;

LONG* pWinRef_Char = nullptr;
LONG* pWinRef_Bio = nullptr;
//LONG *Win_Credits;
//LONG *Win_Death;
LONG* pWinRef_DialogMain = nullptr;

LONG* pWinRef_DialogNpcText = nullptr;
LONG* pWinRef_DialogPcText = nullptr;
LONG* pWinRef_DialogBaseSub = nullptr;

LONG* pWinRef_Inventory = nullptr;
LONG* pWinRef_EndSlides = nullptr;
LONG* pWinRef_Iface = nullptr;
LONG* pWinRef_NotifyBar = nullptr;
LONG* pWinRef_Skills = nullptr;
LONG* pWinRef_LoadSave = nullptr;
LONG* pWinRef_MainMenu = nullptr;
LONG* pWinRef_Movies;
LONG* pWinRef_Options = nullptr;
LONG* pWinRef_Pipboy = nullptr;
//LONG *Win_Splash;
LONG* pWinRef_GameArea = nullptr;
LONG* pWinRef_WorldMap = nullptr;


void* CREATE_WINDOW_OBJECT = nullptr;
void* DESTROY_WINDOW_OBJECT = nullptr;
void* DRAW_WINDOW_OBJECT = nullptr;

void* SHOW_WINDOW_OBJECT = nullptr;
void* HIDE_WINDOW_OBJECT = nullptr;

void* GET_WIN_STRUCT = nullptr;

void* WIN_PRINT_TEXT = nullptr;

void* DRAW_WIN_RECT = nullptr;

void* GET_WIN_AT_POS = nullptr;


void* SET_BUTTON = nullptr;
void* SET_BUTTON_FUNCTIONS = nullptr;
void* SET_BUTTON_SOUNDS = nullptr;

void* CHECK_BUTTONS = nullptr;
void* GET_BUTTON_STRUCT = nullptr;

void* SET_MOUSE_PIC = nullptr;

void* PLAY_ACM = nullptr;

void* MESSAGE_BOX = nullptr;

F_VOID_FUNC_VOID F_ShowMouse;
F_VOID_FUNC_VOID F_HideMouse;
F_DWORD_FUNC_VOID F_GetMousePicNum;

LONG* pIS_MOUSE_HIDDEN = nullptr;
DWORD* MOUSE_FLAGS = nullptr;
void* GET_MOUSE_POS = nullptr;
void* F_CHECK_MOUSE_IN_RECT = nullptr;
void* GET_MOUSE_RECT = nullptr;
void* F_SET_MOUSE_POS = nullptr;

void* F_SET_MOUSE_FRM = nullptr;

void* ALLOCATE_MEM = nullptr;
void* REALLOCATE_MEM = nullptr;
void* DEALLOCATE_MEM = nullptr;

void* AFF_FONT_ADDRESS = nullptr;

void* F_LOAD_PALETTE = nullptr;

void* F_SET_PALETTE = nullptr;

void* F_FADE_TO_PALETTE = nullptr;

BYTE* pBLACK_PAL = nullptr;
BYTE* pMAIN_PAL = nullptr;
BYTE* pCURRENT_PAL = nullptr;

void* DRAW_SCENE_1 = nullptr;

void* DRAW_TO_SCRN = nullptr;

void* F_FADE_TRANS_SETUP = nullptr;
void* F_FADE_TRANS_SETUP2 = nullptr;

bool isFadeReset = false;
bool isFadeResetAlways = false;

LONG* P_GAME_EXIT_FLAG = nullptr;

F_VOID_FUNC_VOID F_ToggleMouseHex = nullptr;

void* F_SEND_DOS_KEY = nullptr;

void* F_SET_MOUSE_PIC = nullptr;


//_____________________________________________________________________________________________________________
LONG F_SetMousePic(BYTE* fBuff, LONG subWidth, LONG subHeight, LONG fWidth, LONG fX, LONG fY, DWORD maskColour) {

	LONG retVal = 0; //0=pass, -1=fail
	__asm {
		push maskColour
		push fY
		push fX
		mov ecx, fWidth
		mov ebx, subHeight
		mov edx, subWidth
		mov eax, fBuff
		call F_SET_MOUSE_PIC
		add esp, 0xC
		mov retVal, eax
	}
	return retVal;
}


//______________________________________
void F_SetMousePos(LONG xPos, LONG yPos) {

	__asm {
		mov edx, yPos
		mov eax, xPos
		call F_SET_MOUSE_POS
	}

}


//________________________________
void F_GetMouseRect(RECT* rcMouse) {

	__asm {
		mov eax, rcMouse
		call GET_MOUSE_RECT
	}
}


//___________________________
void F_SendDosKey(DWORD key) {

	__asm {
		mov eax, key
		call F_SEND_DOS_KEY
	}

}


//_______________
bool IsExitGame() {

	if (*P_GAME_EXIT_FLAG)
		return true;
	else
		return false;
}


//____________________________________________
void __declspec(naked) fade_trans_setup2(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		sub esp, 0x8
		jmp F_FADE_TRANS_SETUP2
	}
}


//______________________________________
void __declspec(naked) fade_to_pal(void) {

	__asm {
		cmp isFadeResetAlways, 0
		jne fadeReset
		cmp isFadeReset, 0
		je endFunc
		fadeReset :

		pushad
		call fade_trans_setup2
		popad

		mov isFadeReset, 0
		endFunc:

		pop ebx //ret address
		push ecx
		push edx
		push esi
		push edi
		push ebp
		jmp ebx  //ret address
	}
}


//______________________
bool isIfaceBarVisible() {

	if (*pWinRef_Iface == -1)
		return false;
	WinStruct* ifaceWin = GetWinStruct(*pWinRef_Iface);
	if (!ifaceWin || ifaceWin->flags & F_WIN_HIDDEN)
		return false;
	return true;
}


//____________________________________________________________________
bool AdjustWinPosToGame(DWORD width, DWORD height, LONG* px, LONG* py) {

	bool isInGame = false;
	if (*pWinRef_GameArea == -1 || !isIfaceBarVisible()) {
		*px = (SCR_WIDTH >> 1) - (width >> 1);
		*py = (SCR_HEIGHT >> 1) - (height >> 1);
		isInGame = false;
	}
	else {
		WinStruct* gameWin = GetWinStruct(*pWinRef_GameArea);
		if (gameWin) {
#if defined(DX9_ENHANCED)
			*px = (rcGameTrue.right - rcGameTrue.left) / 2 - (width >> 1);
			*py = (rcGameTrue.bottom - rcGameTrue.top) / 2 - (height >> 1);
#else
			* px = (gameWin->width >> 1) - (width >> 1);
			*py = (gameWin->height >> 1) - (height >> 1);
#endif
			isInGame = true;
		}
		else {
			*px = (SCR_WIDTH >> 1) - (width >> 1);
			*py = (SCR_HEIGHT >> 1) - (height >> 1);
			isInGame = false;
		}
	}
	return isInGame;
}


//________________________________
LONG F_GetWinAtPos(LONG x, LONG y) {

	LONG winRef = 0;
	__asm {
		mov edx, y
		mov eax, x
		call GET_WIN_AT_POS
		mov winRef, eax
	}
	return winRef;
}


//_______________________________________________________
void F_DrawToScrn(WinStruct* win, RECT* rect, BYTE* buff) {

	__asm {
		mov ebx, buff
		mov edx, rect
		mov eax, win
		call DRAW_TO_SCRN
	}
}


//__________________________________________
void DrawWindowArea(LONG winRef, RECT* rect) {

	WinStruct* win = GetWinStruct(winRef);
	RECT newRect;
	CopyRect(&newRect, rect);
	newRect.left += win->rect.left;
	newRect.top += win->rect.top;
	newRect.right += win->rect.left;
	newRect.bottom += win->rect.top;
	F_DrawToScrn(win, &newRect, nullptr);
}


//______________________________________________
void DrawWindowArea2(WinStruct* win, RECT* rect) {

	RECT newRect;
	CopyRect(&newRect, rect);
	newRect.left += win->rect.left;
	newRect.top += win->rect.top;
	newRect.right += win->rect.left;
	newRect.bottom += win->rect.top;
	F_DrawToScrn(win, &newRect, nullptr);
}


//___________________________
void FDrawWinRect(RECT* rect) {

	__asm {
		mov eax, rect
		CALL DRAW_WIN_RECT
	}
}


//___________________________________________________
void FDrawWinRectRelative(WinStruct* win, RECT* rect) {

	RECT newRect;
	CopyRect(&newRect, rect);
	newRect.left += win->rect.left;
	newRect.top += win->rect.top;
	newRect.right += win->rect.left;
	newRect.bottom += win->rect.top;
	FDrawWinRect(&newRect);
}


//_____________________________________________________________________________________________
void Fix8BitColours(BYTE* buff, UINT subWidth, UINT subHeight, UINT buffWidth, BYTE darkColour) {

	if (COLOUR_BITS != 8)
		return;

	for (UINT y = 0; y < subHeight; y++) {
		for (UINT x = 0; x < subWidth; x++) {
			if (buff[x] > 254)
				buff[x] = darkColour;//colour;
		}
		buff += buffWidth;
	}
}


//_________________________________
BYTE FindDarkPalRef(BYTE* pPalette) {

	int palSize = 256;

	if (COLOUR_BITS == 8)
		palSize--;

	BYTE r = 0, g = 0, b = 0;
	BYTE cMax = 63;
	BYTE cMaxT = 63;
	BYTE cdiff = 63;
	BYTE cdiffT = 63;
	int palOff = 0;
	BYTE darkRef = 0;

	for (int ref = 0; ref < palSize; ref++) {
		r = pPalette[palOff + 2];
		g = pPalette[palOff + 1];
		b = pPalette[palOff + 0];

		//find max colour
		cMax = r;
		if (g > cMax) cMax = g;
		if (b > cMax) cMax = b;
		//find greatest diff between colours
		cdiff = abs(r - g);
		if (abs(r - b) > cdiff)
			cdiff = abs(r - b);
		if (abs(g - b) > cdiff)
			cdiff = abs(g - b);

		if (cMax <= cMaxT && cdiff <= cdiffT) {
			cdiffT = cdiff;
			cMaxT = cMax;
			darkRef = ref;
		}
		palOff += 3;
	}
	return darkRef;
}


//____________________________________
LONG FLoadPalette(const char* FileName) {

	int retVal = 0;
	__asm {
		mov eax, FileName
		CALL F_LOAD_PALETTE
		mov retVal, eax
	}
	return retVal;
}


//_________________________
void FSetPalette(BYTE* pal) {

	__asm {
		mov eax, pal
		CALL F_SET_PALETTE
	}

}


//____________________________
void FFadeToPalette(BYTE* pal) {

	__asm {
		mov eax, pal
		CALL F_FADE_TO_PALETTE
	}

}


//___________________
bool IsMouseHidden() {

	if (*pIS_MOUSE_HIDDEN)
		return true;
	return false;
}


//___________________
DWORD GetMouseFlags() {

	return *MOUSE_FLAGS;
}


//____________________________
bool IsMouseInRect(RECT* rect) {

	LONG x = 0, y = 0;
	F_GetMousePos(&x, &y);

	if (x < rect->left || x > rect->right)
		return false;
	if (y < rect->top || y > rect->bottom)
		return false;

	return true;
}


//________________________________________
void F_GetMousePos(LONG* xPtr, LONG* yPtr) {

	__asm {
		mov edx, yPtr
		mov eax, xPtr
		CALL GET_MOUSE_POS
	}
}


//___________________________________________________________________
LONG F_CheckMouseInRect(LONG left, LONG top, LONG right, LONG bottom) {

	DWORD retVal = 0;
	__asm {
		mov ecx, bottom
		mov ebx, right
		mov edx, top
		mov eax, left
		CALL F_CHECK_MOUSE_IN_RECT
		mov retVal, eax
	}
	return retVal;
}


//_________________________________
WinStruct* GetWinStruct(LONG winRef) {

	WinStruct* winStruct = nullptr;
	__asm {
		mov eax, winRef
		CALL GET_WIN_STRUCT
		mov winStruct, eax
	}
	return winStruct;
}


//__________________________________________________________________________________________
LONG Win_Create(LONG x, LONG y, DWORD width, DWORD height, DWORD BGColourIndex, DWORD flags) {

	LONG winRef = -1;
	__asm {
		push flags
		push BGColourIndex
		mov ecx, height
		mov ebx, width
		mov eax, x
		mov edx, y
		call CREATE_WINDOW_OBJECT
		mov winRef, eax
	}
	return winRef;
}


//__________________________
void DestroyWin(LONG winRef) {

	__asm {
		mov eax, winRef
		call DESTROY_WINDOW_OBJECT
	}
}


//___________________________
BYTE* GetWinBuff(LONG winRef) {

	WinStruct* win = GetWinStruct(winRef);
	if (!win)return nullptr;
	return win->buff;
}


//_______________________
void ShowWin(LONG winRef) {

	__asm {
		cmp eax, -1
		je exitFunc
		mov eax, winRef
		call SHOW_WINDOW_OBJECT
		exitFunc :
	}
}


//_______________________
void HideWin(LONG winRef) {

	__asm {
		mov eax, winRef
		call HIDE_WINDOW_OBJECT
	}
}


//_________________________
void RedrawWin(LONG winRef) {

	__asm {
		mov eax, winRef
		call DRAW_WINDOW_OBJECT
	}
}


//_______________________________________________________________________________________________________
void WinPrintText(LONG WinRef, const char* txtBuff, DWORD txtWidth, LONG x, LONG y, DWORD palColourFlags) {

	__asm {
		push palColourFlags
		push y
		mov ecx, x
		mov ebx, txtWidth
		mov edx, txtBuff
		mov eax, WinRef
		call WIN_PRINT_TEXT
	}
}


//___________________________________________________________________________________________________________________________________________________________________________________________________________
LONG CreateButton(LONG winRef, LONG x, LONG y, DWORD width, DWORD height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, BYTE* upPicBuff, BYTE* downPicBuff, BYTE* otherPicBuff, DWORD flags) {

	LONG buttonRef = -1;
	__asm {
		push flags//normal 0;0x1; 0x2; 0x4;  move window pos 0x10; no transparency 0x20; toggle click 0x23;
		push otherPicBuff
		push downPicBuff
		push upPicBuff
		push keyLift//release button
		push keyPush//push button
		push keyHoverOff//off button
		push keyHoverOn//over button
		push height
		mov ecx, width
		mov ebx, y
		mov edx, x
		mov eax, winRef
		call SET_BUTTON
		mov buttonRef, eax
	}
	return buttonRef;
}


//____________________________________________________________________________________________________________
LONG SetButtonFunctions(LONG buttonRef, void* hoverOnfunc, void* hoverOffFunc, void* pushfunc, void* liftfunc) {

	LONG retVal = 0;
	__asm {
		push liftfunc
		mov ecx, pushfunc
		mov ebx, hoverOffFunc
		mov edx, hoverOnfunc
		mov eax, buttonRef
		call SET_BUTTON_FUNCTIONS
		mov retVal, eax
	}
	return retVal;
}


//______________________________________________________________________
LONG SetButtonSounds(LONG buttonRef, void* soundFunc1, void* soundFunc2) {

	LONG retVal = 0;
	__asm {
		mov ebx, soundFunc2
		mov edx, soundFunc1
		mov eax, buttonRef
		call SET_BUTTON_SOUNDS
		mov retVal, eax
	}
	return retVal;
}


//_____________________
LONG CheckButtons(void) {

	LONG key_code = 0;
	__asm {
		call CHECK_BUTTONS
		mov key_code, eax
	}
	return key_code;
}


//_______________________________________________________
ButtonStruct* GetButton(LONG buttRef, WinStruct** retWin) {

	ButtonStruct* button = nullptr;
	__asm {
		mov edx, retWin
		mov eax, buttRef
		call GET_BUTTON_STRUCT
		mov button, eax
	}
	return button;
}


//_______________________________
LONG SetMouseImage(LONG imageNum) {

	LONG ret_val = -1;
	__asm {
		mov eax, imageNum
		call SET_MOUSE_PIC
		mov ret_val, eax
	}
	return ret_val;//0 = success, -1 = fail
}


//______________________________
DWORD F_SetMouseFrm(DWORD frmID) {

	int ret_val = -1;
	__asm {
		mov eax, frmID
		call F_SET_MOUSE_FRM
		mov ret_val, eax
	}
	return ret_val;//0 = success, -1 = fail
}


//__________________________________
LONG PlayAcm(const char* sound_name) {

	LONG ret_val = 0;
	__asm {
		mov eax, sound_name
		call PLAY_ACM
		mov ret_val, eax
	}
	return ret_val;
}


//________________________________________________________________________________________________________________________________________________________
LONG FMessageBox(const char* text1, const char* text2, LONG text2Flag, LONG xPos, LONG yPos, DWORD text1Colour, DWORD unknown, DWORD text2Colour, DWORD flags) {
	
	//flags 0x10 = yes/no box
	LONG winRef = -1;
	__asm {
		push flags
		push text2Colour
		push unknown
		push text1Colour
		push yPos
		mov ecx, xPos
		mov ebx, text2Flag
		lea edx, text2
		mov eax, text1
		call MESSAGE_BOX
		mov winRef, eax
	}
	return winRef;
}


//____________________________________
BYTE* FAllocateMemory(DWORD sizeBytes) {

	BYTE* mem = nullptr;
	__asm {
		mov eax, sizeBytes
		call ALLOCATE_MEM
		mov mem, eax
	}
	return mem;
}


//_________________________________________________
BYTE* FReallocateMemory(BYTE* mem, DWORD sizeBytes) {

	__asm {
		mov edx, sizeBytes
		mov eax, mem
		call REALLOCATE_MEM
		mov mem, eax
	}
	return mem;
}


//_______________________________
void FDeallocateMemory(BYTE* mem) {

	__asm {
		mov eax, mem
		call DEALLOCATE_MEM
	}
}


//_________________________________________________
void F_ResizeWindows(LONG oldWidth, LONG oldHeight) {

	LONG shiftX = 0;
	LONG shiftY = 0;

	WinStruct* pMainWin = pWinArray[0];
	if (pMainWin) {
		pMainWin->width = SCR_WIDTH;
		pMainWin->height = SCR_HEIGHT;
		pMainWin->rect.left = 0;
		pMainWin->rect.top = 0;
		pMainWin->rect.right = SCR_WIDTH - 1;
		pMainWin->rect.bottom = SCR_HEIGHT - 1;
		*lpWinMainBuff = FReallocateMemory(*lpWinMainBuff, SCR_WIDTH * SCR_HEIGHT);
	}

	ResizeGameWin();
	ReSizeIface();
	ReSizeMaps();

	ReSizeMainMenu();
	ReSizeCredits();
	ResizeMovies();
	ReSizeFadeScrns();
	ReSizeHelpScrn();
	ReSizeDeath();
	ReSize_EndSlides();

	ResizePauseWin();

	for (int i = 1; i < *numWindows; i++) {

		if (pWinArray[i] && pWinArray[i]->ref != -1 &&
			//winArray[i]->ref == *pWinRef_Main ||
			pWinArray[i]->ref != *pWinRef_GameArea &&
			pWinArray[i]->ref != *pWinRef_MainMenu &&
			pWinArray[i]->ref != *pWinRef_EndSlides &&
			pWinArray[i]->ref != *pWinRef_Movies &&
			pWinArray[i]->ref != *pWinRef_Iface &&
			pWinArray[i]->ref != wRef_ifaceLeft &&
			pWinArray[i]->ref != wRef_ifaceRight &&
			pWinArray[i]->ref != *pWinRef_NotifyBar &&
			pWinArray[i]->ref != *pWinRef_Skills &&
			pWinArray[i]->ref != *pWinRef_DialogPcText &&
			pWinArray[i]->ref != *pWinRef_DialogNpcText &&
			pWinArray[i]->ref != *pWinRef_DialogBaseSub) {

			shiftX = 0;
			shiftY = 0;

			if (pWinArray[i]->width <= 640 && SCR_WIDTH != oldWidth) {
				shiftX = (oldWidth / 2) - (SCR_WIDTH / 2);
				pWinArray[i]->rect.left -= shiftX;
				pWinArray[i]->rect.right -= shiftX;
			}

			if (pWinArray[i]->height <= 480 && SCR_HEIGHT != oldHeight) {
				shiftY = (oldHeight / 2) - (SCR_HEIGHT / 2);
				pWinArray[i]->rect.top -= shiftY;
				pWinArray[i]->rect.bottom -= shiftY;
			}

			if (pWinArray[i]->rect.right >= (LONG)SCR_WIDTH) {
				shiftX = pWinArray[i]->rect.right - SCR_WIDTH;
				pWinArray[i]->rect.left -= shiftX;
				pWinArray[i]->rect.right -= shiftX;
			}
			else if (pWinArray[i]->rect.left < 0) {
				pWinArray[i]->rect.left = 0;
				pWinArray[i]->rect.right = pWinArray[i]->rect.left + pWinArray[i]->width - 1;
			}

			if (pWinArray[i]->rect.bottom >= (LONG)SCR_HEIGHT) {
				shiftY = pWinArray[i]->rect.bottom - SCR_HEIGHT;
				pWinArray[i]->rect.top -= shiftY;
				pWinArray[i]->rect.bottom -= shiftY;
			}
			else if (pWinArray[i]->rect.top < 0) {
				pWinArray[i]->rect.top = 0;
				pWinArray[i]->rect.bottom = pWinArray[i]->rect.top + pWinArray[i]->height - 1;
			}
		}

		ReSize_InvDialog();

#if defined(DX9_ENHANCED)

		if (pWinArray[i]->ref != *pWinRef_GameArea)
			SetDxWin(pWinArray[i]);
#endif
	}
	isFadeReset = true;
}



//____________________________
void FWindowsSetup(int region) {

	if (ConfigReadInt("OTHER_SETTINGS", "FADE_TIME_RECALCULATE_ON_FADE", 0))// {
		isFadeResetAlways = true;

	if (region == 4) {
		CREATE_WINDOW_OBJECT = (void*)0x4DC0BD;
		DESTROY_WINDOW_OBJECT = (void*)0x4DC3C0;
		DRAW_WINDOW_OBJECT = (void*)0x4DD278;
		SHOW_WINDOW_OBJECT = (void*)0x4DCFCE;
		HIDE_WINDOW_OBJECT = (void*)0x4DD0FE;

		WIN_PRINT_TEXT = (void*)0x4DC8FE;

		GET_WIN_STRUCT = (void*)0x4DDF63;

		DRAW_WIN_RECT = (void*)0x4DDB46;

		GET_WIN_AT_POS = (void*)0x4DE01C;

		DRAW_SCENE_1 = (void*)0x4B0178;

		DRAW_TO_SCRN = (void*)0x4DD351;

		SET_BUTTON = (void*)0x4DEC67;

		SET_BUTTON_FUNCTIONS = (void*)0x4DF26D;

		SET_BUTTON_SOUNDS = (void*)0x4DF3A6;
		CHECK_BUTTONS = (void*)0x4C92C0;

		GET_BUTTON_STRUCT = (void*)0x4DE25A;

		PLAY_ACM = (void*)0x4510A8;

		MESSAGE_BOX = (void*)0x41CEB0;

		F_ShowMouse = (void(__cdecl*)(void))0x4CB73D;

		F_HideMouse = (void(__cdecl*)(void))0x4CB97B;

		pIS_MOUSE_HIDDEN = (LONG*)0x6BCCFC;

		MOUSE_FLAGS = (DWORD*)0x6BCD1C;

		GET_MOUSE_POS = (void*)0x4CBFD2;
		F_CHECK_MOUSE_IN_RECT = (void*)0x4CBE94;
		GET_MOUSE_RECT = (void*)0x4CBF6F;
		SET_MOUSE_PIC = (void*)0x44BF90;

		F_SET_MOUSE_FRM = (void*)FixAddress(0x44C34C);


		F_GetMousePicNum = (DWORD(__cdecl*)(void))0x44C138;

		ALLOCATE_MEM = (void*)0x4C479B;
		REALLOCATE_MEM = (void*)0x4C4876;
		DEALLOCATE_MEM = (void*)0x4C49A8;

		AFF_FONT_ADDRESS = (void*)0x44123E;

		F_LOAD_PALETTE = (void*)0x4C7C16;
		F_SET_PALETTE = (void*)0x4928F8;
		F_FADE_TO_PALETTE = (void*)0x492884;


		pBLACK_PAL = (BYTE*)0x674550;
		pMAIN_PAL = (BYTE*)0x52DD1C;
		pCURRENT_PAL = (BYTE*)0x683608;

		pWinRef_Char = (LONG*)0x58060C;
		pWinRef_Bio = (LONG*)0x52C5E8;
		//pWinRef_Credits=;
		//pWinRef_Death;
		pWinRef_DialogMain = (LONG*)0x528530;
		pWinRef_DialogNpcText = (LONG*)0x5284D4;
		pWinRef_DialogPcText = (LONG*)0x5284D8;
		pWinRef_DialogBaseSub = (LONG*)0x528534;
		pWinRef_Inventory = (LONG*)0x5AEEE4;
		pWinRef_EndSlides = (LONG*)0x580BF4;
		pWinRef_Iface = (LONG*)0x528E14;
		pWinRef_NotifyBar = (LONG*)0x528E18;
		pWinRef_Skills = (LONG*)0x6786C0;
		pWinRef_LoadSave = (LONG*)0x624844;
		pWinRef_MainMenu = (LONG*)0x5292E0;
		pWinRef_Movies = (LONG*)0x5293A8;
		pWinRef_Options = (LONG*)0x673E84;
		pWinRef_Pipboy = (LONG*)0x674A50;
		//pWinRef_Splash;
		pWinRef_GameArea = (LONG*)0x6423CC;
		pWinRef_WorldMap = (LONG*)0x52DC04;

		pWinArray = (WinStruct**)0x6BE3B4;
		numWindows = (LONG*)0x6BE480;
		lpWinMainBuff = (BYTE**)0x52E1D8;


		//00425EB4  |.  A3 74D35700       MOV DWORD PTR DS:[57D374],EAX            ; pWinRef_Target
		//0043BB2D  |.  A3 E0055800       MOV DWORD PTR DS:[5805E0],EAX            ; pWinRef_CharPerks
		//0043EA95  |.  A3 540A5800       MOV DWORD PTR DS:[580A54],EAX            ; pWinRef_Elevator
		//0047611A  |.  A3 14EE5A00       MOV DWORD PTR DS:[5AEE14],EAX            ; pWinRef_MoveItems
		//0048F40A  |.  A3 803E6700       MOV DWORD PTR DS:[673E80],EAX            ; pWinRef_InGameMenu

		F_FADE_TRANS_SETUP = (void*)0x4927B0;
		F_FADE_TRANS_SETUP2 = (void*)0x4927FC;

		MemWrite8(0x492885, 0x51, 0xE8);
		FuncWrite32(0x492886, 0x55575652, (DWORD)&fade_to_pal);

		P_GAME_EXIT_FLAG = (LONG*)0x5284BC;

		F_ToggleMouseHex = (void(__cdecl*)(void))0x44C2C4;

		F_SEND_DOS_KEY = (void*)0x4C93DC;

		F_SET_MOUSE_POS = (void*)0x4CC017;

		F_SET_MOUSE_PIC = (void*)0x4CB419;
	}
	else {
		CREATE_WINDOW_OBJECT = (void*)FixAddress(0x4D6238);
		DESTROY_WINDOW_OBJECT = (void*)FixAddress(0x4D6468);
		DRAW_WINDOW_OBJECT = (void*)FixAddress(0x4D6F5C);
		SHOW_WINDOW_OBJECT = (void*)FixAddress(0x4D6DAC);
		HIDE_WINDOW_OBJECT = (void*)FixAddress(0x4D6E64);

		GET_WIN_STRUCT = (void*)FixAddress(0x4D7888);
		WIN_PRINT_TEXT = (void*)FixAddress(0x4D684C);

		DRAW_WIN_RECT = (void*)FixAddress(0x4D759C);

		GET_WIN_AT_POS = (void*)FixAddress(0x4D78CC);

		DRAW_SCENE_1 = (void*)FixAddress(0x4B15E8);


		DRAW_TO_SCRN = (void*)FixAddress(0x4D6FD8);

		SET_BUTTON = (void*)FixAddress(0x4D8260);
		SET_BUTTON_FUNCTIONS = (void*)FixAddress(0x4D8758);
		SET_BUTTON_SOUNDS = (void*)FixAddress(0x4D87F8);
		CHECK_BUTTONS = (void*)FixAddress(0x4C8B78);
		GET_BUTTON_STRUCT = (void*)FixAddress(0x4D79DC);


		PLAY_ACM = (void*)FixAddress(0x4519A8);

		MESSAGE_BOX = (void*)FixAddress(0x41CF20);

		F_ShowMouse = (void(__cdecl*)(void))FixAddress(0x4CA34C);
		F_HideMouse = (void(__cdecl*)(void))FixAddress(0x4CA534);


		pIS_MOUSE_HIDDEN = (LONG*)FixAddress(0x6AC790);
		MOUSE_FLAGS = (DWORD*)FixAddress(0x6AC7B0);

		GET_MOUSE_POS = (void*)FixAddress(0x4CA9DC);
		F_CHECK_MOUSE_IN_RECT = (void*)FixAddress(0x4CA934);
		GET_MOUSE_RECT = (void*)FixAddress(0x4CA9A0);
		SET_MOUSE_PIC = (void*)FixAddress(0x44C840);

		F_SET_MOUSE_FRM = (void*)FixAddress(0x44CBFC);

		F_GetMousePicNum = (DWORD(__cdecl*)(void))FixAddress(0x44C9E8);



		ALLOCATE_MEM = (void*)FixAddress(0x4C5AD0);
		REALLOCATE_MEM = (void*)FixAddress(0x4C5B50);
		DEALLOCATE_MEM = (void*)FixAddress(0x4C5C24);

		AFF_FONT_ADDRESS = (void*)FixAddress(0x58E93C);

		F_LOAD_PALETTE = (void*)FixAddress(0x4C78E4);
		F_SET_PALETTE = (void*)FixAddress(0x493B48);
		F_FADE_TO_PALETTE = (void*)FixAddress(0x493AD4);


		pBLACK_PAL = (BYTE*)FixAddress(0x663FD0);
		pMAIN_PAL = (BYTE*)FixAddress(0x51DF34);
		pCURRENT_PAL = (BYTE*)FixAddress(0x673090);

		pWinRef_Char = (LONG*)FixAddress(0x57060C);
		pWinRef_Bio = (LONG*)FixAddress(0x51C7F8);
		//Win_Credits=;
		//Win_Death;
		pWinRef_DialogMain = (LONG*)FixAddress(0x518740);
		pWinRef_DialogNpcText = (LONG*)FixAddress(0x5186E4);
		pWinRef_DialogPcText = (LONG*)FixAddress(0x5186E8);
		pWinRef_DialogBaseSub = (LONG*)FixAddress(0x518744);
		pWinRef_Inventory = (LONG*)FixAddress(0x59E964);
		pWinRef_EndSlides = (LONG*)FixAddress(0x570BF4);
		pWinRef_Iface = (LONG*)FixAddress(0x519024);
		pWinRef_NotifyBar = (LONG*)FixAddress(0x519028);
		pWinRef_Skills = (LONG*)FixAddress(0x668140);
		pWinRef_LoadSave = (LONG*)FixAddress(0x6142C4);
		pWinRef_MainMenu = (LONG*)FixAddress(0x5194F0);
		pWinRef_Movies = (LONG*)FixAddress(0x5195B8);
		pWinRef_Options = (LONG*)FixAddress(0x663904);
		pWinRef_Pipboy = (LONG*)FixAddress(0x6644C4);
		//Win_Splash;
		pWinRef_GameArea = (LONG*)FixAddress(0x631E4C);
		pWinRef_WorldMap = (LONG*)FixAddress(0x51DE14);

		pWinArray = (WinStruct**)FixAddress(0x6ADE58);
		numWindows = (LONG*)FixAddress(0x6ADF24);
		lpWinMainBuff = (BYTE**)FixAddress(0x51E3FC);

		//00426270  |.  A3 74D35600      MOV DWORD PTR DS:[56D374],EAX            ; pWinRef_Target
		//0043C585  |.  A3 E0055700      MOV DWORD PTR DS:[5705E0],EAX            ; pWinRef_CharPerks
		//0043F565  |.  A3 540A5700      MOV DWORD PTR DS:[570A54],EAX            ; pWinRef_Elevator
		//004462AC  |.  A3 E4865100      MOV DWORD PTR DS:[5186E4],EAX            ; pWinRef_DialogNPCResponce
		//0044638C  |.  A3 E8865100      MOV DWORD PTR DS:[5186E8],EAX            ; pWinRef_DialogPCResponce
		//00448338  |.  A3 44875100      MOV DWORD PTR DS:[518744],EAX            ; pWinRef_DialogBaseSub
		//00461604  |.  A3 28905100      MOV DWORD PTR DS:[519028],EAX            ; *pWinRef_NotifyBar
		//00476B1E  |.  A3 94E85900      MOV DWORD PTR DS:[59E894],EAX            ; pWinRef_MoveItems
		//0049000A  |.  A3 00396600      MOV DWORD PTR DS:[663900],EAX            ; pWinRef_InGameMenu
		//004AC265  |.  A3 40816600      MOV DWORD PTR DS:[668140],EAX            ; *pWinRef_SkillBar

		F_FADE_TRANS_SETUP = (void*)FixAddress(0x493A00);
		F_FADE_TRANS_SETUP2 = (void*)FixAddress(0x493A4C);

		MemWrite8(0x493AD5, 0x51, 0xE8);
		FuncWrite32(0x493AD6, 0x55575652, (DWORD)&fade_to_pal);

		P_GAME_EXIT_FLAG = (LONG*)FixAddress(0x5186CC);


		F_ToggleMouseHex = (void(__cdecl*)(void))FixAddress(0x44CB74);

		F_SEND_DOS_KEY = (void*)FixAddress(0x4C8C04);

		F_SET_MOUSE_POS = (void*)FixAddress(0x4CAA04);

		F_SET_MOUSE_PIC = (void*)FixAddress(0x4CA0AC);
	}
}
