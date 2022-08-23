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
#include "F_Art.h"
#include "F_Text.h"
#include "F_Msg.h"
#include "configTools.h"
#include "WinFall.h"


//for hires win setup
LONG OPTIONS_BUTTON = -1;

LONG MAIN_MENU_SIZE = 0;
LONG USE_HIRES_IMAGES = 0;
LONG SCALE_BUTTONS_AND_TEXT_MENU = 0;

LONG main_menu_region = 0;

MSGList* pMenuMsgList = nullptr;
BYTE** lpMainMenuColour = nullptr;
BYTE** lpMainCopyrightColour = nullptr;
BYTE** lpMainVersionColour = nullptr;

LONG* isMainmenu = 0;
LONG* isMainmenuHidden = 0;

F_LONG_FUNC_VOID F_MainMenuSetup = nullptr;
F_LONG_FUNC_VOID F_MainMenuDestroy = nullptr;

void* F_SHOW_MAIN_MENU = nullptr;
void* F_HIDE_MAIN_MENU = nullptr;
void* F_GET_VERSION_TEXT = nullptr;

void* P_WIN_PRINT_VERSION_TEXT = nullptr;

UNLSTDfrm* pFrm_MainMenuButts = nullptr;
LONG* pMainMenuButtRefs = nullptr;
LONG* pMainMenuButtKeys = nullptr;


//______________________
bool IsMainMenuVisible() {

	if (*isMainmenu && !*isMainmenuHidden)
		return true;
	return false;
}

//_________________________________
void F_HideMainMenu(DWORD fadeFlag) {

	__asm {
		mov eax, fadeFlag
		call F_HIDE_MAIN_MENU
	}
}


//__________________________________
void F_ShowMainMenu(DWORD fadeFlag) {

	__asm {
		mov eax, fadeFlag
		call F_SHOW_MAIN_MENU
	}
}


//____________________________________
char* F_GetVersionText(char* outText) {

	__asm {
		mov eax, outText
		call F_GET_VERSION_TEXT
	}
	return outText;
}


//_______________________________________
void _stdcall DeleteButtons(DWORD FrmObj) {

	if (pFrm_MainMenuButts)
		delete pFrm_MainMenuButts;
	pFrm_MainMenuButts = nullptr;
}


//_______________________________________
void __declspec(naked) h_delete_buttons() {

	__asm {
		push eax//FrmObj
		call DeleteButtons
		xor edi, edi
		ret
	}
}


//___________________
void ReSizeMainMenu() {

	if (!*isMainmenu)
		return;

	LONG wasHidden = *isMainmenuHidden;

	F_MainMenuDestroy();
	*pWinRef_MainMenu = -1;
	if (F_MainMenuSetup() == -1)
		MessageBox(nullptr, "F_MainMenuSetup failed", "Hi-Res patch Error", MB_ICONERROR);

	if (!wasHidden) {
		ShowWin(*pWinRef_MainMenu);
		*isMainmenuHidden = 0;
	}
}


//_______________________________________________
void __declspec(naked) fix_destroy_mainmenu_ref() {

	__asm {
		push eax
		call DestroyWin
		add esp, 0x4
		mov esi, pWinRef_MainMenu
		mov dword ptr ds : [esi] , -1
		ret
	}
}


//________________________________________________________________________________________________________
void PrintMainMenuText(BYTE* buff, LONG xPos, LONG yPos, LONG buffWidth, LONG buffHeight, DWORD palColour) {

	if (main_menu_region == 4)
		return;
	LONG oldFont = GetFont();
	SetFont(104);

	LONG lineY = 0;

	LONG buffPos = xPos + yPos * buffWidth;

	LONG lineYOff = 0, lineXOff = 0;
	LONG txtWidth = 0;
	LONG txtHeight = GetTextHeight();
	MSGNode* msgNode = nullptr;
	for (DWORD i = 0; i < 6; i++) {
		msgNode = GetMsgNode(pMenuMsgList, 9 + i);
		if (msgNode) {
			txtWidth = GetTextWidth(msgNode->msg2);
			if (txtWidth > buffWidth)
				txtWidth = buffWidth;
			lineXOff = txtWidth >> 1;

			if (xPos - lineXOff < 0)
				lineXOff = xPos;
			else if (xPos - lineXOff + txtWidth > buffWidth)
				txtWidth = buffWidth - (xPos - lineXOff);

			if (yPos - lineY >= 0 && yPos - lineY + txtHeight <= buffHeight)
				PrintText(buff + buffPos - lineYOff - lineXOff, msgNode->msg2, txtWidth, buffWidth, palColour & 0xFF);
		}
		buffPos += (42 * buffWidth);
		lineYOff += buffWidth;
		yPos += 42;
		lineY += 1;
	}
	SetFont(oldFont);
}


//_____________________________________________
void __declspec(naked) win_print_version_text() {

	__asm {
		pop edi//ret address
		mov dword ptr ss : [esp + 0x8] , edi// put return offset from call into space on stack
		jmp P_WIN_PRINT_VERSION_TEXT
	}
}


//______________________________________________________________________________________________________________
void WinPrintVersionText(LONG WinRef, const char* txtBuff, DWORD txtWidth, LONG x, LONG y, DWORD palColourFlags) {
	__asm {
		pushad
		push 0xFFFFFFFF//space on stack for return offset
		push palColourFlags
		push y
		mov ecx, x
		mov ebx, txtWidth
		mov edx, txtBuff
		mov eax, WinRef
		call win_print_version_text
		popad
	}
}


//_________________
int MainMenuSetup() {

	if (*isMainmenu)
		return 0;
	FLoadPalette("color.pal");
	*pWinRef_MainMenu = Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0xC);
	if (*pWinRef_MainMenu == -1) {
		F_MainMenuDestroy();
		return -1;
	}
	WinStruct* win = GetWinStruct(*pWinRef_MainMenu);
	BYTE* winBuff = win->buff;

	DWORD palColour = 0;

	UNLSTDfrm* pMainMenuFrm = nullptr;
	UNLSTDfrm* menuBG = nullptr;

	MAIN_MENU_SIZE = ConfigReadInt("MAINMENU", "MAIN_MENU_SIZE", 0);
	USE_HIRES_IMAGES = ConfigReadInt("MAINMENU", "USE_HIRES_IMAGES", 0);
	SCALE_BUTTONS_AND_TEXT_MENU = ConfigReadInt("MAINMENU", "SCALE_BUTTONS_AND_TEXT_MENU", 0);

	LONG menuBGOffX = ConfigReadInt("MAINMENU", "MENU_BG_OFFSET_X", -14);
	LONG menuBGOffY = ConfigReadInt("MAINMENU", "MENU_BG_OFFSET_Y", -4);

	LONG buttX = 30;
	LONG buttY = 19;
	buttX += SfallReadInt("Misc", "MainMenuOffsetX", 0);
	buttY += SfallReadInt("Misc", "MainMenuOffsetY", 0);

	LONG textX = buttX + 96;
	LONG textY = buttY + 1;

	if (lpMainMenuColour)
		palColour = (**lpMainMenuColour) | 0x06000000;

	if (USE_HIRES_IMAGES) {
		pMainMenuFrm = LoadUnlistedFrm("HR_MAINMENU.frm", ART_INTRFACE);
		if (main_menu_region == 4)
			menuBG = LoadUnlistedFrm("HR_MENU_BG_CHI.frm", ART_INTRFACE);
		else
			menuBG = LoadUnlistedFrm("HR_MENU_BG.frm", ART_INTRFACE);
		if (menuBG)
			PrintMainMenuText(menuBG->frames[0].buff, 96 - menuBGOffX, 1 - menuBGOffY, menuBG->frames[0].width, menuBG->frames[0].height, palColour);
	}
	if (!pMainMenuFrm || !USE_HIRES_IMAGES) {
		pMainMenuFrm = LoadUnlistedFrm("MAINMENU.frm", ART_INTRFACE);
		PrintMainMenuText(pMainMenuFrm->frames[0].buff, textX, textY, pMainMenuFrm->frames[0].width, pMainMenuFrm->frames[0].height, palColour);
	}
	if (!pMainMenuFrm) {
		F_MainMenuDestroy();
		return -1;
	}

	float xRO = 1;
	float yRO = 1;
	LONG bgX = 0, bgY = 0, bgWidth = pMainMenuFrm->frames[0].width, bgHeight = pMainMenuFrm->frames[0].height;

	if (MAIN_MENU_SIZE == 0 && (SCR_WIDTH < bgWidth || SCR_HEIGHT < bgHeight))
		MAIN_MENU_SIZE = 1;

	if (MAIN_MENU_SIZE == 0) {
		xRO = 1;
		yRO = 1;
		bgX = (SCR_WIDTH - bgWidth) / 2;
		bgY = (SCR_HEIGHT - bgHeight) / 2;
		winBuff += bgY * SCR_WIDTH + bgX;
		MemBlt8(pMainMenuFrm->frames[0].buff, bgWidth, bgHeight, bgWidth, winBuff, SCR_WIDTH);
		if (MAIN_MENU_SIZE == 0) {
			yRO = (float)bgHeight / 480.0f;
			xRO = (float)bgWidth / 640.0f;
			if (xRO >= yRO)
				xRO = yRO;
			else
				yRO = xRO;
		}
	}
	else {
		yRO = (float)SCR_HEIGHT / 480.0f;
		xRO = (float)SCR_WIDTH / 640.0f;
		bool isSameAspect = true;

		if (MAIN_MENU_SIZE == 1) {
			if (xRO >= yRO) {
				bgX = (LONG)((640 * xRO / 2) - (640 * yRO / 2));
				xRO = yRO;
				bgY = 0;
			}
			else {
				bgY = (LONG)((480 * yRO / 2) - (480 * xRO / 2));
				yRO = xRO;
				bgX = 0;
			}
		}
		else if (MAIN_MENU_SIZE == 2) {
			isSameAspect = false;
			bgX = 0;
			bgY = 0;
		}
		if (MAIN_MENU_SIZE != 0)
			MemBlt8Stretch(pMainMenuFrm->frames[0].buff, bgWidth, bgHeight, bgWidth, winBuff, SCR_WIDTH, SCR_HEIGHT, isSameAspect, true);
	}
	delete pMainMenuFrm;

	LONG oldFont = GetFont();
	SetFont(100);

	MSGNode* msgNode = GetMsgNode(pMenuMsgList, 20);
	//480-460;
	if (msgNode) {
		if (lpMainCopyrightColour)
			palColour = (**lpMainCopyrightColour) | 0x06000000;
		WinPrintText(*pWinRef_MainMenu, msgNode->msg2, 0, bgX + 15, SCR_HEIGHT - bgY - 20, palColour);
	}
	LONG verX = 615;
	LONG verY = 460;
	verX += SfallReadInt("Misc", "MainMenuCreditsOffsetX", 0);
	verY += SfallReadInt("Misc", "MainMenuCreditsOffsetY", 0);
	verX = 640 - verX;
	verY = 480 - verY;
	//640-615
	char verText[256];
	F_GetVersionText(verText);
	if (lpMainVersionColour)
		palColour = (**lpMainVersionColour) | 0x06000000;
	WinPrintVersionText(*pWinRef_MainMenu, verText, 0, SCR_WIDTH - bgX - verX - GetTextWidth(verText), SCR_HEIGHT - bgY - verY, palColour);


	UNLSTDfrm* frmA = LoadUnlistedFrm("MENUUP.FRM", ART_INTRFACE);
	UNLSTDfrm* frmB = LoadUnlistedFrm("MENUDOWN.FRM", ART_INTRFACE);
	if (!frmA || !frmB) {
		F_MainMenuDestroy();
		return -1;
	}

	DWORD buttWidth = 0;
	DWORD buttHeight = 0;
	buttX = (LONG)(buttX * xRO);
	buttY = (LONG)(buttY * yRO);

	if ((!USE_HIRES_IMAGES && MAIN_MENU_SIZE == 0) || (USE_HIRES_IMAGES && SCALE_BUTTONS_AND_TEXT_MENU == 0)) {
		buttWidth = frmA->frames[0].width;
		buttHeight = frmA->frames[0].height;
		xRO = 1;
		yRO = 1;
	}
	else {
		buttWidth = (DWORD)(frmA->frames[0].width * xRO);
		buttHeight = (DWORD)(frmA->frames[0].height * yRO);
	}
	if (pFrm_MainMenuButts)
		delete pFrm_MainMenuButts;
	pFrm_MainMenuButts = CreateUnlistedFrm(buttWidth, buttHeight, 2, 1);
	MemBlt8Stretch(frmA->frames[0].buff, frmA->frames[0].width, frmA->frames[0].height, frmA->frames[0].width, pFrm_MainMenuButts->frames[0].buff, buttWidth, buttHeight, false, false);
	MemBlt8Stretch(frmB->frames[0].buff, frmA->frames[0].width, frmA->frames[0].height, frmA->frames[0].width, pFrm_MainMenuButts->frames[1].buff, buttWidth, buttHeight, false, false);
	delete frmA;
	frmA = nullptr;
	delete frmB;
	frmB = nullptr;

	LONG yOff = 0;
	LONG xPos = (LONG)(buttX + bgX);
	LONG yPos = 0;
	LONG i = 0;
	ButtonStruct* butt = nullptr;
	for (i = 0; i < 6; i++) {
		yPos = yOff - i;
		yPos = (LONG)(yPos * yRO + buttY + bgY);

		pMainMenuButtRefs[i] = CreateButton(*pWinRef_MainMenu, xPos, yPos, pFrm_MainMenuButts->frames[0].width, pFrm_MainMenuButts->frames[0].height,
			-1, -1, 1111, pMainMenuButtKeys[i],
			pFrm_MainMenuButts->frames[0].buff, pFrm_MainMenuButts->frames[1].buff, nullptr, FLG_ButtTrans);
		if (pMainMenuButtKeys[i] == 'o')
			OPTIONS_BUTTON = pMainMenuButtRefs[i];

		butt = GetButton(pMainMenuButtRefs[i], nullptr);
		butt->buffDefault = pFrm_MainMenuButts->frames[0].buff;
		yOff += 42;
	}

	if (USE_HIRES_IMAGES) {
		if (menuBG) {
			xPos += (DWORD)(menuBGOffX * xRO);
			yPos = (LONG)(buttY + bgY);
			yPos += (DWORD)(menuBGOffY * yRO);

			if (xPos < 0)xPos = 0;
			if (yPos < 0)yPos = 0;

			if (SCALE_BUTTONS_AND_TEXT_MENU == 0) {
				if (xPos + menuBG->frames[0].width > win->width)
					xPos = win->width - menuBG->frames[0].width;
				if (yPos + menuBG->frames[0].height > win->height)
					yPos = win->height - menuBG->frames[0].height;

				winBuff = win->buff + xPos + (yPos)*win->width;
				MemBltMasked8(menuBG->frames[0].buff, menuBG->frames[0].width, menuBG->frames[0].height, menuBG->frames[0].width, winBuff, win->width);
			}
			else {
				LONG newWidth = (DWORD)(menuBG->frames[0].width * xRO);
				LONG newHeight = (DWORD)(menuBG->frames[0].height * yRO);
				BYTE* newBuff = new BYTE[newWidth * newHeight];
				MemBlt8Stretch(menuBG->frames[0].buff, menuBG->frames[0].width, menuBG->frames[0].height, menuBG->frames[0].width, newBuff, newWidth, newHeight, false, false);

				if (xPos + newWidth > win->width)
					xPos = win->width - newWidth;
				if (yPos + newHeight > win->height)
					yPos = win->height - newHeight;
				winBuff = win->buff + xPos + (yPos)*win->width;
				MemBltMasked8(newBuff, newWidth, newHeight, newWidth, winBuff, win->width);
				delete[] newBuff;
			}
			delete menuBG;
			menuBG = nullptr;
		}
	}
	LONG mouseX, mouseY;
	F_GetMousePos(&mouseX, &mouseY);
	F_SetMousePos(mouseX, mouseY);
	SetFont(oldFont);
	*isMainmenu = 1;
	*isMainmenuHidden = 1;
	return 0;
}



//______________________________________
void __declspec(naked) main_menu_setup() {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		call MainMenuSetup
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}



//_________________________
void MainMenuFixes_CH(void) {

	lpMainCopyrightColour = (BYTE**)FixAddress(0x480C04);//FixAddress(0x6A8B33);
	lpMainVersionColour = (BYTE**)FixAddress(0x480C3C);//FixAddress(0x6A8B33);


	isMainmenu = (LONG*)0x5292F8;
	isMainmenuHidden = (LONG*)0x624DD8;

	pMainMenuButtRefs = (LONG*)0x624DC0;
	pMainMenuButtKeys = (LONG*)0x529300;

	F_GET_VERSION_TEXT = (void*)0x4B3110;

	F_HIDE_MAIN_MENU = (void*)0x480E30;
	F_SHOW_MAIN_MENU = (void*)0x480E78;

	F_MainMenuSetup = (LONG(__cdecl*)(void))0x480B10;
	F_MainMenuDestroy = (LONG(__cdecl*)(void))0x480D98;

	FuncWrite32(0x480DDD, 0xFFF98417, (DWORD)&h_delete_buttons);

	FuncReplace32(0x480E1C, 0x05B5A0, (DWORD)&fix_destroy_mainmenu_ref);

	MemWrite8(0x480B10, 0x53, 0xE9);
	FuncWrite32(0x480B11, 0x57565251, (DWORD)&main_menu_setup);

	///Version Print - for sfall
	P_WIN_PRINT_VERSION_TEXT = (void*)0x480C67;
	///return from version print - for sfall
	MemWrite8(0x480C6C, 0x6A, 0xC3);

}


//__________________________________
void MainMenuFixes_MULTI(int region) {

	pMenuMsgList = (MSGList*)FixAddress(0x58E940);
	lpMainMenuColour = (BYTE**)FixAddress(0x481906);//FixAddress(0x6A8B33);

	lpMainCopyrightColour = (BYTE**)FixAddress(0x481748);//FixAddress(0x6A8B33);
	lpMainVersionColour = (BYTE**)FixAddress(0x481780);//FixAddress(0x6A8B33);

	isMainmenu = (LONG*)FixAddress(0x519508);
	isMainmenuHidden = (LONG*)FixAddress(0x614858);

	pMainMenuButtRefs = (LONG*)FixAddress(0x614840);
	pMainMenuButtKeys = (LONG*)FixAddress(0x519510);

	F_GET_VERSION_TEXT = (void*)FixAddress(0x4B4580);

	F_HIDE_MAIN_MENU = (void*)FixAddress(0x481A00);
	F_SHOW_MAIN_MENU = (void*)FixAddress(0x481A48);

	F_MainMenuSetup = (LONG(__cdecl*)(void))FixAddress(0x481650);
	F_MainMenuDestroy = (LONG(__cdecl*)(void))FixAddress(0x481968);

	FuncReplace32(0x4819AD, 0xFFF978AF, (DWORD)&h_delete_buttons);

	FuncReplace32(0x4819EC, 0x054A78, (DWORD)&fix_destroy_mainmenu_ref);

	MemWrite8(0x481650, 0x53, 0xE9);
	FuncWrite32(0x481651, 0x57565251, (DWORD)&main_menu_setup);

	///Version Print - for sfall
	P_WIN_PRINT_VERSION_TEXT = (void*)FixAddress(0x4817AB);
	///return from version print - for sfall
	MemWrite8(0x4817B0, 0x6A, 0xC3);
}


//______________________________
void MainMenuFixes(DWORD region) {

	MAIN_MENU_SIZE = ConfigReadInt("MAINMENU", "MAIN_MENU_SIZE", 0);
	USE_HIRES_IMAGES = ConfigReadInt("MAINMENU", "USE_HIRES_IMAGES", 0);
	SCALE_BUTTONS_AND_TEXT_MENU = ConfigReadInt("MAINMENU", "SCALE_BUTTONS_AND_TEXT_MENU", 0);

	if (region == 4)
		MainMenuFixes_CH(), main_menu_region = 4;
	else
		MainMenuFixes_MULTI(region);
}
