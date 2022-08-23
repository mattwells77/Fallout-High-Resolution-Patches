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


LONG winRefFade[3] = { -1,-1,-1 };
double* pFadeTimeModifier = nullptr;



//_______________________________
void SetFadeTime(double fadeTime) {

	if (pFadeTimeModifier)
		*pFadeTimeModifier = fadeTime;
}


//____________________________________________
LONG FadeWinSetup(DWORD colour, DWORD winFlags) {

	return Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);
}


//_______________________________________
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
long GetScrHeight() {

	return SCR_HEIGHT;
}


//________________
long GetScrWidth() {

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
		push esi
		push edi
		push ebp

		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push ecx
		push ebx
		call GenWinSetup
		add esp, 0x10

		pop ebp
		pop edi
		pop esi
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


//___________________________
void WinGeneralFixes_CH(void) {

	//Say Window Fixes
	//addregion
	FuncReplace32(0x462228, 0x055AE8, (DWORD)&get_480);
	FuncReplace32(0x462243, 0x055AC5, (DWORD)&get_640);
	//createwin
	FuncReplace32(0x4626F7, 0x055611, (DWORD)&get_640);
	FuncReplace32(0x46270F, 0x0555F9, (DWORD)&get_640);
	FuncReplace32(0x462722, 0x0555EE, (DWORD)&get_480);
	FuncReplace32(0x46273E, 0x0555D2, (DWORD)&get_480);
	//resizewin
	FuncReplace32(0x46287B, 0x05548D, (DWORD)&get_640);
	FuncReplace32(0x462893, 0x055475, (DWORD)&get_640);
	FuncReplace32(0x4628A6, 0x05546A, (DWORD)&get_480);
	FuncReplace32(0x4628C2, 0x05544E, (DWORD)&get_480);
	//scalewin
	FuncReplace32(0x4629F3, 0x055315, (DWORD)&get_640);
	FuncReplace32(0x462A0B, 0x0552FD, (DWORD)&get_640);
	FuncReplace32(0x462A1E, 0x0552F2, (DWORD)&get_480);
	FuncReplace32(0x462A3A, 0x0552D6, (DWORD)&get_480);
	//addbutton
	FuncReplace32(0x46346A, 0x0548A6, (DWORD)&get_480);
	FuncReplace32(0x463481, 0x054887, (DWORD)&get_640);
	FuncReplace32(0x463499, 0x054877, (DWORD)&get_480);
	FuncReplace32(0x4634B5, 0x054853, (DWORD)&get_640);


	//fix for obj_on_screen script function
	MemWrite8(0x45BF85, 0xBE, 0xE8);
	FuncWrite32(0x45BF86, FixAddress(0x453FC0), (DWORD)&fix_script_obj_on_screen_rect);


	//Target Panel
	FuncWrite32(0x425EB0, 0x0B6209, (DWORD)&gen_win_setup);

	//CENTRE ELEVATOR
	FuncWrite32(0x43EA91, 0x09D628, (DWORD)&gen_win_setup);


	//IN GAME MENU
	FuncWrite32(0x48F406, 0x04CCB3, (DWORD)&gen_win_setup);

	// MESSAGE_BOX
	FuncWrite32(0x41D095, 0x0BF024, (DWORD)&gen_win_setup);

	//LOCAL_MAP
	FuncWrite32(0x41B90A, 0x0C07AF, (DWORD)&gen_win_setup);

	//options window
	FuncWrite32(0x48FD06, 0x04C3B3, (DWORD)&gen_win_setup);

	//bio scrn window
	FuncWrite32(0x4A6197, 0x035F22, (DWORD)&gen_win_setup);

	//char scrn window
	FuncWrite32(0x432A6C, 0x0A964D, (DWORD)&gen_win_setup);

	//Pipboy
	FuncWrite32(0x4961A5, 0x045F14, (DWORD)&gen_win_setup);


	//QUICK LOAD BACKGROUND WIN
	FuncWrite32(0x47BCF3, 0x0603C6, (DWORD)&h_fade_win_1_setup);
	FuncWrite32(0x47BD9F, 0x06061D, (DWORD)&h_fade_win_1_destroy);
	FuncWrite32(0x47BE7B, 0x060541, (DWORD)&h_fade_win_1_destroy);

	//LOAD_SCRN - fade OUT BLACK
	FuncWrite32(0x47FF8E, 0x05C12B, (DWORD)&h_fade_win_2_setup);
	FuncWrite32(0x48001C, 0x05C3A0, (DWORD)&h_fade_win_2_destroy);
	FuncWrite32(0x480037, 0x05C385, (DWORD)&h_fade_win_2_destroy);

	//FADE TO MAP AT START
	FuncWrite32(0x480212, 0x05BEA7, (DWORD)&h_fade_win_3_setup);
	FuncWrite32(0x48025B, 0x05C161, (DWORD)&h_fade_win_3_destroy);

	//QUICK LOAD BACKGROUND FILL BLACK
	FuncWrite32(0x47BD1C, 0x05D412, (DWORD)&h_colour_fill);


	pFadeTimeModifier = (double*)FixAddress(0x51C240);

	if (*pFadeTimeModifier != 60.0f)
		*pFadeTimeModifier = 0;
	else
		SetFadeTime((double)ConfigReadInt("OTHER_SETTINGS", "FADE_TIME_MODIFIER", 60));
}


//______________________________
void WinGeneralFixes_MULTI(void) {

	//Say Window Fixes
	//addregion
	FuncReplace32(0x462B28, 0x056524, (DWORD)&get_480);
	FuncReplace32(0x462B43, 0x056501, (DWORD)&get_640);
	//createwin
	FuncReplace32(0x462FF7, 0x05604D, (DWORD)&get_640);
	FuncReplace32(0x46300F, 0x056035, (DWORD)&get_640);
	FuncReplace32(0x463022, 0x05602A, (DWORD)&get_480);
	FuncReplace32(0x46303E, 0x05600E, (DWORD)&get_480);
	//resizewin
	FuncReplace32(0x46317B, 0x055EC9, (DWORD)&get_640);
	FuncReplace32(0x463193, 0x055EB1, (DWORD)&get_640);
	FuncReplace32(0x4631A6, 0x055EA6, (DWORD)&get_480);
	FuncReplace32(0x4631C2, 0x055E8A, (DWORD)&get_480);
	//scalewin
	FuncReplace32(0x4632F3, 0x055D51, (DWORD)&get_640);
	FuncReplace32(0x46330B, 0x055D39, (DWORD)&get_640);
	FuncReplace32(0x46331E, 0x055D2E, (DWORD)&get_480);
	FuncReplace32(0x46333A, 0x055D12, (DWORD)&get_480);
	//addbutton
	FuncReplace32(0x463D6A, 0x0552E2, (DWORD)&get_480);
	FuncReplace32(0x463D81, 0x0552C3, (DWORD)&get_640);
	FuncReplace32(0x463D99, 0x0552B3, (DWORD)&get_480);
	FuncReplace32(0x463DB5, 0x05528F, (DWORD)&get_640);

	//fix for obj_on_screen script function
	MemWrite8(0x45C885, 0xBE, 0xE8);
	FuncWrite32(0x45C886, FixAddress(0x453FC0), (DWORD)&fix_script_obj_on_screen_rect);

	//Target Panel
	FuncReplace32(0x42626C, 0x0AFFC8, (DWORD)&gen_win_setup);

	//CENTRE ELEVATOR
	FuncReplace32(0x43F561, 0x096CD3, (DWORD)&gen_win_setup);

	//IN GAME MENU
	FuncReplace32(0x490006, 0x04622E, (DWORD)&gen_win_setup);

	// MESSAGE_BOX
	FuncReplace32(0x41D105, 0x0B912F, (DWORD)&gen_win_setup);

	//LOCAL_MAP
	FuncReplace32(0x41B97A, 0x0BA8BA, (DWORD)&gen_win_setup);

	//options window
	FuncReplace32(0x490962, 0x0458D2, (DWORD)&gen_win_setup);

	//bio scrn panel
	FuncReplace32(0x4A7497, 0x02ED9D, (DWORD)&gen_win_setup);

	//char scrn panel
	FuncReplace32(0x432DE9, 0x0A344B, (DWORD)&gen_win_setup);
	//Pipboy
	FuncReplace32(0x497406, 0x03EE2E, (DWORD)&gen_win_setup);

	//QUICK LOAD BACKGROUND WIN
	FuncReplace32(0x47C6F3, 0x059B41, (DWORD)&h_fade_win_1_setup);
	FuncReplace32(0x47C79F, 0x059CC5, (DWORD)&h_fade_win_1_destroy);
	FuncReplace32(0x47C87B, 0x059BE9, (DWORD)&h_fade_win_1_destroy);

	//LOAD_SCRN - fade OUT BLACK
	FuncReplace32(0x480B0E, 0x055726, (DWORD)&h_fade_win_2_setup);
	FuncReplace32(0x480B9C, 0x0558C8, (DWORD)&h_fade_win_2_destroy);
	FuncReplace32(0x480BB7, 0x0558AD, (DWORD)&h_fade_win_2_destroy);

	//FADE TO MAP AT START
	FuncReplace32(0x480D8E, 0x0554A6, (DWORD)&h_fade_win_3_setup);
	FuncReplace32(0x480DD7, 0x05568D, (DWORD)&h_fade_win_3_destroy);

	//QUICK LOAD BACKGROUND FILL BLACK
	FuncReplace32(0x47C71C, 0x05715C, (DWORD)&h_colour_fill);

	pFadeTimeModifier = (double*)FixAddress(0x50C3A8);
	if (*pFadeTimeModifier != 60.0f)
		*pFadeTimeModifier = 0;
	else
		SetFadeTime((double)ConfigReadInt("OTHER_SETTINGS", "FADE_TIME_MODIFIER", 60));
}


//___________________________
void cd_checker(int CD_CHECK) {

	if (CD_CHECK == 0) {
		if (*(DWORD*)0x4426A4 == 1)
			MemWrite8(0x4426A4, 0x1, 0x0);
	}
}


//______________________________
void WinGeneralFixes(int region) {

	switch (region) {
	case 1://US
	case 5://RULC
		WinGeneralFixes_MULTI();
		break;
	case 2://UK
	case 3://FR GR
		WinGeneralFixes_MULTI();
		cd_checker(ConfigReadInt("OTHER_SETTINGS", "CD_CHECK", 0));
		break;
	case 4://CHI
		WinGeneralFixes_CH();
		break;
	default:
		break;
	}
}
