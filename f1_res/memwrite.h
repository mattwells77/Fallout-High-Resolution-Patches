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


void FuncWrite32(DWORD offset, DWORD original, DWORD funcAddress);

void FuncReplace32(DWORD offset, DWORD original, DWORD funcAddress);

void MemWriteString(DWORD  offset, unsigned char*original, unsigned char* change_to, DWORD length);
void MemWrite32(DWORD offset, DWORD original, DWORD change_to);
void MemWrite16(DWORD offset, WORD original, WORD change_to);
void MemWrite8(DWORD offset, BYTE original, BYTE change_to);


void MemBlt8(BYTE *fromBuff, int subWidth, int subHeight, int fromWidth, BYTE *toBuff, int toWidth);
void MemBltMasked8(BYTE *fromBuff, int subWidth, int subHeight, int fromWidth, BYTE *toBuff, int toWidth);
void MemBlt8Stretch(BYTE *fromBuff, int subWidth, int subHeight, int fromWidth, BYTE *toBuff, int toWidth, int toHeight, bool ARatio, bool centred);

void print_mem_errors();

DWORD FixAddress(DWORD memAddess);

int region_check(void);

void MemWriteUnSafe32(DWORD offset, DWORD change_to);
void FuncWriteUnSafe32(DWORD offset, DWORD funcAddress);
void MemWriteUnSafe8(DWORD offset, BYTE change_to);
