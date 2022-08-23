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
#include "F_Text.h"
#include "configTools.h"
#include "F_Windows.h"
#include "WinFall.h"


LONG winRef_Death = -1;

LONG DEATH_SCRN_SIZE = 0;

LONG deathWidth = 0;
LONG deathHeight = 0;

BYTE* deathTextBoxBuff = nullptr;

LONG deathTextBoxWidth = 0;
LONG deathTextBoxHeight = 0;


//______________________________________________________________________________________________
void DeathCreateTextBox(BYTE* buff, LONG subWidth, LONG subHeight, LONG width, DWORD colorFlags) {

	deathTextBoxBuff = buff;
	deathTextBoxWidth = subWidth;
	deathTextBoxHeight = subHeight;

	if (deathTextBoxBuff)
		delete[] deathTextBoxBuff;

	deathTextBoxBuff = new BYTE[subWidth * subHeight];
	memset(deathTextBoxBuff, (BYTE)colorFlags, subWidth * subHeight);


	WinStruct* win = GetWinStruct(winRef_Death);
	int yPos = SCR_HEIGHT - 8 - deathTextBoxHeight;
	int xPos = SCR_WIDTH / 2 - deathTextBoxWidth / 2;
	buff = win->buff + yPos * SCR_WIDTH + xPos;

	while (subHeight) {
		memset(buff, (BYTE)colorFlags, subWidth);
		buff += SCR_WIDTH;
		subHeight--;
	}
}


//________________________________________________
void __declspec(naked) death_create_text_box(void) {

	__asm {
		push dword ptr ss : [esp + 0x4]
		push ecx
		push ebx
		push edx
		push eax
		call DeathCreateTextBox
		add esp, 0x14
		ret 0x4
	}
}


//_______________________________________________________________________________________________________
void DeathPrintText(LONG lineNum, BYTE* toBuff, char* text, LONG txtWidth, LONG toWidth, BYTE colorFlags) {

	WinStruct* win = GetWinStruct(winRef_Death);

	int lineOffset = win->width * GetTextHeight() * lineNum;
	int yPos = SCR_HEIGHT - 8 - deathTextBoxHeight;
	int xPos = win->width / 2 - deathTextBoxWidth / 2;
	toBuff = win->buff + yPos * win->width + xPos;
	PrintText(toBuff + lineOffset, text, txtWidth, win->width, colorFlags);

	lineOffset = deathTextBoxWidth * GetTextHeight() * lineNum;
	PrintText(deathTextBoxBuff + lineOffset, text, txtWidth, deathTextBoxWidth, colorFlags);
}


//___________________________________________
void __declspec(naked) death_print_text(void) {

	__asm {
		push dword ptr ss : [esp + 0x4]
		push ecx
		push ebx
		push edx
		push eax
		push edi//linenum
		call DeathPrintText
		add esp, 0x18
		ret 0x4
	}
}


//______________________________________________
void __declspec(naked) death_print_text_ch(void) {

	__asm {
		push dword ptr ss : [esp + 0x4]
		push ecx
		push ebx
		push edx
		push eax
		push ebp//linenum
		call DeathPrintText
		add esp, 0x18
		ret 0x4
	}
}


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

	DEATH_SCRN_SIZE = ConfigReadInt("STATIC_SCREENS", "DEATH_SCRN_SIZE", 0);
	return winRef_Death = Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);
}


//_________________________________
void Death_DestroyWin(LONG winRef) {

	DestroyWin(winRef_Death);
	winRef_Death = -1;
	if (deathTextBoxBuff)
		delete[] deathTextBoxBuff;
	deathTextBoxBuff = nullptr;
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
		call Death_DestroyWin
		add esp, 0x4

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

	if (!deathTextBoxBuff)
		return;
	int yPos = SCR_HEIGHT - 8 - deathTextBoxHeight;
	int xPos = SCR_WIDTH / 2 - deathTextBoxWidth / 2;
	buff = win->buff + yPos * SCR_WIDTH + xPos;
	MemBlt8(deathTextBoxBuff, deathTextBoxWidth, deathTextBoxHeight, deathTextBoxWidth, buff, SCR_WIDTH);
}


//__________________________
void DeathScrnFixes_CH(void) {

	FuncWrite32(0x4807EF, 0x05893F, (DWORD)&death_create_text_box);

	MemWrite16(0x480832, 0x15FF, 0xE890);
	FuncWrite32(0x480834, 0x52E194, (DWORD)&death_print_text_ch);

	//DEATH DRAW FIX
	FuncWrite32(0x4806EE, 0x0587CA, (DWORD)&DrawDeathBack);

	FuncWrite32(0x4806AA, 0xFFF98A72, (DWORD)&get_death_frm_vars);

	FuncWrite32(0x480666, 0x05BA53, (DWORD)&CreateWin_Death);

	FuncWrite32(0x48093D, 0x05BA7F, (DWORD)&destroy_win_death);
}


//_________________________
void DeathScrnFixes_MULTI() {

	FuncReplace32(0x481354, 0x052524, (DWORD)&death_create_text_box);

	MemWrite16(0x481380, 0x15FF, 0xE890);
	FuncWrite32(0x481382, FixAddress(0x51E3B8), (DWORD)&death_print_text);

	//DEATH DRAW FIX
	FuncReplace32(0x48126A, 0x052466, (DWORD)&DrawDeathBack);

	FuncReplace32(0x481226, 0xFFF97F5E, (DWORD)&get_death_frm_vars);

	FuncReplace32(0x4811E2, 0x055052, (DWORD)&CreateWin_Death);

	FuncReplace32(0x481477, 0x054FED, (DWORD)&destroy_win_death);
}


//_______________________________
void DeathScrnFixes(DWORD region) {

	if (region == 4)
		DeathScrnFixes_CH();
	else
		DeathScrnFixes_MULTI();
}
