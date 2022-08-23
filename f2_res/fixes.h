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
extern LONG wRef_ifaceLeft;
extern LONG wRef_ifaceRight;

extern LONG OPTIONS_BUTTON;


void OptionsWindow();

long GetScrWidth();
long GetScrHeight();

LONG GenWinSetup(DWORD width, DWORD height, DWORD colour, DWORD winFlags);

void HiResSettings(int region);
void DirectDraw7_Fixes(int region);
void DirectX9_Fixes(int region);

bool IsMainMenuVisible();

void HideIfaceWindows();
void ShowIfaceWindows();
void IfaceSidesReload(int ifaceArtNum);
void DestroyIfaceWindows();

void UpdateMovieGlobals();
void UpdateMapSettings(int region);

LONG SaveBMP8(DWORD width, DWORD height, BYTE* buff, BYTE* palette);
LONG SaveBMP24(LONG srcWidth, LONG srcHeight, LONG srcPixelSize, LONG srcPitch, BYTE* srcBuff);

int CheckMouseInInvRects(int zDelta, int* keyCode);
bool CheckMouseInImonitorRect(int zDelta, bool scrollPage);

#if defined(DX9_ENHANCED)
bool CheckMouseInGameRectScroll(int zDelta, bool scrollPage);
void ResetZoomLevel();
#endif

void SetFadeTime(double fadeTime);

void CharScrnFixes(DWORD region);
void CreditsFixes(int region);
void DialogInventoryFixes(DWORD region);
void OptionsFixes(DWORD region);
void EndSlidesFixes(DWORD region);
void WorldMapFixes(DWORD region);
void DeathScrnFixes(DWORD region);
void MovieFixes(DWORD region);
void LoadSaveFixes(DWORD region);
void MainMenuFixes(DWORD region);
void IfaceFixes(DWORD region);
void PipBoyFixes(DWORD region);
void WinGeneralFixes(int region);
void SplashScrnFixes(DWORD region);
void MapFixes(DWORD region);
void GameScrnFixes(DWORD region);
void HelpScrnFixes(DWORD region);

void WinFallFixes(int region);
void PauseScrnFixes(int region);
