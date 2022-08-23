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
#include "F_Mapper.h"

int HELP_SCRN_SIZE = 0;
LONG helpWidth = 0;
LONG helpHeight = 0;
int winRefHelpScrn = -1;


//______________________________________________________
LONG _stdcall HelpWinSetup(DWORD colour, DWORD winFlags) {

	HELP_SCRN_SIZE = ConfigReadInt("STATIC_SCREENS", "HELP_SCRN_SIZE", 0);
	FLoadPalette("art\\intrface\\helpscrn.pal");
	winRefHelpScrn = Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, FindDarkPalRef(pMAIN_PAL), winFlags);
	return winRefHelpScrn;
}


//______________________________________________________________________________________________________________
void HelpBackDraw(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	int colour = FindDarkPalRef(pMAIN_PAL);
	Fix8BitColours(fromBuff, subWidth, subHeight, fromWidth, colour);

	if (HELP_SCRN_SIZE == 1 || helpWidth > SCR_WIDTH || helpHeight > SCR_HEIGHT)
		MemBlt8Stretch(fromBuff, helpWidth, helpHeight, helpWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, true, true);
	else if (HELP_SCRN_SIZE == 2)
		MemBlt8Stretch(fromBuff, helpWidth, helpHeight, helpWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, false, false);
	else {
		toBuff += ((SCR_WIDTH >> 1) - (helpWidth >> 1)) + (((SCR_HEIGHT >> 1) - (helpHeight >> 1)) * SCR_WIDTH);
		MemBlt8(fromBuff, helpWidth, helpHeight, helpWidth, toBuff, SCR_WIDTH);
	}
}


//____________________________________________
BYTE* GetHelpFrmVars(DWORD fid, DWORD* frmObj) {

	FRMhead* frm = F_GetFrm(fid, frmObj);

	helpWidth = F_GetFrmFrameWidth(frm, 0, 0);
	helpHeight = F_GetFrmFrameHeight(frm, 0, 0);

	return F_GetFrmFrameBuff(frm, 0, 0);
}


//____________________________________________
void __declspec(naked) get_help_frm_vars(void) {

	__asm {
		//push ebx
		//push ecx
		//push edi

		push ecx
		push eax
		call GetHelpFrmVars
		add esp, 0x8

		//pop edi
		//pop ecx
		//pop ebx
		ret
	}
}


//__________________________________________
void __declspec(naked) h_help_scrn_destroy() {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi

		push winRefHelpScrn
		call DestroyWin
		add esp, 0x4
		mov winRefHelpScrn, -1
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}

}


//___________________
void ReSizeHelpScrn() {

	if (winRefHelpScrn == -1)
		return;
	WinStruct* win = GetWinStruct(winRefHelpScrn);
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
	DWORD fid = F_GetFrmID(ART_INTRFACE, 297, 0, 0, 0);
	DWORD frmObj;
	FRMhead* frm = F_GetFrm(fid, &frmObj);
	BYTE* buff = F_GetFrmFrameBuff(frm, 0, 0);
	int width = F_GetFrmFrameWidth(frm, 0, 0);
	int height = F_GetFrmFrameHeight(frm, 0, 0);
	HelpBackDraw(buff, width, height, width, win->buff, SCR_WIDTH);
	F_UnloadFrm(frmObj);
}


//________________________________________
void __declspec(naked) h_fix_map_palette() {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi

		push eax
		call FSetPalette
		add esp, 0x4
		///
		call ReDrawViewWin
		///
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}



//__________________
void HelpScrnFixes() {

	FuncReplace32(0x43D51F, 0x085305, (DWORD)&HelpWinSetup);

	FuncReplace32(0x43D58A, 0x080F06, (DWORD)&HelpBackDraw);

	FuncReplace32(0x43D55B, 0xFFFDB459, (DWORD)&get_help_frm_vars);

	FuncReplace32(0x43D5E4, 0x08546C, (DWORD)&h_help_scrn_destroy);

	FuncReplace32(0x43D5F8, 0x04807C, (DWORD)&h_fix_map_palette);
}
