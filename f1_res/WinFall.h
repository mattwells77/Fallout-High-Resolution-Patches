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


extern HWND *phWinMain;
extern HWND hGameWnd;

extern DWORD scaler;

extern LONG *is_winActive;
extern RECT *SCRN_RECT;
extern bool isWindowed;
extern bool isGameMode;
extern bool isAltMouseInput;
extern bool isMapperExiting;

extern int graphicsMode;

extern bool isMapperSizing;
extern bool isGrayScale;

union BUFF {
  BYTE *b;
  WORD *w;
  DWORD *d;
};

extern int (*GraphicsLibSetup)(void);

int SettingsMenu();

void WindowVars_Save();
void WindowVars_Load();
void SetWindowTitle(HWND hwnd, const char *msg);

void GrayScale(BYTE *r, BYTE *g , BYTE *b);

int CheckMessages();
