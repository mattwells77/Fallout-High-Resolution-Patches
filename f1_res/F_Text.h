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


LONG Str_FindLastWord(const char *msg, LONG lineWidth);
LONG Str_FindLastWord_NL(const char *msg, LONG lineWidth);
LONG Str_FindLastWordMulti(const char *msg, LONG lineWidth);
LONG Str_FindLastWordMulti_NL(const char *msg, LONG lineWidth);//NewLine detection - doesn't work with msg's

void SetFont(LONG ref);
LONG GetFont(void);
void PrintText(BYTE *toBuff, const char *txtBuff, DWORD txtWidth, DWORD toWidth, BYTE palColour);
void PrintText2(BYTE *toBuff, const char *txtBuff, DWORD txtWidth, LONG toX, LONG toY, DWORD toWidth, BYTE palColour);
void PrintText2Win(LONG winRef, const char *txtBuff, LONG txtWidth, LONG toX, LONG toY, BYTE palColour);
DWORD GetTextHeight();
DWORD GetTextWidth(const char *TextMsg);
DWORD GetCharWidth(char CharVal);
DWORD GetMaxTextWidth(const char *TextMsg);
DWORD GetCharGapWidth();
DWORD GetTextBoxSize(const char *TextMsg);
DWORD GetMaxCharWidth();


void FTextSetup();



