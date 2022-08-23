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
#include "configTools.h"
#include "WinFall.h"


void* pFUNC_PC_RUN = nullptr;
static LONG destHexPos = 0;


//_______________________________________________
void __declspec(naked) double_click_running(void) {

	//in PC_WALK(EAX actionPoints) function.
	__asm {
		cmp eax, destHexPos
		jne exitFunc
		mov eax, dword ptr ss : [esp + 0x4]//actionPoints
		jmp pFUNC_PC_RUN
		exitFunc :
		mov destHexPos, eax
		mov eax, 0x2
		ret
	}
}



//_______________
void OtherFixes() {

	if (ConfigReadInt("OTHER_SETTINGS", "DOUBLE_CLICK_RUNNING", 1)) {

		pFUNC_PC_RUN = (void*)FixAddress(0x417A5C);
		MemWrite8(0x417A33, 0xB8, 0xE8);
		FuncWrite32(0x417A34, 0x02, (DWORD)&double_click_running);
	}

	//Bypass hard drive space check - can cause a false error. 
	//"Not enough free hard disk space.  Fallout requires at least %.1f megabytes of free hard disk space."
	MemWrite16(0x43D84C, 0x5153, 0xC031);//xor eax, eax
	MemWrite8(0x43D84E, 0x52, 0xC3);//return
}
