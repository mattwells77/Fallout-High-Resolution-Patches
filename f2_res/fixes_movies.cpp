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
#include "configTools.h"
#include "WinFall.h"
#include "F_Mapper.h"


DWORD MOVIE_SIZE = 0;
DWORD* pSUBTITLES_FLAG = nullptr;

LONG* pMovieX = nullptr;
LONG* pMovieY = nullptr;
LONG* pMovieWidth = nullptr;
LONG* pMovieHeight = nullptr;

LONG* pMovieWinX = nullptr;
LONG* pMovieWinY = nullptr;
LONG* pMovieWinWidth = nullptr;
LONG* pMovieWinHeight = nullptr;

RECT* pMovieWinRect = nullptr;
RECT* pMovieWinRect2 = nullptr;

LONG* pSubsWinWidth = nullptr;
LONG* pSubsWinHeight = nullptr;

LONG* pSubFontRef = nullptr;


//__________________________________________________________________________________
int MovieWinSetup(int x, int y, int width, int height, DWORD colour, DWORD winFlags) {

	if (*pWinRef_Movies != -1)
		return -1;
	return *pWinRef_Movies = Win_Create(x, y, width, height, colour, winFlags);
}


//______________________________________
void __declspec(naked) movie_win_setup() {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]
		push ecx
		push ebx
		push edx
		push eax
		call MovieWinSetup
		add esp, 0x18
		ret 0x8
	}
}


//_______________________________________
void __declspec(naked) h_movie_kill(void) {

	__asm {
		mov eax, pWinRef_Movies
		mov eax, dword ptr ds : [eax]
		push eax
		call DestroyWin
		add esp, 0x4
		mov eax, pWinRef_Movies
		mov dword ptr ds : [eax] , -1
		///
		call ReDrawViewWin
		///
		ret
	}
}


//________________
void ResizeMovie() {

	if (*pWinRef_Movies != -1)
		DestroyWin(*pWinRef_Movies);

	int subsHeight = 0;
	if (*pSUBTITLES_FLAG == 1)
		subsHeight = *pSubsWinHeight;

	if (MOVIE_SIZE == 1 || SCR_WIDTH < *pMovieWidth || SCR_HEIGHT < *pMovieHeight) {//scale - maintain aspect RO
		float movieRO = (float)*pMovieWidth / *pMovieHeight;
		float winRO = (float)SCR_WIDTH / SCR_HEIGHT;

		if (movieRO >= winRO) {
			*pMovieWinX = 0;
			*pMovieWinWidth = SCR_WIDTH;
			*pMovieWinHeight = (int)(*pMovieWinWidth / movieRO);
			*pMovieWinY = (SCR_HEIGHT - *pMovieWinHeight) / 2;

			if (SCR_HEIGHT - subsHeight < *pMovieWinY + *pMovieWinHeight) {
				if (subsHeight < *pMovieWinY)
					*pMovieWinY -= subsHeight;
				else {
					*pMovieWinHeight -= (subsHeight - *pMovieWinY);
					*pMovieWinWidth = (int)(*pMovieWinHeight * movieRO);
					*pMovieWinX = (SCR_WIDTH - *pMovieWinWidth) / 2;
					*pMovieWinY = 0;
				}
			}
		}
		else {
			*pMovieWinY = 0;
			*pMovieWinHeight = SCR_HEIGHT - subsHeight;
			*pMovieWinWidth = (int)(*pMovieWinHeight * movieRO);
			*pMovieWinX = (SCR_WIDTH - *pMovieWinWidth) / 2;
		}
	}
	else if (MOVIE_SIZE == 0 || MOVIE_SIZE != 2) {//original size
		*pMovieWinWidth = *pMovieWidth;
		*pMovieWinHeight = *pMovieHeight;
		*pMovieWinX = (SCR_WIDTH - *pMovieWinWidth) / 2;
		*pMovieWinY = (SCR_HEIGHT - *pMovieWinHeight) / 2;
	}

	*pSubsWinWidth = SCR_WIDTH;

	*pWinRef_Movies = Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0x10 | F_WIN_FRONT);

	WinStruct* win = GetWinStruct(*pWinRef_Movies);

	pMovieWinRect->left = win->rect.left;
	pMovieWinRect->top = win->rect.top;
	pMovieWinRect->right = win->rect.right;
	pMovieWinRect->bottom = win->rect.bottom;

	pMovieWinRect2->left = 0;
	pMovieWinRect2->top = 0;
	pMovieWinRect2->right = win->rect.right;
	pMovieWinRect2->bottom = win->rect.bottom;

	win = nullptr;
}


//_______________________________________
void __declspec(naked) resize_movie(void) {

	__asm {
		pushad
		call ResizeMovie
		popad
		mov ecx, 0x1B
		ret
	}
}


//________________
void ResizeMovies() {

	if (*pWinRef_Movies != -1)
		ResizeMovie();
}


//__________________________________________
void __declspec(naked) get_scrn_height(void) {

	__asm {
		mov edx, SCR_HEIGHT
		ret
	}
}


//______________________
void MovieFixes_CH(void) {

	pSubsWinWidth = (LONG*)0x6493C8;
	pSubsWinHeight = (LONG*)FixAddress(0x649420);

	pSUBTITLES_FLAG = (DWORD*)0x673EF4;

	pMovieWinX = (LONG*)FixAddress(0x649430);
	pMovieWinY = (LONG*)FixAddress(0x649434);
	pMovieWinWidth = (LONG*)0x649414;
	pMovieWinHeight = (LONG*)0x649404;

	pMovieWinRect = (RECT*)0x649390;
	pMovieWinRect2 = (RECT*)0x6493A0;

	pMovieX = (LONG*)FixAddress(0x6C456B);
	pMovieY = (LONG*)FixAddress(0x6C456F);
	pMovieWidth = (LONG*)0x6C424C;
	pMovieHeight = (LONG*)0x6C457F;


	pSubFontRef = (LONG*)0x44DFD9;

	//MOVIE SCALING###################

	//SETS MOVIES TO BUFFERED MODE -ALLOWS FOR VIDEO IN 16BIT
	MemWrite8(0x486B75, 0x74, 0xEB);


	//FIX SCR_HEIGHT WHEN SETTING SUBTITLE HEIGHT
	MemWrite8(0x486973, 0xBA, 0xE8);
	FuncWrite32(0x486974, 480, (DWORD)&get_scrn_height);

	FuncWrite32(0x44E16D, 0x08E24F, (DWORD)&h_movie_kill);

	MemWrite16(0x501E94, 0x870F, 0x9090);
	MemWrite32(0x501E96, 0x033D, 0x90909090);

	MemWrite16(0x501EB1, 0x870F, 0x9090);
	MemWrite32(0x501EB3, 0x0320, 0x90909090);

	MemWrite8(0x502B90, 0xB9, 0xE8);
	FuncWrite32(0x502B91, 0x1B, (DWORD)&resize_movie);

	MemWrite16(0x4864FB, 0x2D89, 0x9090);
	MemWrite32(0x4864FD, 0x5293A8, 0x90909090);

	FuncWrite32(0x44DED8, 0x08E1E1, (DWORD)&movie_win_setup);

	///00485D47  |.  8935 CC936400 MOV DWORD PTR DS:[6493CC],ESI            ; arg2 moveWidth -this is wrong and should be movieHeight-
	MemWrite8(0x485D48, 0x35, 0x2D);

	MemWrite32(0x48697A, FixAddress(0x6493F0), (DWORD)pMovieWinY);

	MemWrite32(0x4869A2, FixAddress(0x6493F0), (DWORD)pMovieWinY);

	MemWrite32(0x486980, FixAddress(0x6493E4), (DWORD)pMovieWinHeight);

	MemWrite32(0x48699B, FixAddress(0x6493E4), (DWORD)pMovieWinHeight);
}

//_______________________________
void MovieFixes_MULTI(int region) {

	pSubsWinWidth = (LONG*)FixAddress(0x638E48);
	pSubsWinHeight = (LONG*)FixAddress(0x638EA0);

	pSUBTITLES_FLAG = (DWORD*)FixAddress(0x663974);

	pMovieWinX = (LONG*)FixAddress(0x638EB0);
	pMovieWinY = (LONG*)FixAddress(0x638EB4);
	pMovieWinWidth = (LONG*)FixAddress(0x638E94);
	pMovieWinHeight = (LONG*)FixAddress(0x638E84);

	pMovieWinRect = (RECT*)FixAddress(0x638E10);
	pMovieWinRect2 = (RECT*)FixAddress(0x638E20);

	pMovieX = (LONG*)FixAddress(0x6B401B);
	pMovieY = (LONG*)FixAddress(0x6B401F);
	pMovieWidth = (LONG*)FixAddress(0x6B3CFC);
	pMovieHeight = (LONG*)FixAddress(0x6B402F);

	pSubFontRef = (LONG*)FixAddress(0x44E8E2);

	//MOVIE SCALING###################

	//SETS MOVIES TO BUFFERED MODE -ALLOWS FOR VIDEO IN 16BIT
	MemWrite8(0x487781, 0x74, 0xEB);

	//FIX SCR_HEIGHT WHEN SETTING SUBTITLE HEIGHT
	MemWrite8(0x48757F, 0xBA, 0xE8);
	FuncWrite32(0x487580, 480, (DWORD)&get_scrn_height);

	FuncReplace32(0x44EA76, 0x0879EE, (DWORD)&h_movie_kill);

	MemWrite16(0x4F5044, 0x870F, 0x9090);
	MemWrite32(0x4F5046, 0x033D, 0x90909090);

	MemWrite16(0x4F5061, 0x870F, 0x9090);
	MemWrite32(0x4F5063, 0x0320, 0x90909090);

	MemWrite8(0x4F5D40, 0xB9, 0xE8);
	FuncWrite32(0x4F5D41, 0x1B, (DWORD)&resize_movie);

	//prevent movie winRef set to -1 as it has not been destroyed
	MemWrite16(0x48710B, 0x2D89, 0x9090);
	MemWrite32(0x48710D, FixAddress(0x5195B8), 0x90909090);

	FuncReplace32(0x44E7E4, 0x087A50, (DWORD)&movie_win_setup);

	///00486957  |.  8935 4C8E6300         MOV DWORD PTR DS:[638E4C],ESI            ; arg2 moveWidth -this is wrong and should be movieHeight-
	MemWrite8(0x486958, 0x35, 0x2D);

	MemWrite32(0x487586, FixAddress(0x638E70), (DWORD)pMovieWinY);

	MemWrite32(0x4875AE, FixAddress(0x638E70), (DWORD)pMovieWinY);

	MemWrite32(0x48758C, FixAddress(0x638E64), (DWORD)pMovieWinHeight);

	MemWrite32(0x4875A7, FixAddress(0x638E64), (DWORD)pMovieWinHeight);
}


//______________________
void UpdateMovieGlobals() {

	MOVIE_SIZE = ConfigReadInt("MOVIES", "MOVIE_SIZE", 0);
}


//___________________________
void MovieFixes(DWORD region) {

	MOVIE_SIZE = ConfigReadInt("MOVIES", "MOVIE_SIZE", 0);

	if (region == 4)
		MovieFixes_CH();
	else
		MovieFixes_MULTI(region);
}
