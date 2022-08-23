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

#pragma once


struct ButtonStruct {
	  LONG ref;//?//0x00
	  DWORD flags;//0x04
	  RECT rect;//left 0x08, top 0x0C, right 0x10, bottom 0x14
	  LONG refHvOn;//0x18
	  LONG refHvOff;//0x1C
	  LONG refDn;//0x20
	  LONG refUp;//0x24
	  LONG refDnRht;//-1//0x28 //right mouse button
      LONG refUpRht;//-1//0x2C //right mouse button
      BYTE *buffUp;//0x30
      BYTE *buffDn;//0x34
	  BYTE *buffHv;//0//0x38 other pic
	  BYTE *buffUpDis;//0x3C //upDisabledPic?
	  BYTE *buffDnDis;//0x40 //downDisabledPic?
	  BYTE *buffHvDis;//0x44 //otherDisabledPic?
	  BYTE *buffCurrent;//0x48 //current pic?
	  BYTE *buffDefault;//0x4C //default pic?
	  void *funcHvOn;//0x50
	  void *funcHvOff;//0x54
	  void *funcDn;//0x58
	  void *funcUp;//0x5C
	  void *funcDnRht;//0x60//right mouse button
	  void *funcUpRht;//0x64//right mouse button
	  void *funcDnSnd;//0x68 push sound func
	  void *funcUpSnd;//0x6C lift sound func
	  DWORD unknown70;//0x70
	  ButtonStruct *nextButton;//0x74
	  ButtonStruct *prevButton;//0x78
};

#define FLG_ButtToggle      0x00000001
#define FLG_ButtHoldOn      0x00000002  //button pops up if not held.
#define FLG_ButtTglOnce     0x00000004  //this + toggle == when toggle set button remains down permanently once pressed;
#define FLG_ButtDisabled    0x00000008  //button disabled?
#define FLG_ButtDragWin     0x00000010  //drag owner window position
#define FLG_ButtTrans       0x00000020  //palette pos 0 is tansparent
#define FLG_ButtReturnMsg   0x00000040  //return button message even if custom fuctions used.
#define FLG_ButtPics        0x00010000  //custom pics, buffers must be destroyed manually - automaticaly set by create pic button func.
#define FLG_ButtTglDn       0x00020000  //Button starts on, for use with toggle, will be stuck down in normal mode.
//#define FLG_ButtTUnknown   0x00080000  //dont know?

//#pragma pack(4)
class WinStruct {
      public:
	  LONG ref;//0x00
	  DWORD flags;//0x04
	  RECT rect;//left 0x08, top 0x0C, right 0x10, bottom 0x14
	  LONG width;//0x18
      LONG height;//0x1C
      DWORD colour;//0x20//colour index offset?
	  DWORD unknown24;//0x24//x?
	  DWORD unknown28;//0x28//y?
	  BYTE *buff;//0x2C         // bytes frame data ref to palette
	  ButtonStruct *ButtonList;//0x30//button struct list?
	  DWORD unknown34;//0x34
	  DWORD unknown38;//0x38
	  DWORD unknown3C;//0x3C
	  void (*pBlit)(BYTE *fBuff, LONG subWidth, LONG subHeight, LONG fWidth, BYTE *LONG, DWORD tWidth);//0x40//drawing func address
};


#define F_WIN_HIDDEN 0x8
#define F_WIN_FRONT  0x4

#define F_MSE_LPRESS 0x1
#define F_MSE_LHOLD  0x4
#define F_MSE_RPRESS 0x2
#define F_MSE_RHOLD  0x8

//extern LONG *pWinRef_Main;
extern LONG *pWinRef_Char;
extern LONG *pWinRef_Bio;
//extern LONG *pWinRef_Credits;
//extern LONG *pWinRef_Death;
extern LONG *pWinRef_DialogMain;

extern LONG *pWinRef_DialogNpcText;
extern LONG *pWinRef_DialogPcText;
extern LONG *pWinRef_DialogBaseSub;

extern LONG *pWinRef_Inventory;
extern LONG *pWinRef_EndSlides;
extern LONG *pWinRef_Iface;
extern LONG *pWinRef_NotifyBar;
extern LONG *pWinRef_Skills;
extern LONG *pWinRef_LoadSave;
extern LONG *pWinRef_MainMenu;
extern LONG *pWinRef_Movies;
extern LONG *pWinRef_Options;
extern LONG *pWinRef_Pipboy;
//extern LONG *pWinRef_Splash;
extern LONG *pWinRef_GameArea;
extern LONG *pWinRef_WorldMap;



extern BYTE* pBLACK_PAL;
extern BYTE* pMAIN_PAL;
extern BYTE* pCURRENT_PAL;

extern bool isFadeReset;

extern LONG *numWindows;
extern WinStruct **pWinArray;

bool IsExitGame();

void F_ResizeWindows(LONG oldWidth, LONG oldHeight);
bool ResizeGameWin();
bool ReSizeMaps();
bool ReSizeIface();
void ReSizeMainMenu();
void ReSizeCredits();
void ResizeMovies();
void ReSizeFadeScrns();
void ReSizeHelpScrn();
void ReSizeDeath();
void ReSize_EndSlides();
void ReSize_InvDialog();
void ResizePauseWin();

void FWindowsSetup();

LONG Win_Create(LONG x, LONG y, DWORD width, DWORD height, DWORD BGColourIndex, DWORD flags);
void DestroyWin(LONG winRef);
WinStruct *GetWinStruct(LONG winRef);
BYTE* GetWinBuff(LONG winRef);

void ShowWin(LONG winRef);
void HideWin(LONG winRef);
void RedrawWin(LONG winRef);

void WinPrintText(LONG WinRef, const char *txtBuff, LONG txtWidth, LONG x, LONG y, DWORD palColourFlags);

void FDrawWinRect(RECT *rect);
void FDrawWinRectRelative(WinStruct *win, RECT *rect);


LONG CreateButton(LONG winRef, LONG x, LONG y, LONG width, LONG height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, BYTE *upPicBuff, BYTE *downPicBuff, BYTE *otherPicBuff, DWORD flags);
LONG SetButtonFunctions(LONG buttonRef, void *hoverOnfunc, void *hoverOffFunc, void *pushfunc, void *liftfunc);
LONG SetButtonSounds(LONG buttonRef, void *soundFunc1, void *soundFunc2);
LONG CheckButtons(void);
ButtonStruct* GetButton(LONG buttRef, WinStruct **retWin);


LONG PlayAcm(const char*sound_name);


LONG FMessageBox(const char*text1, const char*text2, DWORD text2Flag, LONG xPos, LONG yPos, DWORD text1Colour, DWORD unknown, DWORD text2Colour, DWORD flags);

typedef void (*F_VOID_FUNC_VOID)(void);
typedef LONG (*F_LONG_FUNC_VOID)(void);
typedef DWORD (*F_DWORD_FUNC_VOID)(void);

extern F_VOID_FUNC_VOID F_ShowMouse;
extern F_VOID_FUNC_VOID F_HideMouse;

extern F_DWORD_FUNC_VOID F_GetMousePicNum;
LONG SetMouseImage(LONG imageNum);

bool IsMouseHidden();

DWORD GetMouseFlags();
void F_GetMousePos(LONG *xPtr, LONG *yPtr);
LONG F_CheckMouseInRect(LONG left, LONG top, LONG right, LONG bottom);
bool IsMouseInRect(RECT *rect);

BYTE* FAllocateMemory(DWORD sizeBytes);
BYTE* FReallocateMemory(BYTE* mem, DWORD sizeBytes);
void FDeallocateMemory( BYTE* mem);

LONG FLoadPalette(const char *FileName);
void FSetPalette(BYTE *pal);
void FFadeToPalette(BYTE *pal);

void Fix8BitColours(BYTE *buff, LONG subWidth, LONG subHeight, LONG buffWidth, BYTE darkColour);
BYTE FindDarkPalRef( BYTE* pPalette);

void F_DrawToScrn(WinStruct *win, RECT *rect, BYTE *buff);
void DrawWindowArea(LONG winRef, RECT *rect);
LONG F_GetWinAtPos(LONG x, LONG y);


void F_HideMainMenu( DWORD fadeFlag);
void F_ShowMainMenu( DWORD fadeFlag);
bool HiResWindow();

bool AdjustWinPosToGame(DWORD width, DWORD height, LONG *px, LONG *py);
DWORD F_SetMouseFrm(DWORD frmID);
extern F_VOID_FUNC_VOID F_ToggleMouseHex;

void F_SendDosKey(DWORD key);

void F_GetMouseRect(RECT *rcMouse);
void F_SetMousePos(LONG xPos, LONG yPos);
LONG F_SetMousePic(BYTE *fBuff, LONG subWidth, LONG subHeight, LONG fWidth, LONG fX, LONG fY, DWORD maskColour);
