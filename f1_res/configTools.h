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


UINT ConfigReadInt(const char *lpAppName, const char *lpKeyName, int nDefault);
BOOL ConfigWriteInt(const char *lpAppName, const char *lpKeyName, int intVal);

DWORD ConfigReadString(const char *lpAppName, const char *lpKeyName, const char *lpDefault, char *lpReturnedString, DWORD nSize);
BOOL ConfigWriteString(const char *lpAppName, const char *lpKeyName, const char *lpString);

BOOL ConfigReadWinData(const char *lpAppName, const char *lpKeyName, WINDOWPLACEMENT *pWinData);
BOOL ConfigWriteWinData(const char *lpAppName, const char *lpKeyName, WINDOWPLACEMENT *pWinData);

BOOL ConfigReadStruct(LPCTSTR lpszSection, LPCTSTR lpszKey, LPVOID lpStruct, UINT uSizeStruct);
BOOL ConfigWriteStruct(LPCTSTR lpszSection, LPCTSTR lpszKey, LPVOID lpStruct, UINT uSizeStruct);

UINT SfallReadInt(const char *lpAppName, const char *lpKeyName, int nDefault);
BOOL SfallWriteInt(const char *lpAppName, const char *lpKeyName, int intVal);


