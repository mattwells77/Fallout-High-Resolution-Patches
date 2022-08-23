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
//WinStruct **lpWinMain = nullptr;
BYTE** lpWinMainBuff = nullptr;

LONG* pWinRef_Char = nullptr;
LONG* pWinRef_Bio = nullptr;
//LONG* Win_Credits = nullptr;
//LONG* Win_Death = nullptr;
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
//LONG* Win_Splash = nullptr;
LONG* pWinRef_GameArea = nullptr;
LONG* pWinRef_WorldMap = nullptr;


void* CREATE_WINDOW_OBJECT = nullptr;
void* DESTROY_WINDOW_OBJECT = nullptr;
void* DRAW_WINDOW_OBJECT = nullptr;

void* SHOW_WINDOW_OBJECT = nullptr;
void* HIDE_WINDOW_OBJECT = nullptr;

//void* GET_WINDOW_SURFACE = nullptr;
//void* GET_WINDOW_WIDTH = nullptr;
//void* GET_WINDOW_HEIGHT = nullptr;

void* GET_WIN_STRUCT = nullptr;

void* WIN_PRINT_TEXT = nullptr;

void* DRAW_WIN_RECT = nullptr;

void* GET_WIN_AT_POS = nullptr;


void* SET_BUTTON = nullptr;
void* SET_BUTTON_FUNCTIONS = nullptr;
void* SET_BUTTON_SOUNDS = nullptr;
//DWORD SET_DEFAULT_BUTTON_PIC = nullptr;
void* CHECK_BUTTONS = nullptr;
void* GET_BUTTON_STRUCT = nullptr;

void* SET_MOUSE_PIC = nullptr;

void* PLAY_ACM = nullptr;


void* MESSAGE_BOX = nullptr;
//void* LOAD_MSG_FILE = nullptr;

//void* MENUSELECT1 = nullptr;
//void* SETTEXTBUTTON = nullptr;

//void (*F_ShowMouse)(void);
//void (*F_HideMouse)(void);
F_VOID_FUNC_VOID F_ShowMouse = nullptr;
F_VOID_FUNC_VOID F_HideMouse = nullptr;
F_DWORD_FUNC_VOID F_GetMousePicNum = nullptr;

LONG* pIS_MOUSE_HIDDEN = nullptr;
DWORD* MOUSE_FLAGS = nullptr;
void* GET_MOUSE_POS = nullptr;
void* F_CHECK_MOUSE_IN_RECT = nullptr;
void* GET_MOUSE_RECT = nullptr;
void* F_SET_MOUSE_POS = nullptr;

void* F_SET_MOUSE_FRM = nullptr;

//void* BLT_01 = nullptr;
//void* BLT_02 = nullptr;

void* ALLOCATE_MEM = nullptr;
void* REALLOCATE_MEM = nullptr;
void* DEALLOCATE_MEM = nullptr;



void* AFF_FONT_ADDRESS = nullptr;

void* F_LOAD_PALETTE = nullptr;

void* F_SET_PALETTE = nullptr;

void* F_FADE_TO_PALETTE = nullptr;


//DWORD F_DDRAW_FUNC_1;

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
		//push eax //*pal
		pushad
			call fade_trans_setup2
			popad
			//pop eax   //*pal

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

/*
//______________________________________
void FDrawScene1(RECT *rect, LONG level) {

   __asm {
	  mov edx, level
	  mov eax, rect
	  call DRAW_SCENE_1
   }
}
*/
/*
//____________________________________________________________________________________________________________________________________________
void FDDraw01(BYTE*fromBuff, DWORD fromWidth, DWORD fromHeight, DWORD fromX, DWORD fromY, DWORD toWidth, DWORD toHeight, DWORD toX, DWORD toY) {

   __asm
	{
	 push toY
	 push toX
	 push toHeight
	 push toWidth
	 push fromY
	 push fromX
	 push fromHeight
	 push fromWidth
	 push fromBuff
	 mov ebx, F_DDRAW_FUNC_1
	 CALL DWORD PTR DS:[ebx]
	 add esp, 0x24
	}
}
*/


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
void Fix8BitColours(BYTE* buff, LONG subWidth, LONG subHeight, LONG buffWidth, BYTE darkColour) {

	if (COLOUR_BITS != 8)
		return;

	for (LONG y = 0; y < subHeight; y++) {
		for (LONG x = 0; x < subWidth; x++) {
			if (buff[x] > 254)
				buff[x] = darkColour;//colour;
		}
		buff += buffWidth;
	}
}


//____________________________________
BYTE FindDarkPalRef(BYTE* pPalette) {

	int palSize = 256;
	if (COLOUR_BITS == 8)
		palSize--;
	//BYTE *pPalette =pMAIN_PAL;
	BYTE r = 0, g = 0, b = 0;
	BYTE cMax = 63;
	BYTE cMaxT = 63;
	BYTE cdiff = 63;
	BYTE cdiffT = 63;
	LONG palOff = 0;
	BYTE darkRef = 0;
	//int tempColour=255+255+255;
	//int tempColour=63+63+63;
	//int currentColour=0;

	for (int ref = 0; ref < palSize; ref++) {
		r = pPalette[palOff + 2];
		g = pPalette[palOff + 1];
		b = pPalette[palOff + 0];
		//currentColour=r+g+b;

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
			//if(currentColour < tempColour) {//} && currentColour!=0) {

			cdiffT = cdiff;
			cMaxT = cMax;
			//tempColour=currentColour;
			darkRef = ref;
			// }
		}
		palOff += 3;
	}

	return darkRef;
}


//_____________________________________
LONG FLoadPalette(const char* FileName) {

	LONG retVal = 0;
	__asm {
		mov eax, FileName
		CALL F_LOAD_PALETTE
		mov retVal, eax
	}
	return retVal;
}


//__________________________
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


//__________________
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


//__________________________________
WinStruct* GetWinStruct(LONG WinRef) {

	WinStruct* winStruct = nullptr;
	__asm {
		mov eax, WinRef
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

/*
//__________________________
BYTE* GetWinBuff(LONG winRef) {

   BYTE *surface = nullptr;
   __asm {
	  mov eax, winRef
	  call GET_WINDOW_SURFACE
	  mov surface, eax
   }
   return surface;
}
*/

//___________________________
BYTE* GetWinBuff(LONG winRef) {

	WinStruct* win = GetWinStruct(winRef);
	if (!win)return nullptr;
	return win->buff;
}

/*
//_____________________________
LONG GetWinWidth(LONG winRef) {

   LONG width;
   __asm {
	  mov eax, winRef
	  call GET_WINDOW_WIDTH
	  mov width, eax
   }
   return width;
}
*/

/*
//______________________________
LONG GetWinHeight(LONG winRef) {

   LONG height;
   __asm {
	  mov eax, winRef
	  call GET_WINDOW_HEIGHT
	  mov height, eax
   }
   return height;
}
*/

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


//______________________________________________________________________________________________________
void WinPrintText(LONG winRef, const char* txtBuff, LONG txtWidth, LONG x, LONG y, DWORD palColourFlags) {

	__asm {
		//xor eax, eax
		//mov al, palColour
		//push eax
		push palColourFlags
		push y
		mov ecx, x
		mov ebx, txtWidth
		mov edx, txtBuff
		mov eax, winRef
		call WIN_PRINT_TEXT
	}
}


//_________________________________________________________________________________________________________________________________________________________________________________________________________
LONG CreateButton(LONG winRef, LONG x, LONG y, LONG width, LONG height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, BYTE* upPicBuff, BYTE* downPicBuff, BYTE* otherPicBuff, DWORD flags) {

	LONG buttonRef = -1;
	__asm {
		push flags//normal 0; move window pos 0x10; no transparency 0x20; toggle click 0x23;
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


//_____________________________________________________________________________________________________________________________________________________________
LONG FMessageBox(const char* text1, const char* text2, DWORD text2Flag, LONG xPos, LONG yPos, DWORD text1Colour, DWORD unknown, DWORD text2Colour, DWORD flags) {
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

/*
//____________________________________________________
LONG load_msg_file(DWORD msg_object, char* file_name) {

   LONG ret_val = 0;
   __asm {
	  mov edx, file_name
	  mov eax, msg_object
	  call LOAD_MSG_FILE
	  mov ret_val, eax
   }
   return ret_val;
}
*/

/*
//______________________________________________________________________________________________________________________________________________
LONG MenuSelect1(char* MessageText, char **ListPtr, DWORD ListSize, DWORD zero, LONG Xpos, LONG Ypos, DWORD MenuTextColour, LONG CurrentMenuPos) {

   __asm {
	  push CurrentMenuPos
	  push MenuTextColour
	  push Ypos
	  push Xpos
	  mov ecx, zero
	  mov ebx, ListSize
	  mov edx, ListPtr
	  mov eax, MessageText
	  CALL MENUSELECT1
	  mov CurrentMenuPos, eax
   }
   return CurrentMenuPos;
}


//__________________________________________________________________________________________________________________________________________________________________
LONG SetTextButton(LONG winRef, LONG Xpos, LONG Ypos, LONG ButtonHoverOn, LONG ButtonHoverOff, LONG ButtonPush, LONG ButtonLift, char*ButtonLabel, DWORD ButtonType) {

   LONG ret_val = 0;
   __asm {
	  PUSH ButtonType
	  PUSH ButtonLabel
	  PUSH ButtonLift
	  PUSH ButtonPush
	  PUSH ButtonHoverOff
	  MOV ECX, ButtonHoverOn
	  MOV EBX, Ypos
	  MOV EDX, Xpos
	  MOV EAX, winRef
	  CALL SETTEXTBUTTON
	  mov ret_val, eax
   }
   return ret_val;
}
*/

/*
//__________________________________________________________________________________________________
void Blit(BYTE *FromBuff, LONG SubWidth, LONG SubHeight, LONG FromWidth, BYTE *ToBuff, LONG ToWidth) {

  __asm {
	 push ToWidth
	 push ToBuff
	 push FromWidth
	 push SubHeight
	 push SubWidth
	 push FromBuff
	 call BLT_01
	 add esp, 0x18
  }
}
*/

/*
//________________________________________________________________________________________________________
void BlitMasked(BYTE *FromBuff, LONG SubWidth, LONG SubHeight, LONG FromWidth, BYTE *ToBuff, LONG ToWidth) {

   __asm {
	  push ToWidth
	  push ToBuff
	  push FromWidth
	  push SubHeight
	  push SubWidth
	  push FromBuff
	  call BLT_02
	  add esp, 0x18
   }

}
*/

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


//__________________________________________________
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

	int shiftX = 0;
	int shiftY = 0;
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

	//ReSize_InvDialog();
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
			//pWinArray[i]->ref != *pWinRef_Inventory &&
			//pWinArray[i]->ref != *pWinRef_DialogMain &&
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
				// shiftY = ((oldHeight-100)/2) - (newHeight/2);
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


//__________________
void FWindowsSetup() {

	CREATE_WINDOW_OBJECT = (void*)FixAddress(0x4C2828);//donef1
	DESTROY_WINDOW_OBJECT = (void*)FixAddress(0x4C2A54);
	DRAW_WINDOW_OBJECT = (void*)FixAddress(0x4C3548);
	SHOW_WINDOW_OBJECT = (void*)FixAddress(0x4C3398);
	HIDE_WINDOW_OBJECT = (void*)FixAddress(0x4C3450);
	//GET_WINDOW_SURFACE = (void*)FixAddress(0x4C3E98);
	//GET_WINDOW_WIDTH = (void*)FixAddress(0x4C3F00);
	//GET_WINDOW_HEIGHT = (void*)FixAddress(0x4C3F1C);


	GET_WIN_STRUCT = (void*)FixAddress(0x4C3E70);
	WIN_PRINT_TEXT = (void*)FixAddress(0x4C2E38);
	DRAW_WIN_RECT = (void*)FixAddress(0x4C3B84);

	GET_WIN_AT_POS = (void*)FixAddress(0x4C3EB4);

	DRAW_SCENE_1 = (void*)FixAddress(0x49E6DC);

	//BLT_01 = FixAddress(0x4D36D4);
	//BLT_02 = FixAddress(0x4D3704);//////////////////////////////////////////////////////////////////////////////////////////
	//WIN_GET_RECT=(void*)FixAddress(0x4D7950);//takes edx ptr to RECT struct, eax Window obj

	DRAW_TO_SCRN = (void*)FixAddress(0x4C35C4);

	SET_BUTTON = (void*)FixAddress(0x4C4850);
	SET_BUTTON_FUNCTIONS = (void*)FixAddress(0x4C4D40);
	SET_BUTTON_SOUNDS = (void*)FixAddress(0x4C4DE0);
	CHECK_BUTTONS = (void*)FixAddress(0x4B38F8);
	GET_BUTTON_STRUCT = (void*)FixAddress(0x4C3FC4);

	PLAY_ACM = (void*)FixAddress(0x449734);

	MESSAGE_BOX = (void*)FixAddress(0x41BFF0);


	//MENUSELECT1 = FixAddress(0x4DA70C);
	//SETTEXTBUTTON = FixAddress(0x4D8308);

	F_ShowMouse = (void(__cdecl*)(void))FixAddress(0x4B50B8);
	F_HideMouse = (void(__cdecl*)(void))FixAddress(0x4B52A0);

	pIS_MOUSE_HIDDEN = (LONG*)FixAddress(0x671F38);

	MOUSE_FLAGS = (DWORD*)FixAddress(0x671F58);

	GET_MOUSE_POS = (void*)FixAddress(0x4B5798);
	F_CHECK_MOUSE_IN_RECT = (void*)FixAddress(0x4B56F0);
	GET_MOUSE_RECT = (void*)FixAddress(0x4B575C);
	SET_MOUSE_PIC = (void*)FixAddress(0x444734);

	F_SET_MOUSE_FRM = (void*)FixAddress(0x444AF0);

	F_GetMousePicNum = (DWORD(__cdecl*)(void))FixAddress(0x4448DC);


	ALLOCATE_MEM = (void*)FixAddress(0x4AF140);
	REALLOCATE_MEM = (void*)FixAddress(0x4AF1D4);
	DEALLOCATE_MEM = (void*)FixAddress(0x4AF2A8);


	AFF_FONT_ADDRESS = (void*)FixAddress(0x58CC2C);

	F_LOAD_PALETTE = (void*)FixAddress(0x4C099C);
	F_SET_PALETTE = (void*)FixAddress(0x485678);
	F_FADE_TO_PALETTE = (void*)FixAddress(0x485604);

	//F_DDRAW_FUNC_1=FixAddress(0x6721B8);

	pBLACK_PAL = (BYTE*)FixAddress(0x662540);
	pMAIN_PAL = (BYTE*)FixAddress(0x539FF4);
	pCURRENT_PAL = (BYTE*)FixAddress(0x6732E0);

	pWinRef_Char = (LONG*)FixAddress(0x56EC94);
	pWinRef_Bio = (LONG*)FixAddress(0x507A6C);
	//Win_Credits=;
	//Win_Death;
	pWinRef_DialogMain = (LONG*)FixAddress(0x505120);
	pWinRef_DialogNpcText = (LONG*)FixAddress(0x505290);
	pWinRef_DialogPcText = (LONG*)FixAddress(0x505294);
	pWinRef_DialogBaseSub = (LONG*)FixAddress(0x505124);
	//00442657  |.  A3 24535000           MOV DWORD PTR DS:[505324],EAX            ; pWinRef_DialogTellMeAbout
	pWinRef_Inventory = (LONG*)FixAddress(0x59CF18);
	pWinRef_EndSlides = (LONG*)FixAddress(0x56EEE0);
	pWinRef_Iface = (LONG*)FixAddress(0x505700);
	pWinRef_NotifyBar = (LONG*)FixAddress(0x505704);
	pWinRef_Skills = (LONG*)FixAddress(0x6651A8);
	pWinRef_LoadSave = (LONG*)FixAddress(0x612D78);
	pWinRef_MainMenu = (LONG*)FixAddress(0x505B78);
	pWinRef_Movies = (LONG*)FixAddress(0x505C24);
	pWinRef_Options = (LONG*)FixAddress(0x661E90);
	pWinRef_Pipboy = (LONG*)FixAddress(0x662CA8);
	//Win_Splash;
	pWinRef_GameArea = (LONG*)FixAddress(0x6303E8);
	pWinRef_WorldMap = (LONG*)FixAddress(0x670FF4);


	pWinArray = (WinStruct**)FixAddress(0x6AC208);
	numWindows = (LONG*)FixAddress(0x6AC2D4);

	//lpWinMain = (WinStruct**)FixAddress(0x6AC208);
	lpWinMainBuff = (BYTE**)FixAddress(0x53A340);




	F_FADE_TRANS_SETUP = (void*)FixAddress(0x485530);
	F_FADE_TRANS_SETUP2 = (void*)FixAddress(0x48557C);

	if (ConfigReadInt("OTHER_SETTINGS", "FADE_TIME_RECALCULATE_ON_FADE", 0))
		isFadeResetAlways = true;

	MemWrite8(0x485605, 0x51, 0xE8);
	FuncWrite32(0x485606, 0x55575652, (DWORD)&fade_to_pal);


	P_GAME_EXIT_FLAG = (LONG*)FixAddress(0x5050C8);

	F_ToggleMouseHex = (void(__cdecl*)(void))FixAddress(0x444A68);

	F_SEND_DOS_KEY = (void*)FixAddress(0x4B3984);

	F_SET_MOUSE_POS = (void*)FixAddress(0x4B57BC);

	F_SET_MOUSE_PIC = (void*)FixAddress(0x4B4E1C);
}
