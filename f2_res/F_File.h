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

extern void* pCFG_FILE_PTR;
extern char* pCFG_FILE_NAME;

LONG FSetDataPath(const char* path1, int isFolder1, const char* path2, int isFolder2);

LONG FReadCfgInt(void* FileStream, const char* secName, const char* keyName, int* pIntVal);
LONG FReadCfgString(void* FileStream, const char* secName, const char* keyName, char** ppString);
LONG FWriteCfgInt(void* FileStream, const char* secName, const char* keyName, int IntVal);


void* F_fopen(const char* FileName, const char* flags);
LONG F_fclose(void* FileStream);

LONG F_fseek(void* FileStream, LONG fOffset, LONG origin);

LONG F_fread8(void* FileStream, BYTE* toMem);
LONG F_fread16(void* FileStream, WORD* toMem);
LONG F_fread32(void* FileStream, DWORD* toMem);
LONG F_fread16Array(void* FileStream, WORD* toMem, DWORD NumElements);
LONG F_fread32Array(void* FileStream, DWORD* toMem, DWORD NumElements);
LONG FReadString(void* FileStream, char* toMem, DWORD charLength, DWORD NumStrings);
LONG FReadCharArray(void* FileStream, char* toMem, DWORD NumElements);

LONG FDeleteTmpSaveFiles(const char* path, const char* ext);
LONG F_fwrite8(void* FileStream, BYTE val8);
LONG F_fwrite16(void* FileStream, WORD val16);
LONG F_fwrite32(void* FileStream, DWORD val32);
LONG F_fwrite32Array(void* FileStream, DWORD* val32Array, DWORD numElements);

LONG F_GetFileList(const char* path, char*** list);
LONG F_ReleaseFileList(char*** list);

void FFileSetup(DWORD region);
