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
#include "F_Art.h"
#include "F_Windows.h"
#include "configTools.h"

int END_SLIDE_SIZE = 0;

LONG slideWidth = 0;
LONG slideHeight = 0;

BYTE darkColour = 0;

BYTE** winBuff_EndSlides = nullptr;



//________________________________________________________________________________________________________________
void DrawStillSlide(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	memset(toBuff, darkColour, SCR_WIDTH * SCR_HEIGHT);

	if (END_SLIDE_SIZE == 1 || slideWidth > SCR_WIDTH || slideHeight > SCR_HEIGHT)
		MemBlt8Stretch(fromBuff, slideWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, true, true);
	else if (END_SLIDE_SIZE == 2)
		MemBlt8Stretch(fromBuff, slideWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, false, false);
	else {
		toBuff += ((SCR_WIDTH >> 1) - (slideWidth >> 1)) + (((SCR_HEIGHT >> 1) - (slideHeight >> 1)) * SCR_WIDTH);
		MemBlt8(fromBuff, slideWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH);
	}
}


//____________________________________________________________________________________________________________________
void DrawScrollingSlide(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	if (END_SLIDE_SIZE == 1)
		MemBlt8Stretch(fromBuff, subWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, true, true);
	else if (END_SLIDE_SIZE == 2)
		MemBlt8Stretch(fromBuff, subWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, false, false);
	else {
		toBuff += ((SCR_WIDTH >> 1) - (subWidth >> 1)) + (((SCR_HEIGHT >> 1) - (slideHeight >> 1)) * SCR_WIDTH);
		MemBlt8(fromBuff, subWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH);
	}
}


//_________________________________
void ScrollSlideBGColour(BYTE* pal) {

	darkColour = FindDarkPalRef(pal);
	FSetPalette(pal);

	WinStruct* win = GetWinStruct(*pWinRef_EndSlides);
	memset(win->buff, darkColour, win->width * win->height);
}


//_________________________________________________
void __declspec(naked) scroll_slide_bg_colour(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		push eax
		call ScrollSlideBGColour
		add esp, 0x4

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//____________________________________________
void SlideBGColour(BYTE* slideBuff, BYTE* pal) {

	WinStruct* win = GetWinStruct(*pWinRef_EndSlides);
	//win->clearColour= FindDarkPalRef( pal);
	darkColour = FindDarkPalRef(pal);
	//memset(win->buff, colour, win->width*win->height);
	memset(win->buff, darkColour, win->width * win->height);
	Fix8BitColours(slideBuff, slideWidth, slideHeight, slideWidth, darkColour);

	BYTE* toBuff = win->buff;
	if (END_SLIDE_SIZE == 1 || slideWidth > SCR_WIDTH || slideHeight > SCR_HEIGHT)
		MemBlt8Stretch(slideBuff, slideWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, true, true);
	else if (END_SLIDE_SIZE == 2)
		MemBlt8Stretch(slideBuff, slideWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, false, false);
	else {
		toBuff += ((SCR_WIDTH >> 1) - (slideWidth >> 1)) + (((SCR_HEIGHT >> 1) - (slideHeight >> 1)) * SCR_WIDTH);
		MemBlt8(slideBuff, slideWidth, slideHeight, slideWidth, toBuff, SCR_WIDTH);
	}
	RedrawWin(*pWinRef_EndSlides);
	FFadeToPalette(pal);
}


//__________________________________________
void __declspec(naked) slide_bg_colour(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		push eax
		push edi
		call SlideBGColour
		add esp, 0x8

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
FRMhead* GetSlideFrmVars(DWORD fid, DWORD* frmObj) {

	FRMhead* frm = F_GetFrm(fid, frmObj);

	slideWidth = F_GetFrmFrameWidth(frm, 0, 0);
	slideHeight = F_GetFrmFrameHeight(frm, 0, 0);

	return frm;
}


//_____________________________________________
void __declspec(naked) get_slide_frm_vars(void) {

	__asm {
		push ebx
		push ecx

		push edx
		push eax
		call GetSlideFrmVars
		add esp, 0x8

		pop ecx
		pop ebx
		ret
	}
}


//____________________________________________________________
int _stdcall CreateWin_EndSlides(DWORD colour, DWORD winFlags) {

	END_SLIDE_SIZE = ConfigReadInt("STATIC_SCREENS", "END_SLIDE_SIZE", 0);
	return Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);
}


//_____________________
void ReSize_EndSlides() {

	if (*pWinRef_EndSlides == -1)
		return;

	WinStruct* win = GetWinStruct(*pWinRef_EndSlides);
	if (!win)
		return;

	win->width = SCR_WIDTH;
	win->height = SCR_HEIGHT;
	win->rect.left = 0;
	win->rect.top = 0;
	win->rect.right = SCR_WIDTH - 1;
	win->rect.bottom = SCR_HEIGHT - 1;
	win->buff = FReallocateMemory(win->buff, SCR_WIDTH * SCR_HEIGHT);
	memset(win->buff, win->colour, SCR_WIDTH * SCR_HEIGHT);

	*winBuff_EndSlides = win->buff;
}


//_______________________________________
void __declspec(naked) fix_sub_ypos(void) {

	__asm {
		mov edx, SCR_HEIGHT
		ret
	}
}


//_________________________________________
void __declspec(naked) fix_sub_width1(void) {

	__asm {
		mov ecx, SCR_WIDTH
		ret
	}
}


//_________________________________________
void __declspec(naked) fix_sub_width2(void) {

	__asm {
		mov edx, dword ptr ss : [esp + 0xA0]
		mov eax, SCR_WIDTH
		imul eax, edx
		mov edx, SCR_WIDTH
		ret
	}
}


//__________________________
void EndSlidesFixes_CH(void) {

	//Ending-Slides-subs-----------------------------------------------
	MemWrite8(0x43FAB7, 0xBA, 0xE8);
	FuncWrite32(0x43FAB8, 480, (DWORD)&fix_sub_ypos);

	MemWrite8(0x43FB56, 0xB9, 0xE8);
	FuncWrite32(0x43FB57, 640, (DWORD)&fix_sub_width1);

	MemWrite8(0x43FB8D, 0xB9, 0xE8);
	FuncWrite32(0x43FB8E, 640, (DWORD)&fix_sub_width1);

	MemWrite8(0x43FB3D, 0xBA, 0xE8);
	FuncWrite32(0x43FB3E, 640, (DWORD)&fix_sub_width2);


	FuncWrite32(0x43F14B, 0xFFFD9FA9, (DWORD)&get_slide_frm_vars);

	FuncWrite32(0x43F318, 0x099BA0, (DWORD)&DrawScrollingSlide);

	FuncWrite32(0x43F65E, 0x09985A, (DWORD)&DrawStillSlide);

	FuncWrite32(0x43F547, 0xFFFD9BAD, (DWORD)&get_slide_frm_vars);

	FuncWrite32(0x43F5D6, 0x0532AA, (DWORD)&slide_bg_colour);

	MemWrite8(0x43F582, 0xE8, 0x90);
	MemWrite32(0x43F583, 0x099935, 0x90909090);

	MemWrite8(0x43F58F, 0xE8, 0x90);
	MemWrite32(0x43F590, 0x09DCE4, 0x90909090);

	FuncWrite32(0x43F3ED, 0x053507, (DWORD)&scroll_slide_bg_colour);

	FuncWrite32(0x43EF4E, 0x09D16B, (DWORD)&CreateWin_EndSlides);

	winBuff_EndSlides = (BYTE**)FixAddress(0x580BF0);
}


//_____________________________
void EndSlidesFixes_MULTI(void) {

	//Ending-Slides-subs--------------------------------------------
	MemWrite8(0x440583, 0xBA, 0xE8);
	FuncWrite32(0x440584, 480, (DWORD)&fix_sub_ypos);

	MemWrite8(0x440622, 0xB9, 0xE8);
	FuncWrite32(0x440623, 640, (DWORD)&fix_sub_width1);

	MemWrite8(0x440659, 0xB9, 0xE8);
	FuncWrite32(0x44065A, 640, (DWORD)&fix_sub_width1);

	MemWrite8(0x440609, 0xBA, 0xE8);
	FuncWrite32(0x44060A, 640, (DWORD)&fix_sub_width2);


	FuncReplace32(0x43FC17, 0xFFFD9545, (DWORD)&get_slide_frm_vars);

	FuncReplace32(0x43FDE4, 0x0938EC, (DWORD)&DrawScrollingSlide);

	FuncReplace32(0x440013, 0xFFFD9149, (DWORD)&get_slide_frm_vars);

	FuncReplace32(0x44012A, 0x0935A6, (DWORD)&DrawStillSlide);

	FuncReplace32(0x4400A2, 0x053A2E, (DWORD)&slide_bg_colour);

	MemWrite8(0x44004E, 0xE8, 0x90);
	MemWrite32(0x44004F, FixAddress(0x093681), 0x90909090);

	MemWrite8(0x44005B, 0xE8, 0x90);
	MemWrite32(0x44005C, FixAddress(0x096EFC), 0x90909090);

	FuncReplace32(0x43FEB9, 0x053C8B, (DWORD)&scroll_slide_bg_colour);

	FuncReplace32(0x43FA1E, 0x096816, (DWORD)&CreateWin_EndSlides);

	winBuff_EndSlides = (BYTE**)FixAddress(0x570BF0);
}


//_______________________________
void EndSlidesFixes(DWORD region) {

	if (region == 4)
		EndSlidesFixes_CH();
	else
		EndSlidesFixes_MULTI();
}
