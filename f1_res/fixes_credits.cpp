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

#include "fixes.h"
#include "memwrite.h"
#include "F_Windows.h"
#include "WinFall.h"

BYTE* creditWinBuff = nullptr;
BYTE* creditBuff1 = nullptr;
BYTE* creditBuff2 = nullptr;
BYTE* creditBuff2_top = nullptr;
BYTE* creditBuff2_bottom = nullptr;
int winRef_Credits = -1;


//______________________________________________________
DWORD _stdcall CreditsWinSetup(int colour, int winFlags) {

	winRef_Credits = Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);
	WinStruct* win = GetWinStruct(winRef_Credits);
	creditWinBuff = win->buff;

	return winRef_Credits;
}


//___________________________________________________________________________________________________________
void CreditsBltWinBuff(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth) {

	WinStruct* creditWin = GetWinStruct(winRef_Credits);
	fromBuff = creditBuff2;
	toBuff = creditWinBuff;
	toBuff += (SCR_WIDTH >> 1) - 320;
	MemBlt8(fromBuff, subWidth, SCR_HEIGHT, fromWidth, toBuff, SCR_WIDTH);
}


//_________________________________________________________________________________________________________
void CreditsBltBuff1(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth) {

	fromBuff = creditBuff1;
	toBuff = creditWinBuff;
	toBuff += (SCR_WIDTH >> 1) - 320;
	MemBlt8(fromBuff, subWidth, SCR_HEIGHT, fromWidth, toBuff, SCR_WIDTH);
}


//____________________________________________________
void __declspec(naked)allocate_credit_scrn_buff1(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		mov eax, 640
		imul eax, SCR_HEIGHT
		push eax
		call FAllocateMemory
		add esp, 0x04
		//lea eax, eax
		mov creditBuff1, eax
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//_____________________________
BYTE* AllocateCreditScrnBuff2() {

	creditBuff2 = FAllocateMemory(640 * SCR_HEIGHT);
	creditBuff2_top = &creditBuff2[640];
	creditBuff2_bottom = &creditBuff2[640 * (SCR_HEIGHT - 1)];
	return creditBuff2;
}


//____________________________________________________
void __declspec(naked)allocate_credit_scrn_buff2(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		call AllocateCreditScrnBuff2
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//________________________________________________
void* F_memset(void* buffer, int ch, size_t count) {

	return memset(buffer, ch, count);
}


//________________________________________________
void __declspec(naked)clear_credit_scrn_buff(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp


		mov ebx, 640
		imul ebx, SCR_HEIGHT
		push ebx
		push edx
		push eax
		call F_memset
		add esp, 0x0C

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//______________________________________________________
void* F_memcpy(void* to, const void* from, size_t count) {

	count = 640 * (SCR_HEIGHT - 1);
	return memcpy(to, from, count);
}

//___________________________________________________
void __declspec(naked)memcpy_credit_scrn_buff(void) {

	__asm {
		push ecx
		push esi
		push edi


		push ebx
		push edx
		push eax
		call F_memcpy
		add esp, 0x0C

		pop edi
		pop esi
		pop ecx
		ret
	}
}


//___________________________________________
void __declspec(naked)copy_textbuff_fix(void) {

	__asm {
		mov edi, 640
		sub edi, ecx
		shr edi, 1
		add edi, creditBuff2_bottom
		ret
	}
}


//_____________________________________________
void __declspec(naked)destroy_win_credits(void) {

	__asm {
		cmp winRef_Credits, -1
		je exitFunc
		push winRef_Credits
		call DestroyWin
		add esp, 0x4
		mov winRef_Credits, -1
		exitFunc:
		ret
	}
}

//__________________
void ReSizeCredits() {

	if (winRef_Credits == -1)
		return;

	WinStruct* creditWin = GetWinStruct(winRef_Credits);
	if (!creditWin)
		return;

	LONG oldHeight = creditWin->height;
	BYTE* oldBuff = new BYTE[640 * oldHeight];
	memcpy(oldBuff, creditBuff2, 640 * oldHeight);

	creditWin->width = SCR_WIDTH;
	creditWin->height = SCR_HEIGHT;
	creditWin->rect.left = 0;
	creditWin->rect.top = 0;
	creditWin->rect.right = SCR_WIDTH - 1;
	creditWin->rect.bottom = SCR_HEIGHT - 1;

	if (creditWin->buff != 0) {
		creditWin->buff = FReallocateMemory(creditWin->buff, SCR_WIDTH * SCR_HEIGHT);
		creditWinBuff = creditWin->buff;
		memset(creditWin->buff, 0, SCR_WIDTH * SCR_HEIGHT);
	}

	if (creditBuff1 != 0) {
		creditBuff1 = FReallocateMemory(creditBuff1, 640 * SCR_HEIGHT);
		memset(creditBuff1, 0, 640 * SCR_HEIGHT);
	}


	if (creditBuff2 != 0) {
		creditBuff2 = FReallocateMemory(creditBuff2, 640 * SCR_HEIGHT);
		memset(creditBuff2, 0, 640 * SCR_HEIGHT);

		UINT offset = 0;
		if (SCR_HEIGHT > oldHeight) {
			offset = 640 * SCR_HEIGHT - 640 * oldHeight;
			memcpy(creditBuff2 + offset, oldBuff, 640 * oldHeight);
		}
		else {
			offset = 640 * oldHeight - 640 * SCR_HEIGHT;
			memcpy(creditBuff2, oldBuff + offset, 640 * SCR_HEIGHT);
		}
		creditBuff2_top = &creditBuff2[640];
		creditBuff2_bottom = &creditBuff2[640 * (SCR_HEIGHT - 1)];
	}

	if (oldBuff)
		delete[] oldBuff;
}


//_________________
void CreditsFixes() {

	FuncReplace32(0x4279A9, 0x09B0A7, (DWORD)&destroy_win_credits);

	FuncReplace32(0x4276AF, 0x0AB410, (DWORD)&memcpy_credit_scrn_buff);
	FuncReplace32(0x4278D3, 0x0AB1EC, (DWORD)&memcpy_credit_scrn_buff);

	MemWrite8(0x4276A7, 0x8B, 0x90);
	MemWrite16(0x4276A8, 0x24BC, 0xE890);
	FuncWrite32(0x4276AA, 0x0248, (DWORD)&copy_textbuff_fix);

	MemWrite8(0x4275FD, 0x8B, 0x90);
	MemWrite16(0x4275FE, 0x2494, 0x158B);
	MemWrite32(0x427600, 0x0228, (DWORD)&creditBuff2_bottom);

	MemWrite8(0x42768B, 0x8B, 0x90);
	MemWrite16(0x42768C, 0x2494, 0x158B);
	MemWrite32(0x42768E, 0x0240, (DWORD)&creditBuff2_top);

	MemWrite8(0x4277BC, 0x8B, 0x90);
	MemWrite16(0x4277BD, 0x24B4, 0x358B);
	MemWrite32(0x4277BF, 0x025C, (DWORD)&creditWinBuff);

	MemWrite16(0x427981, 0x848B, 0x9090);
	MemWrite8(0x427983, 0x24, 0xA1);
	MemWrite32(0x427984, 0x0250, (DWORD)&creditBuff1);

	MemWrite16(0x427692, 0x848B, 0x9090);
	MemWrite8(0x427694, 0x24, 0xA1);
	MemWrite32(0x427695, 0x0254, (DWORD)&creditBuff2);

	MemWrite16(0x42788F, 0x848B, 0x9090);
	MemWrite8(0x427891, 0x24, 0xA1);
	MemWrite32(0x427892, 0x0254, (DWORD)&creditBuff2);

	MemWrite8(0x427896, 0x8B, 0x90);
	MemWrite16(0x427897, 0x24BC, 0x3D8B);
	MemWrite32(0x427899, 0x0254, (DWORD)&creditBuff2_bottom);
	MemWrite32(0x4278A6, 306560, 0);

	MemWrite16(0x4278CB, 0x848B, 0x9090);
	MemWrite8(0x4278CD, 0x24, 0xA1);
	MemWrite32(0x4278CE, 0x0254, (DWORD)&creditBuff2);

	MemWrite16(0x427975, 0x848B, 0x9090);
	MemWrite8(0x427977, 0x24, 0xA1);
	MemWrite32(0x427978, 0x0254, (DWORD)&creditBuff2);

	FuncReplace32(0x427380, 0x087DBC, (DWORD)&allocate_credit_scrn_buff1);

	FuncReplace32(0x4273AA, 0x0AB762, (DWORD)&clear_credit_scrn_buff);

	FuncReplace32(0x427442, 0x087CFA, (DWORD)&allocate_credit_scrn_buff2);

	FuncReplace32(0x42745F, 0x0AB6AD, (DWORD)&clear_credit_scrn_buff);

	MemWrite32(0x4273EF, 480, SCR_HEIGHT);

	FuncReplace32(0x427349, 0x09B4DB, (DWORD)&CreditsWinSetup);

	FuncReplace32(0x4276E8, 0x096DA8, (DWORD)&CreditsBltBuff1);
	FuncReplace32(0x427714, 0x096DAC, (DWORD)&CreditsBltWinBuff);

	FuncReplace32(0x42790A, 0x096B86, (DWORD)&CreditsBltBuff1);
	FuncReplace32(0x427936, 0x096B8A, (DWORD)&CreditsBltWinBuff);
}
