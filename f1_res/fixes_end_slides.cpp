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

//BYTE slidePal[768];
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
	darkColour = FindDarkPalRef(pal);
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


//___________________
void EndSlidesFixes() {

	MemWrite8(0x439976, 0xBA, 0xE8);
	FuncWrite32(0x439977, 480, (DWORD)&fix_sub_ypos);

	MemWrite8(0x439A15, 0xB9, 0xE8);
	FuncWrite32(0x439A16, 640, (DWORD)&fix_sub_width1);

	MemWrite8(0x439A4C, 0xB9, 0xE8);
	FuncWrite32(0x439A4D, 640, (DWORD)&fix_sub_width1);

	MemWrite8(0x4399FC, 0xBA, 0xE8);
	FuncWrite32(0x4399FD, 640, (DWORD)&fix_sub_width2);

	//Ending-Slides-window
	FuncReplace32(0x438FEA, 0xFFFDF9A2, (DWORD)&get_slide_frm_vars);
	FuncReplace32(0x4391DF, 0x0852B1, (DWORD)&DrawScrollingSlide);

	FuncReplace32(0x43940B, 0xFFFDF581, (DWORD)&get_slide_frm_vars);
	FuncReplace32(0x439522, 0x084F6E, (DWORD)&DrawStillSlide);

	FuncReplace32(0x43949A, 0x04C166, (DWORD)&slide_bg_colour);

	MemWrite8(0x439446, 0xE8, 0x90);
	MemWrite32(0x439447, FixAddress(0x4BE494) - FixAddress(0x439447) - 4, 0x90909090);
	MemWrite8(0x439453, 0xE8, 0x90);
	MemWrite32(0x439454, FixAddress(0x4C3548) - FixAddress(0x439454) - 4, 0x90909090);

	FuncReplace32(0x4392AF, 0x04C3C5, (DWORD)&scroll_slide_bg_colour);

	FuncReplace32(0x438E19, 0x089A0B, (DWORD)&CreateWin_EndSlides);

	winBuff_EndSlides = (BYTE**)FixAddress(0x56EED8);
}
