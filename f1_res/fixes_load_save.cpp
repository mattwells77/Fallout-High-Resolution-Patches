/*
The MIT License (MIT)
Copyright ? 2022 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the ?Software?), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ?AS IS?, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
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

WinStruct* lsWinStruct = nullptr;

//____________________________________________________________________________
LONG LoadSaveWinSetup(DWORD width, DWORD height, DWORD colour, DWORD winFlags) {

	LONG x = 0, y = 0;
	AdjustWinPosToGame(width, height, &x, &y);

	LONG winRef = Win_Create(x, y, width, height, colour, winFlags);
	lsWinStruct = GetWinStruct(winRef);
	return winRef;
}


//_______________________________________
void __declspec(naked) h_load_save_scrn() {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]
		push ecx
		push ebx
		call LoadSaveWinSetup
		add esp, 0x10
		ret 0x8
	}
}


//____________________________________________
void __declspec(naked) h_load_save_mouse(void) {

	__asm {
		sub edx, 0x4f
		mov eax, lsWinStruct
		sub edx, dword ptr ds : [eax + 0x0C]
		mov eax, edx
		ret
	}
}


//___________________________________________________________________________________
int SaveNameBoxWin(int x, int y, int width, int height, DWORD colour, DWORD winFlags) {

	return Win_Create(lsWinStruct->rect.left + x, lsWinStruct->rect.top + y, width, height, colour, winFlags);
}


//__________________________________________
void __declspec(naked) h_save_name_box_win() {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]

		push ecx
		push ebx
		push edx
		push eax
		call SaveNameBoxWin
		add esp, 0x18
		ret 0x8
	}
}


//_________________________________________
void __declspec(naked) h_save_pic_stretch() {
	//stretch_blt(EAX fBuff, EDX subWidth, EBX subHeight, ECX fWidth, Arg1 tBuff, Arg2 tSubWidth, Arg3 tHeight, Arg4 tWidth)
	//void MemBlt8Stretch(BYTE *fromBuff, int subWidth, int subHeight, int fromWidth, BYTE *toBuff, int toWidth, int toHeight, bool ARatio, bool centred) {
	__asm {
		push 0//centred
		push 0//ARatio
		push dword ptr ss : [esp + 0x14]//toHeight
		push dword ptr ss : [esp + 0x14]//toSubWidth 'same as tWidth'
		push dword ptr ss : [esp + 0x14]//*toBuff

		mov edx, eax//mov fBuff to edx
		mov eax, pWinRef_GameArea
		push dword ptr ds : [eax]
		call GetWinStruct
		add esp, 0x4

		push dword ptr ds : [eax + 0x18]//gameWin->width
		push dword ptr ds : [eax + 0x1C]//gameWin->height
		push dword ptr ds : [eax + 0x18]//gameWin->width
		push dword ptr ds : [eax + 0x2C]//gameWin->buff

		//push edx
		call MemBlt8Stretch
		add esp, 0x24

		ret 0x10
	}
}


//__________________
void LoadSaveFixes() {

	//SAVEGAME NAME BOX POSITION
	FuncReplace32(0x4711E5, 0x05163F, (DWORD)&h_save_name_box_win);

	//QUICK SAVE GAME PIC WIDTH & HEIGHT
 	FuncReplace32(0x46EB9C, 0x04F610, (DWORD)&h_save_pic_stretch);

	//LOAD SAVE GAME PIC LARGE
 	FuncReplace32(0x46F97C, 0x04E830, (DWORD)&h_save_pic_stretch);


	FuncReplace32(0x46FA76, 0x052DAE, (DWORD)&h_load_save_scrn);

	MemWrite8(0x46F18C, 0x83, 0xE8);
	FuncWrite32(0x46F18D, 0xD0894FEA, (DWORD)&h_load_save_mouse);

	MemWrite8(0x46E3DC, 0x83, 0xE8);
	FuncWrite32(0x46E3DD, 0xD0894FEA, (DWORD)&h_load_save_mouse);
}
