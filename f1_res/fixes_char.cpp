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
#include "F_Text.h"


//_______________________________________
void GetCharMouse(LONG* pxPos, LONG* pyPos) {

	WinStruct* charWin = GetWinStruct(*pWinRef_Char);
	F_GetMousePos(pxPos, pyPos);
	*pxPos -= charWin->rect.left;
	*pyPos -= charWin->rect.top;
}


//__________________________________
void __declspec(naked) h_mouse_fix() {

	__asm {
		push ebx
		push ecx
		push esi

		push edx//char mouse y pos address
		push eax//char mouse x pos address
		call GetCharMouse
		add esp, 0x8

		pop esi
		pop ecx
		pop ebx
		ret
	}

}



//____________________________________________________________________________
int CharSubWin(int x, int y, int width, int height, int colour, int winFlags) {
	WinStruct* charWin = GetWinStruct(*pWinRef_Char);
	return Win_Create(x + charWin->rect.left, y + charWin->rect.top, width, height, colour, winFlags);
}


//_______________________________________
void __declspec(naked) h_char_sub_panel() {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]

		push ecx
		push ebx
		push edx
		push eax
		call CharSubWin
		add esp, 0x18
		ret 0x8
	}

}


//__________________________________________
void __declspec(naked) char_sub_menu_fix_2() {

	__asm {
		mov eax, pWinRef_Char
		push dword ptr ds : [eax]
		call GetWinStruct
		add esp, 0x4
		test eax, eax
		je noCharWin
		mov ecx, dword ptr ds : [eax + 0x8]//charWin->rect.left
		add dword ptr ss : [esp + 0x200] , ecx//subWin xPos
		mov ecx, dword ptr ds : [eax + 0x0C]//charWin->rect.top
		add dword ptr ss : [esp + 0x204] , ecx//subWin yPos
		noCharWin :
		call GetFont
			ret
	}

}

//__________________________________________
void __declspec(naked) char_sub_menu_fix_1() {

	__asm {
		mov eax, pWinRef_Char
		push dword ptr ds : [eax]
		call GetWinStruct
		add esp, 0x4
		test eax, eax
		je noCharWin
		mov ecx, dword ptr ds : [eax + 0x8]//charWin->rect.left
		add dword ptr ss : [esp + 0x25C] , ecx//subWin xPos
		mov ecx, dword ptr ds : [eax + 0x0C]//charWin->rect.top
		add dword ptr ss : [esp + 0x260] , ecx//subWin yPos
		noCharWin :
		call GetFont
			ret
	}

}



//__________________
void CharScrnFixes() {

	//----------------MOUSE MOVE--------------------
	FuncReplace32(0x435110, 0x080684, (DWORD)&h_mouse_fix);
	FuncReplace32(0x4351D8, 0x0805BC, (DWORD)&h_mouse_fix);
	FuncReplace32(0x436A50, 0x07ED44, (DWORD)&h_mouse_fix);


	//-------------CREATE NEW CHAR OPTIONS PANEL POSITION------------------
	FuncReplace32(0x432407, 0x09041D, (DWORD)&h_char_sub_panel);

	//----------------------PERK SCRN POSITION----------------------------
	FuncReplace32(0x4364B1, 0x08C373, (DWORD)&h_char_sub_panel);

	//name sub panel
 	FuncReplace32(0x43120B, 0x091619, (DWORD)&h_char_sub_panel);

	//age sub panel
 	FuncReplace32(0x4315AA, 0x09127A, (DWORD)&h_char_sub_panel);

	//sex sub panel
 	FuncReplace32(0x431B18, 0x090D0C, (DWORD)&h_char_sub_panel);

	//load char sub menu
 	FuncReplace32(0x41CDE6, 0x0A50DA, (DWORD)&char_sub_menu_fix_2);

	//save char, print new char, print ingame char sub menues
 	FuncReplace32(0x41D94A, 0x0A4576, (DWORD)&char_sub_menu_fix_1);
}
