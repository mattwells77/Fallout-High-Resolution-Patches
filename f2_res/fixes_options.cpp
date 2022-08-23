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
#include "F_Msg.h"


MSGList* optionsMsgLst = nullptr;
F_VOID_FUNC_VOID F_OptionsWin = nullptr;


//__________________
void OptionsWindow() {

	if (!LoadMsgList(optionsMsgLst, "game\\options.msg"))
		return;
	bool isMouse = IsMouseHidden();
	F_ShowMouse();
	F_OptionsWin();
	if (isMouse)
		F_HideMouse;
}


//___________________________________________
void __declspec(naked) option_mouse_fix(void) {

	__asm {
		push edx
		push eax
		call F_GetMousePos
		mov eax, pWinRef_Options
		push dword ptr ds : [eax]
		call GetWinStruct
		add esp, 0x4
		mov ecx, eax
		pop eax//*mouse_x
		pop edx//*mouse_y
		mov ebx, dword ptr ds : [ecx + 0x8]
		sub[eax], ebx //mouse_x - options->rect_>left
		mov ebx, dword ptr ds : [ecx + 0xC]
		sub[edx], ebx //mouse_y - options->rect_>top
		ret
	}
}


//_________________________
void OptionsFixes_CH(void) {

	optionsMsgLst = (MSGList*)0x673D68;

	F_OptionsWin = (void(__cdecl*)(void))0x48FB40;

	//Dials Pos Fix
	FuncWrite32(0x490053, 0x03BF7B, (DWORD)&option_mouse_fix);
	//Sliders Pos Fix
	FuncWrite32(0x4906C7, 0x03B907, (DWORD)&option_mouse_fix);
}


//__________________________________
void OptionsFixes_MULTI(int region) {

	optionsMsgLst = (MSGList*)FixAddress(0x6637E8);

	F_OptionsWin = (void(__cdecl*)(void))FixAddress(0x490798);


	//Dials Pos Fix
	FuncReplace32(0x490EAD, 0x039B2B, (DWORD)&option_mouse_fix);
	//Sliders Pos Fix
	FuncReplace32(0x491547, 0x039491, (DWORD)&option_mouse_fix);
}


//______________________________
void OptionsFixes(DWORD region) {

	if (region == 4)
		OptionsFixes_CH();
	else
		OptionsFixes_MULTI(region);
}
