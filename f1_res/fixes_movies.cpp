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
DWORD FORCED_SUBTITLES_FLAG = 0;
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


//__________________________________________________________________________________________________
int MovieWinSetup(int x, int y, int width, int height, DWORD colour, DWORD winFlags, DWORD movieNum) {

	if (movieNum == 6 || movieNum == 11 || movieNum == 12)
		FORCED_SUBTITLES_FLAG = 1;
	else FORCED_SUBTITLES_FLAG = 0;


	if (*pWinRef_Movies != -1)
		return -1;
	return *pWinRef_Movies = Win_Create(x, y, width, height, colour, winFlags);
}


//______________________________________
void __declspec(naked) movie_win_setup() {

	__asm {
		push ebp
		push dword ptr ss : [esp + 0xC]
		push dword ptr ss : [esp + 0xC]
		push ecx
		push ebx
		push edx
		push eax
		call MovieWinSetup
		add esp, 0x1C
		ret 0x8
	}
}


//_______________________________________
void __declspec(naked) h_movie_kill(void) {

	__asm {
		push edx

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
		pop edx
		ret
	}
}


//________________
void ResizeMovie() {
	if (*pWinRef_Movies != -1)
		DestroyWin(*pWinRef_Movies);


	int subsHeight = 0;
	if (*pSUBTITLES_FLAG == 1 || FORCED_SUBTITLES_FLAG == 1)
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

	*pWinRef_Movies = Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0x10);

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


//_________________
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


//_______________________
void UpdateMovieGlobals() {

	MOVIE_SIZE = ConfigReadInt("MOVIES", "MOVIE_SIZE", 0);
}


//_______________
void MovieFixes() {

	MOVIE_SIZE = ConfigReadInt("MOVIES", "MOVIE_SIZE", 0);

	pSubsWinWidth = (LONG*)FixAddress(0x637404);
	pSubsWinHeight = (LONG*)FixAddress(0x637440);

	pSUBTITLES_FLAG = (DWORD*)FixAddress(0x661EFC);


	pMovieWinX = (LONG*)FixAddress(0x637420);
	pMovieWinY = (LONG*)FixAddress(0x637424);
	pMovieWinWidth = (LONG*)FixAddress(0x637434);
	pMovieWinHeight = (LONG*)FixAddress(0x637418);

	pMovieWinRect = (RECT*)FixAddress(0x637390);
	pMovieWinRect2 = (RECT*)FixAddress(0x6373A0);

	pMovieX = (LONG*)FixAddress(0x6B2A8B);
	pMovieY = (LONG*)FixAddress(0x6B2A8F);
	pMovieWidth = (LONG*)FixAddress(0x6B276C);
	pMovieHeight = (LONG*)FixAddress(0x6B2A9F);

	pSubFontRef = (LONG*)FixAddress(0x4466A7);

	//MOVIE SCALING###################

	//SETS MOVIES TO BUFFERED MODE -ALLOWS FOR VIDEO IN 16BIT
	MemWrite8(0x479A15, 0x74, 0xEB);

	//FIX SCR_HEIGHT WHEN SETTING SUBTITLE HEIGHT
	MemWrite8(0x479813, 0xBA, 0xE8);
	FuncWrite32(0x479814, 480, (DWORD)&get_scrn_height);

	FuncReplace32(0x44683D, 0x07C213, (DWORD)&h_movie_kill);

	//max movie size
	//MemWrite32(0x486E6E, 480, SCR_HEIGHT);
	//MemWrite32(0x486E73, 480, SCR_HEIGHT);
	//MemWrite32(0x486E78, 640, SCR_WIDTH);

	MemWrite16(0x4D7DC4, 0x870F, 0x9090);
	MemWrite32(0x4D7DC6, 0x033D, 0x90909090);
	MemWrite16(0x4D7DE1, 0x870F, 0x9090);
	MemWrite32(0x4D7DE3, 0x0320, 0x90909090);

	MemWrite8(0x4D8AC0, 0xB9, 0xE8);
	FuncWrite32(0x4D8AC1, 0x1B, (DWORD)&resize_movie);


	//prevent movie winRef set to -1 as it has not been destroyed
	MemWrite16(0x4793AB, 0x2D89, 0x9090);
	MemWrite32(0x4793AD, FixAddress(0x505C24), 0x90909090);


	//Play Movie Window (needed for water warning videos -- checks if movie has forced subtitles)
	FuncReplace32(0x4465A6, 0x07C27E, (DWORD)&movie_win_setup);


	///00478BF7  |.  8935 B8736300        MOV DWORD PTR DS:[6373B8],ESI            ; arg2 moveWidth -this is wrong and should be movieHeight-
	MemWrite8(0x478BF8, 0x35, 0x2D);

	MemWrite32(0x47981A, FixAddress(0x6373DC), (DWORD)pMovieWinY);

	MemWrite32(0x479842, FixAddress(0x6373DC), (DWORD)pMovieWinY);

	MemWrite32(0x479820, FixAddress(0x6373D4), (DWORD)pMovieWinHeight);

	MemWrite32(0x47983B, FixAddress(0x6373D4), (DWORD)pMovieWinHeight);
}
