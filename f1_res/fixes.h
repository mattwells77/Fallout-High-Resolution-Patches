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

#define ARROW_UP 0x100 + 0x48
#define ARROW_DOWN 0x100 + 0x50
#define ARROW_LEFT 0x100 + 0x4B
#define ARROW_RIGHT 0x100 + 0x4D

extern RECT edgeRect;

extern LONG SCR_WIDTH;
extern LONG SCR_HEIGHT;
extern DWORD COLOUR_BITS;
extern DWORD REFRESH_RATE;


extern UINT IFACE_BAR_MODE;
extern int wRef_ifaceLeft;
extern int wRef_ifaceRight;

extern LONG OPTIONS_BUTTON;
void OptionsWindow();


LONG GetScrWidth();
LONG GetScrHeight();

LONG GenWinSetup(DWORD width, DWORD height, DWORD colour, DWORD winFlags);


void HiResSettings();
void DirectDraw7_Fixes();
void DirectX9_Fixes();

bool IsMainMenuVisible();

void HideIfaceWindows();
void ShowIfaceWindows();
void IfaceSidesReload(int ifaceArtNum);
void DestroyIfaceWindows();

void UpdateMovieGlobals();
void UpdateMapSettings();

LONG SaveBMP8(DWORD width, DWORD height, BYTE *buff, BYTE *palette);
LONG SaveBMP24(LONG srcWidth, LONG srcHeight, LONG srcPixelSize, LONG srcPitch, BYTE *srcBuff);

int CheckMouseInInvRects(int zDelta, int *keyCode);
bool CheckMouseInImonitorRect(int zDelta, bool scrollPage);
#if defined(DX9_ENHANCED)
bool CheckMouseInGameRectScroll(int zDelta, bool scrollPage);

void ResetZoomLevel();
#endif

void SetFadeTime(double fadeTime);

void CharScrnFixes();
void CreditsFixes();
void DialogInventoryFixes();
void OptionsFixes();
void EndSlidesFixes();
void WorldMapFixes();
void DeathScrnFixes();
void MovieFixes();
void LoadSaveFixes();
void MainMenuFixes();
void IfaceFixes();
void PipBoyFixes();
void WinGeneralFixes();
void SplashScrnFixes();
void MapFixes();
void GameScrnFixes();
void HelpScrnFixes();

void OtherFixes();

void WinFallFixes();
void PauseScrnFixes();


