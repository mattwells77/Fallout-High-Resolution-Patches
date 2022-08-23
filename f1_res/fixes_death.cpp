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
#include "configTools.h"
#include "F_Windows.h"
#include "WinFall.h"

int winRef_Death = -1;

int DEATH_SCRN_SIZE = 0;


LONG deathWidth = 0;
LONG deathHeight = 0;


//_______________________________________________________________________________________________________________
void DrawDeathBack(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	int colour = FindDarkPalRef(pMAIN_PAL);
	Fix8BitColours(fromBuff, subWidth, subHeight, fromWidth, colour);

	if (DEATH_SCRN_SIZE == 1 || deathWidth > SCR_WIDTH || deathHeight > SCR_HEIGHT)
		MemBlt8Stretch(fromBuff, deathWidth, deathHeight, deathWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, true, true);
	else if (DEATH_SCRN_SIZE == 2)
		MemBlt8Stretch(fromBuff, deathWidth, deathHeight, deathWidth, toBuff, SCR_WIDTH, SCR_HEIGHT, false, false);
	else {
		toBuff += ((SCR_WIDTH >> 1) - (deathWidth >> 1)) + (((SCR_HEIGHT >> 1) - (deathHeight >> 1)) * SCR_WIDTH);
		MemBlt8(fromBuff, deathWidth, deathHeight, deathWidth, toBuff, SCR_WIDTH);
	}
}


//_________________________________________
void __declspec(naked) h_death_subs_x(void) {

	//add SCR_WIDTH/2-640/2 to panel mem offset(edi). to move sub text to center screen
	//multiply SCR_WIDTH x text pos height(edx). to adjust mem offset for screen height
	__asm
	{
		mov eax, SCR_WIDTH
		shr eax, 1
		sub eax, 320
		add edi, eax
		mov eax, SCR_WIDTH
		IMUL EAX, EDX
		ret
	}
}


//_____________________________________________
BYTE* GetDeathFrmVars(DWORD fid, DWORD* frmObj) {

	FRMhead* frm = F_GetFrm(fid, frmObj);

	deathWidth = F_GetFrmFrameWidth(frm, 0, 0);
	deathHeight = F_GetFrmFrameHeight(frm, 0, 0);

	return F_GetFrmFrameBuff(frm, 0, 0);
}


//_____________________________________________
void __declspec(naked) get_death_frm_vars(void) {

	__asm {
		//push ebx
		//push ecx
		//push edi

		push ecx
		push eax
		call GetDeathFrmVars
		add esp, 0x8

		//pop edi
		//pop ecx
		//pop ebx
		ret
	}
}


//________________________________________________________
int _stdcall CreateWin_Death(DWORD colour, DWORD winFlags) {

	return winRef_Death = Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);
}


//_____________________________________________
void __declspec(naked) destroy_win_death(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi


		push eax
		call DestroyWin
		add esp, 0x4
		mov winRef_Death, -1

		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//________________
void ReSizeDeath() {

	if (winRef_Death == -1)
		return;
	WinStruct* win = GetWinStruct(winRef_Death);
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
	DWORD fid = F_GetFrmID(ART_INTRFACE, 309, 0, 0, 0);
	DWORD frmObj;
	FRMhead* frm = F_GetFrm(fid, &frmObj);
	BYTE* buff = F_GetFrmFrameBuff(frm, 0, 0);
	int width = F_GetFrmFrameWidth(frm, 0, 0);
	int height = F_GetFrmFrameHeight(frm, 0, 0);
	DrawDeathBack(buff, width, height, width, win->buff, SCR_WIDTH);
	F_UnloadFrm(frmObj);
}


//___________________
void DeathScrnFixes() {

	DEATH_SCRN_SIZE = ConfigReadInt("STATIC_SCREENS", "DEATH_SCRN_SIZE", 0);

	FuncReplace32(0x47329F, 0x04B1F1, (DWORD)&DrawDeathBack);

	FuncReplace32(0x473260, 0xFFFA5754, (DWORD)&get_death_frm_vars);

	FuncReplace32(0x473224, 0x04F600, (DWORD)&CreateWin_Death);

	FuncReplace32(0x473384, 0x04F6CC, (DWORD)&destroy_win_death);
}
