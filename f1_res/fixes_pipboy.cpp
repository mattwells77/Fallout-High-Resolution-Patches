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


//___________________________________________
void __declspec(naked) pipboy_mouse_fix(void) {

	__asm {
		push ecx
		push ebx

		push edx
		push eax
		call F_GetMousePos
		mov eax, pWinRef_Pipboy
		push dword ptr ds : [eax]
		call GetWinStruct
		add esp, 0x4
		mov ecx, eax
		pop eax//*mouse_x
		pop edx//*mouse_y
		mov ebx, dword ptr ds : [ecx + 0x8]
		sub[eax], ebx //mouse_x - pipboy->rect_>left
		mov ebx, dword ptr ds : [ecx + 0xC]
		sub[edx], ebx //mouse_y - pipboy->rect_>top

		pop ebx
		pop ecx
		ret
	}
}


//________________
void PipBoyFixes() {

	FuncReplace32(0x486F6C, 0x02E828, (DWORD)&pipboy_mouse_fix);
	FuncReplace32(0x489F6E, 0x02B826, (DWORD)&pipboy_mouse_fix);
}
