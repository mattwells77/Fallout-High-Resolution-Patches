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


//_______________________
void PipBoyFixes_CH(void) {

	FuncWrite32(0x495E43, 0x03618B, (DWORD)&pipboy_mouse_fix);
	FuncWrite32(0x498EAB, 0x033123, (DWORD)&pipboy_mouse_fix);
}


//__________________________
void PipBoyFixes_MULTI(void) {

	FuncReplace32(0x497093, 0x033945, (DWORD)&pipboy_mouse_fix);
	FuncReplace32(0x49A1B7, 0x030821, (DWORD)&pipboy_mouse_fix);
}


//____________________________
void PipBoyFixes(DWORD region) {

	if (region == 4)
		PipBoyFixes_CH();
	else
		PipBoyFixes_MULTI();
}
