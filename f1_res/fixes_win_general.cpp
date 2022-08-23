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
#include "WinFall.h"


int winRefFade[3] = { -1,-1,-1 };

double* pFadeTimeModifier = nullptr;


//_______________________________
void SetFadeTime(double fadeTime) {

	if (pFadeTimeModifier)
		*pFadeTimeModifier = fadeTime;
}


//____________________________________________
int FadeWinSetup(DWORD colour, DWORD winFlags) {

	return Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);
}


//_________________________________________
void __declspec(naked) h_fade_win_1_setup() {

	__asm {
		push esi
		push edi
		push ebp

		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		call FadeWinSetup
		add esp, 0x8
		lea esi, winRefFade
		mov dword ptr ds : [esi] , eax

		pop ebp
		pop edi
		pop esi
		ret 0x8
	}
}


//_________________________________________
void __declspec(naked) h_fade_win_2_setup() {

	__asm {
		push esi
		push edi
		push ebp

		push 0x12
		push dword ptr ss : [esp + 0x14]
		call FadeWinSetup
		add esp, 0x8

		lea esi, winRefFade
		mov dword ptr ds : [esi + 4] , eax

		pop ebp
		pop edi
		pop esi
		ret 0x8
	}
}


//_________________________________________
void __declspec(naked) h_fade_win_3_setup() {

	__asm {
		push esi
		push edi
		push ebp

		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		call FadeWinSetup
		add esp, 0x8

		lea esi, winRefFade
		mov dword ptr ds : [esi + 8] , eax

		pop ebp
		pop edi
		pop esi
		ret 0x8
	}
}


//___________________________________________
void __declspec(naked) h_fade_win_1_destroy() {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp


		lea esi, winRefFade
		push dword ptr ds : [esi]

		call DestroyWin
		add esp, 0x4

		lea esi, winRefFade
		mov dword ptr ds : [esi] , -1

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//___________________________________________
void __declspec(naked) h_fade_win_2_destroy() {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		lea esi, winRefFade
		push dword ptr ds : [esi + 4]
		call DestroyWin
		add esp, 0x4

		lea esi, winRefFade
		mov dword ptr ds : [esi + 4] , -1

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//___________________________________________
void __declspec(naked) h_fade_win_3_destroy() {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		lea esi, winRefFade
		push dword ptr ds : [esi + 8]
		call DestroyWin
		add esp, 0x4

		lea esi, winRefFade
		mov dword ptr ds : [esi + 8] , -1

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//__________________________________
void ColourFill(BYTE* buff, char ch) {

	memset(buff, ch, SCR_WIDTH * SCR_HEIGHT);
}


//____________________________________
void __declspec(naked) h_colour_fill() {

	__asm {
		push esi
		push edi
		push ebp

		push[esp + 0x10]
		push eax
		call ColourFill
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		ret 0x4
	}
}


//____________________
void ReSizeFadeScrns() {
	WinStruct* win = nullptr;

	for (int i = 0; i < 3; i++) {
		if (winRefFade[i] != -1) {
			win = GetWinStruct(winRefFade[i]);
			win->width = SCR_WIDTH;
			win->height = SCR_HEIGHT;
			win->rect.left = 0;
			win->rect.top = 0;
			win->rect.right = SCR_WIDTH - 1;
			win->rect.bottom = SCR_HEIGHT - 1;
			win->buff = FReallocateMemory(win->buff, SCR_WIDTH * SCR_HEIGHT);
			memset(win->buff, win->colour, SCR_WIDTH * SCR_HEIGHT);
		}
	}
}


//_________________
LONG GetScrHeight() {

	return SCR_HEIGHT;
}


//________________
LONG GetScrWidth() {

	return SCR_WIDTH;
}


//_______________________________________________________________________
LONG GenWinSetup(DWORD width, DWORD height, DWORD colour, DWORD winFlags) {

	LONG x = 0, y = 0;
	AdjustWinPosToGame(width, height, &x, &y);
	return Win_Create(x, y, width, height, colour, winFlags);
}


//____________________________________
void __declspec(naked) gen_win_setup() {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]
		push ecx
		push ebx
		call GenWinSetup
		add esp, 0x10
		ret 0x8
	}
}


//______________________________
void __declspec(naked) get_480() {

	__asm {
		mov eax, 480
		ret
	}
}


//______________________________
void __declspec(naked) get_640() {

	__asm {
		mov eax, 640
		ret
	}
}


//____________________________________________________
void __declspec(naked) fix_script_obj_on_screen_rect() {

	__asm {
		push eax
		push edx
		push ebx
		push ecx
		push edi
		push ebp

		mov eax, pWinRef_GameArea
		cmp dword ptr ds : [eax] , -1
		je useScrnRect
		push dword ptr ds : [eax]
		call GetWinStruct
		add esp, 0x4
		cmp eax, 0
		je useScrnRect
		lea esi, dword ptr ds : [eax + 0x8]
		jmp exitFunc
		useScrnRect :
		mov esi, SCRN_RECT

		exitFunc :
		pop ebp
		pop edi
		pop ecx
		pop ebx
		pop edx
		pop eax
		ret
	}
}



//____________________
void WinGeneralFixes() {

	//Say Window Fixes-------------------------------
	//addregion
	FuncReplace32(0x45812E, 0x04DF3A, (DWORD)&get_480);
	FuncReplace32(0x458146, 0x04DF1A, (DWORD)&get_640);
	//createwin
	FuncReplace32(0x458481, 0x04DBDF, (DWORD)&get_640);
	FuncReplace32(0x45849D, 0x04DBC3, (DWORD)&get_640);
	FuncReplace32(0x4584B0, 0x04DBB8, (DWORD)&get_480);
	FuncReplace32(0x4584CB, 0x04DB9D, (DWORD)&get_480);
	//resizewin
	FuncReplace32(0x458581, 0x04DADF, (DWORD)&get_640);
	FuncReplace32(0x45859D, 0x04DAC3, (DWORD)&get_640);
	FuncReplace32(0x4585B0, 0x04DAB8, (DWORD)&get_480);
	FuncReplace32(0x4585CB, 0x04DA9D, (DWORD)&get_480);
	//scalewin
	FuncReplace32(0x458675, 0x04D9EB, (DWORD)&get_640);
	FuncReplace32(0x458691, 0x04D9CF, (DWORD)&get_640);
	FuncReplace32(0x4586A4, 0x04D9C4, (DWORD)&get_480);
	FuncReplace32(0x4586BF, 0x04D9A9, (DWORD)&get_480);
	//addbutton
	FuncReplace32(0x458EA0, 0x04D1C8, (DWORD)&get_480);
	FuncReplace32(0x458EB7, 0x04D1A9, (DWORD)&get_640);
	FuncReplace32(0x458ED3, 0x04D195, (DWORD)&get_480);
	FuncReplace32(0x458EEF, 0x04D171, (DWORD)&get_640);
	//--------------------------------------------

	//fix for obj_on_screen script function
 	MemWrite8(0x452769, 0xBE, 0xE8);
	FuncWrite32(0x45276A, FixAddress(0x44B9F8), (DWORD)&fix_script_obj_on_screen_rect);

	//Target Panel---POSITION-----------------------------------------------
 	FuncReplace32(0x423AC7, 0x09ED5D, (DWORD)&gen_win_setup);

	//CENTRE ELEVATOR-------------------------------------------------------
 	FuncReplace32(0x4386F4, 0x08A130, (DWORD)&gen_win_setup);

	//IN GAME MENU---POSITION-----------------------------------------------
 	FuncReplace32(0x481B5A, 0x040CCA, (DWORD)&gen_win_setup);

	// MESSAGE_BOX
	FuncReplace32(0x41C1D6, 0x0A664E, (DWORD)&gen_win_setup);

	//LOCAL_MAP
	FuncReplace32(0x41AA50, 0x0A7DD4, (DWORD)&gen_win_setup);

	//options window
	FuncReplace32(0x4824DE, 0x040346, (DWORD)&gen_win_setup);

	//bio scrn panel
 	FuncReplace32(0x4959C7, 0x02CE5D, (DWORD)&gen_win_setup);

	//char scrn panel
 	FuncReplace32(0x42D047, 0x0957DD, (DWORD)&gen_win_setup);

	//Pipboy
	FuncReplace32(0x48729D, 0x03B587, (DWORD)&gen_win_setup);

	//QUICK LOAD BACKGROUND WIN
	FuncReplace32(0x46EC63, 0x053BC1, (DWORD)&h_fade_win_1_setup);

	FuncReplace32(0x46ED0C, 0x053D44, (DWORD)&h_fade_win_1_destroy);
	FuncReplace32(0x46EDEB, 0x053C65, (DWORD)&h_fade_win_1_destroy);

	//LOAD_SCRN - fade OUT BLACK
	FuncReplace32(0x472BA6, 0x04FC7E, (DWORD)&h_fade_win_2_setup);

	FuncReplace32(0x472C31, 0x04FE1F, (DWORD)&h_fade_win_2_destroy);
	FuncReplace32(0x472C4C, 0x04FE04, (DWORD)&h_fade_win_2_destroy);

	//FADE TO MAP AT START
	FuncReplace32(0x472DDE, 0x04FA46, (DWORD)&h_fade_win_3_setup);

	FuncReplace32(0x472E27, 0x04FC29, (DWORD)&h_fade_win_3_destroy);

	//QUICK LOAD BACKGROUND FILL BLACK
	FuncReplace32(0x46EC8C, 0x04F9AC, (DWORD)&h_colour_fill);

	pFadeTimeModifier = (double*)FixAddress(0x4FB49C);

	if (*pFadeTimeModifier != 60.0f)
		*pFadeTimeModifier = 0;
	else
		SetFadeTime((double)ConfigReadInt("OTHER_SETTINGS", "FADE_TIME_MODIFIER", 60));
}
