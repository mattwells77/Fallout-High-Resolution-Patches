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
#include "configTools.h"
#include "WinFall.h"
#include "F_Art.h"


struct INVdim {
	DWORD fid;
	DWORD width;
	DWORD height;
	LONG x;
	LONG y;
};

INVdim* invDimList[6];//inv and barter dimension array address
DWORD dialogWidthFix = 639;

//dimension array structure: 5 vars
//DWORD Background frm num
//DWORD win width
//DWORD win height
//DWORD win x pos
//DWORD win y pos

//windows listed
//0x30 INVBOX.FRM       inventory
//0x71 use.frm          use-on
//0x72 loot.frm         loot or container
//0x6f barter.frm       barter
//0x131 MOVEMULT.FRM    move multiple items
//0x131 MOVEMULT.FRM    move multiple items

LONG invWinNum_current = 0;

WinStruct* DialogWinStruct = nullptr;
WinStruct* InvWinStruct = nullptr;

int DIALOG_SCRN_BACKGROUND = 0;

LONG* invMouseEdgeRight = nullptr;
LONG* invMouseEdgeBottom = nullptr;

bool invExists = false;

LONG* pInvListItemsVis = nullptr;

LONG* pRgt_Inv = nullptr;
LONG* pTop_Inv = nullptr;
LONG* pLft_Inv = nullptr;

LONG* pRgtPc_Loot = nullptr;
LONG* pTopPc_Loot = nullptr;
LONG* pLftPc_Loot = nullptr;
LONG* pRgtNpc_Loot = nullptr;
LONG* pTopNpc_Loot = nullptr;
LONG* pLftNpc_Loot = nullptr;

LONG* pRgtNpcTrade_Barter = nullptr;
LONG* pTopNpcTrade_Barter = nullptr;
LONG* pLftNpcTrade_Barter = nullptr;
LONG* pRgtPcTrade_Barter = nullptr;
LONG* pTopPcTrade_Barter = nullptr;
LONG* pLftPcTrade_Barter = nullptr;

LONG* pRgtNpcInv_Barter = nullptr;
LONG* pTopNpcInv_Barter = nullptr;
LONG* pLftNpcInv_Barter = nullptr;
LONG* pRgtPcInv_Barter = nullptr;
LONG* pTopPcInv_Barter = nullptr;
LONG* pLftPcInv_Barter = nullptr;



//__________________________________________
void __declspec(naked) h_inv_mouse_rec_pos() {

	__asm {
		pushad

		push edx
		push eax
		call F_GetMousePos
		add esp, 0x4//*mouse_x not tested

		mov eax, pWinRef_Inventory
		push dword ptr ds : [eax]
		call GetWinStruct
		add esp, 0x4

		mov ecx, eax
		pop edx//*mouse_y
		mov ebx, dword ptr ds : [ecx + 0xC]
		sub[edx], ebx //mouse_y - inv->rect_>top

		popad
		ret
	}
}


//____________________________________________________________________________________________________________________
void DrawFixedDialogArt(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	UNLSTDfrm* dialogBgArt = LoadUnlistedFrm("HR_ALLTLK.frm", ART_INTRFACE);
	if (dialogBgArt) {
		MemBlt8(dialogBgArt->frames[0].buff, subWidth, subHeight, dialogBgArt->frames[0].width, toBuff, toWidth);
		delete dialogBgArt;
	}
	else
		MemBlt8(fromBuff, subWidth, subHeight, fromWidth, toBuff, toWidth);
}


//__________________________________________________________________________________
int CheckMouseInInvRect(long left, long top, long right, long bottom, LONG* pWinRef) {

	if (*pWinRef == -1)
		return 0;
	WinStruct* win = GetWinStruct(*pWinRef);
	return F_CheckMouseInRect(left + win->rect.left, top + win->rect.top, right + win->rect.left, bottom + win->rect.top);
}


//________________________________________________
int CheckMouseInInvRects(int zDelta, int* keyCode) {

	//0=inv, 1=use-on, 2=loot, 3=barter, 4=move-multi, 5=move-multi
	int upCode = 0x148;
	int dnCode = 0x150;
	int numItemsScroll = 1;

	bool isFocusOnPrimaryMenu = false;

	if (ConfigReadInt("INPUT", "SCROLLWHEEL_FOCUS_PRIMARY_MENU", 1))
		isFocusOnPrimaryMenu = true;

	if (invExists) {
		numItemsScroll = *pInvListItemsVis;
		switch (invWinNum_current) {
		case 0:
		case 1:
			if (isFocusOnPrimaryMenu || CheckMouseInInvRect(*pLft_Inv - 80, *pTop_Inv, *pRgt_Inv - 80, *pInvListItemsVis * 48 + *pTop_Inv, pWinRef_Inventory))
				upCode = 0x148, dnCode = 0x150;//Up, Dn
			else
				upCode = -1, dnCode = -1;
			break;
		case 2:
			if (CheckMouseInInvRect(*pLftNpc_Loot - 80, *pTopNpc_Loot, *pRgtNpc_Loot - 80, *pInvListItemsVis * 48 + *pTopNpc_Loot, pWinRef_Inventory))
				upCode = 0x18D, dnCode = 0x191;//Ctrl-Up, Ctrl-Dn
			else if (isFocusOnPrimaryMenu || CheckMouseInInvRect(*pLftPc_Loot - 80, *pTopPc_Loot, *pRgtPc_Loot - 80, *pInvListItemsVis * 48 + *pTopPc_Loot, pWinRef_Inventory))
				upCode = 0x148, dnCode = 0x150;//Up, Dn
			else
				upCode = -1, dnCode = -1;
			break;
		case 3:
			if (CheckMouseInInvRect(*pLftNpcInv_Barter, *pTopNpcInv_Barter, *pRgtNpcInv_Barter, *pInvListItemsVis * 48 + *pTopNpcInv_Barter, pWinRef_DialogMain))
				upCode = 0x18D, dnCode = 0x191;//Ctrl-Up, Ctrl-Dn
			else if (CheckMouseInInvRect(*pLftPcTrade_Barter, *pTopPcTrade_Barter, *pRgtPcTrade_Barter, *pInvListItemsVis * 48 + *pTopPcTrade_Barter, pWinRef_DialogMain))
				upCode = 0x149, dnCode = 0x151;//PgUp, PgDn
			else if (CheckMouseInInvRect(*pLftNpcTrade_Barter, *pTopNpcTrade_Barter, *pRgtNpcTrade_Barter, *pInvListItemsVis * 48 + *pTopNpcTrade_Barter, pWinRef_DialogMain))
				upCode = 0x184, dnCode = 0x176;//Ctrl-PgUp, Ctrl-PgDn
			else if (isFocusOnPrimaryMenu || CheckMouseInInvRect(*pLftPcInv_Barter, *pTopPcInv_Barter, *pRgtPcInv_Barter, *pInvListItemsVis * 48 + *pTopPcInv_Barter, pWinRef_DialogMain))
				upCode = 0x148, dnCode = 0x150;//Up, Dn
			else
				upCode = -1, dnCode = -1;
			break;
		default:
			upCode = 0x148, dnCode = 0x150;//Up, Dn
			break;
		}
	}

	if (zDelta > 0)
		*keyCode = upCode;
	else if (zDelta < 0)
		*keyCode = dnCode;
	else
		*keyCode = -1;

	return numItemsScroll;
}


//_________________________________________
void __declspec(naked) h_dialog_mouse_rec() {

	__asm {
		push esi
		push ebp

		push pWinRef_DialogMain
		push ecx
		push ebx
		push edx
		push eax
		call CheckMouseInInvRect
		add esp, 0x14

		pop ebp
		pop esi
		ret
	}
}


//______________________________________
void __declspec(naked) h_inv_mouse_rec() {

	__asm {
		push esi
		push ebp

		sub eax, 80
		sub ebx, 80

		push pWinRef_Inventory
		push ecx
		push ebx
		push edx
		push eax
		call CheckMouseInInvRect
		add esp, 0x14

		pop ebp
		pop esi

		ret
	}
}


//_______________________________________________________________________________________________
int InvWinSetup(int x, int y, int width, int height, DWORD colour, DWORD winFlags, int invWinNum) {

	invWinNum_current = invWinNum;
	invDimList[invWinNum]->x = SCR_WIDTH / 2 - 240;

	if (*pWinRef_GameArea == -1)
		invDimList[invWinNum]->y = SCR_HEIGHT / 2 - height / 2;
	else
		AdjustWinPosToGame(width, height, &invDimList[invWinNum]->x, &invDimList[invWinNum]->y);

	int winNum = Win_Create(invDimList[invWinNum]->x, invDimList[invWinNum]->y, width, height, colour, winFlags);
	InvWinStruct = GetWinStruct(winNum);

	//move items window position
	for (int i = 4; i < 6; i++) {
		invDimList[i]->x = InvWinStruct->rect.left + 60;
		invDimList[i]->y = InvWinStruct->rect.top + 80;
	}

	if (winNum != -1)
		invExists = true;
	return winNum;
}


//__________________________________
void __declspec(naked) h_inventory() {

	__asm {
		push edi//inv win num
		push dword ptr ss : [esp + 0xC]
		push dword ptr ss : [esp + 0xC]

		push ecx
		push ebx
		push edx
		push eax
		call InvWinSetup
		add esp, 0x1C
		ret 0x8
	}
}


//________________________________________
void __declspec(naked) h_inv_win_destroy() {

	__asm {
		push eax
		call DestroyWin
		add esp, 0x4
		//mov pWinRef_Inventory, -1
		mov invExists, 0
		ret
	}

}


//_____________________________________________________________________________
int DialogSubWin(int x, int y, int width, int height, int colour, int winFlags) {

	WinStruct* win = GetWinStruct(*pWinRef_DialogMain);
	return Win_Create(x + win->rect.left, y + win->rect.top, width, height, colour, winFlags);
}


//_______________________________________
void __declspec(naked) h_dialog_sub_win() {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]

		push ecx
		push ebx
		push edx
		push eax
		call DialogSubWin
		add esp, 0x18

		ret 0x8
	}
}


//______________________________________________
void __declspec(naked) h_dialog_sub_win_barter() {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]

		push ecx
		push ebx
		push edx
		push eax
		call DialogSubWin
		add esp, 0x18

		//mouseMaxInvX & mouseMaxInvY settings for barter panel
		//ignored by other funcs
		mov ecx, DialogWinStruct
		mov ebx, dword ptr ds : [ecx + 0x8]
		add ebx, 0x230
		mov ecx, dword ptr ds : [ecx + 0xC]
		add ecx, 0x1D6

		mov invExists, 1
		mov invWinNum_current, 3
		ret 0x8
	}

}


//_______________________________________
void __declspec(naked) mouseMaxInvX(void) {

	__asm {
		mov edx, InvWinStruct
		mov edx, dword ptr ds : [edx + 0x10]  //window xMax
		add edx, 1
		ret
	}
}


//_______________________________________
void __declspec(naked) mouseMaxInvY(void) {

	__asm {
		mov edx, InvWinStruct
		mov edx, dword ptr ds : [edx + 0x14]  //window yMax
		add edx, 1
		ret
	}

}


//__________________________________________________________________________________________
LONG DialogWinSetup(LONG x, LONG y, DWORD width, DWORD height, DWORD colour, DWORD winFlags) {

	if (DIALOG_SCRN_BACKGROUND == 1) {
		HideWin(*pWinRef_GameArea);
		HideIfaceWindows();
	}

	LONG winNum = -1;
	if (*pWinRef_GameArea == -1)
		winNum = Win_Create(SCR_WIDTH / 2 - 320, SCR_HEIGHT / 2 - 240, 640, 480, colour, winFlags);
	else {
		AdjustWinPosToGame(640, 480, &x, &y);
		winNum = Win_Create(x, y, 640, 480, colour, winFlags);
	}

	DialogWinStruct = GetWinStruct(winNum);
	dialogWidthFix = DialogWinStruct->width - 1;

	invDimList[3]->x = DialogWinStruct->rect.left + 80;
	invDimList[3]->y = DialogWinStruct->rect.top + 290;

	for (int i = 4; i < 6; i++) {
		invDimList[i]->x = DialogWinStruct->rect.left + 140;
		invDimList[i]->y = DialogWinStruct->rect.top + 80;
	}

	return winNum;
}


//________________________________________
void __declspec(naked) h_dialog_main_win() {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]

		push ecx
		push ebx
		push edx
		push eax
		call DialogWinSetup
		add esp, 0x18
		ret 0x8
	}
}


//______________________________________
void __declspec(naked) h_dialog_bg(void) {

	__asm {
		push eax
		call DestroyWin //destroy dialog win
		add esp, 0x4
		//reshow view and iface windows if DIALOG_SCRN_BACKGROUND was set to hide
		cmp DIALOG_SCRN_BACKGROUND, 1
		jne skip_bg
		mov eax, pWinRef_GameArea
		push dword ptr ds : [eax]
		call ShowWin
		add esp, 0x4
		mov eax, pWinRef_GameArea
		push dword ptr ds : [eax]
		call RedrawWin
		add esp, 0x4
		call ShowIfaceWindows
		skip_bg :
		mov ebp, -1
			ret
	}
}


//_____________________________________________
void __declspec(naked) h_dialog_pc_txt_bg(void) {

	//add DialogWinStruct*640 to fbuff - for pc text background position
	__asm {
		pop ebx
		mov eax, DialogWinStruct
		mov eax, dword ptr ds : [eax + 0xC]
		lea edx, [eax * 4]
		add eax, edx
		shl eax, 0x7
		add[esp], eax
		call MemBlt8
		push ebx
		ret
	}
}


//_______________________________________________
void __declspec(naked) custom_combat_choice(void) {

	__asm {
		mov ebx, DialogWinStruct
		add eax, dword ptr ds : [ebx + 0x8]
		mov edx, dword ptr ds : [ebx + 0xC]
		mov ebx, dword ptr ds : [ebx + 0x1C]
		shr ebx, 1
		add edx, ebx
		shl edx, 1
		ret
	}
}


//_____________________
void ReSize_InvDialog() {

	WinStruct* win = nullptr;
	if (*pWinRef_Inventory != -1) {
		win = GetWinStruct(*pWinRef_Inventory);
		if (win) {
			invDimList[invWinNum_current]->x = win->rect.left;//SCR_WIDTH/2-240;
			invDimList[invWinNum_current]->y = win->rect.top;//SCR_HEIGHT/2-win->height/2;
			//move items window position
			for (int i = 4; i < 6; i++) {
				//normal but bound to window x pos
				invDimList[i]->x = win->rect.left + 60;
				//normal but bound to window y pos
				invDimList[i]->y = win->rect.top + 80;
			}
		}
	}

	if (*pWinRef_DialogMain != -1) {
		win = GetWinStruct(*pWinRef_DialogMain);
		if (win) {
			invDimList[3]->x = win->rect.left + 80;
			invDimList[3]->y = win->rect.top + 290;
			*invMouseEdgeRight = win->rect.left + 560;
			*invMouseEdgeBottom = win->rect.top + 470;

			for (int i = 4; i < 6; i++) {
				invDimList[i]->x = win->rect.left + 140;
				invDimList[i]->y = win->rect.top + 80;
			}

			WinStruct* winSub = nullptr;
			if (*pWinRef_DialogNpcText != -1)
				winSub = GetWinStruct(*pWinRef_DialogNpcText);
			if (winSub) {
				winSub->rect.left = win->rect.left + 135;
				winSub->rect.top = win->rect.top + 225;
				winSub->rect.right = winSub->rect.left + winSub->width - 1;
				winSub->rect.bottom = winSub->rect.top + winSub->height - 1;
			}
			if (*pWinRef_DialogPcText != -1)
				winSub = GetWinStruct(*pWinRef_DialogPcText);
			if (winSub) {
				winSub->rect.left = win->rect.left + 127;
				winSub->rect.top = win->rect.top + 335;
				winSub->rect.right = winSub->rect.left + winSub->width - 1;
				winSub->rect.bottom = winSub->rect.top + winSub->height - 1;
			}
			if (*pWinRef_DialogBaseSub != -1)
				winSub = GetWinStruct(*pWinRef_DialogBaseSub);
			if (winSub) {
				winSub->rect.left = win->rect.left;
				winSub->rect.top = win->rect.bottom - winSub->height + 1;
				winSub->rect.right = winSub->rect.left + winSub->width - 1;
				winSub->rect.bottom = winSub->rect.top + winSub->height - 1;
			}
		}
	}
}


//____________________________
void DialogInventoryFixes_CH() {

	pInvListItemsVis = (LONG*)0x528E44;

	DWORD offset = 0;
	for (int i = 0; i < 6; i++) {
		invDimList[i] = (INVdim*)(0x528E58 + offset);
		offset += 20;
	}

	MemWrite16(0x46E413, 0x968B, 0xE890);
	FuncWrite32(0x46E415, 0x528E5C, (DWORD)&mouseMaxInvX);

	MemWrite16(0x46E429, 0x968B, 0xE890);
	FuncWrite32(0x46E42B, 0x528E60, (DWORD)&mouseMaxInvY);


	//INVENTORY STUFF----------------------------------------------------------------------------------------------------------

	//MOUSE DROP-----------
	FuncWrite32(0x470846, 0x05B64A, (DWORD)&h_inv_mouse_rec);

	//fix for dropping stuff in bags
	FuncReplace32(0x47085B, 0x05B773, (DWORD)&h_inv_mouse_rec_pos);

	//LEFT HAND
	FuncWrite32(0x470971, 0x05B51F, (DWORD)&h_inv_mouse_rec);

	//RIGHT HAND
	FuncWrite32(0x470A02, 0x05B48E, (DWORD)&h_inv_mouse_rec);

	//ARMOR
	FuncWrite32(0x470A8C, 0x05B404, (DWORD)&h_inv_mouse_rec);


	//use pack on
	//FuncWrite32(0x470B58, 0x05B338, (DWORD)&h_inv_mouse_rec);

	//CONTAINER RIGHT MOUSE
	FuncWrite32(0x473F56, 0x057F3A, (DWORD)&h_inv_mouse_rec);

	//CONTAINER LEFT MOUSE
	FuncWrite32(0x474020, 0x057E70, (DWORD)&h_inv_mouse_rec);

	pRgt_Inv = (LONG*)0x470834;
	pTop_Inv = (LONG*)0x47083C;
	pLft_Inv = (LONG*)0x470841;

	pRgtNpc_Loot = (LONG*)0x473F45;
	pTopNpc_Loot = (LONG*)0x473F4A;
	pLftNpc_Loot = (LONG*)0x473F51;

	pRgtPc_Loot = (LONG*)0x47400F;
	pTopPc_Loot = (LONG*)0x474014;
	pLftPc_Loot = (LONG*)0x47401B;

	FuncWrite32(0x46E40F, 0x06DCAA, (DWORD)&h_inventory);

	FuncWrite32(0x46E4CA, 0x06DBEF, (DWORD)&h_dialog_sub_win_barter);

	FuncReplace32(0x46F3B1, 0x06D00B, (DWORD)&h_inv_win_destroy);


	//DIALOG SCRN STUFF-------------------------------------------

	//CUSTOM COMBAT BUTTON MOUSE SELECTION RECTANGLE
	FuncWrite32(0x448E26, 0x08306A, (DWORD)&h_dialog_mouse_rec);

	//DIALOG_NPC_RESPONSE TEXT SCRN POSITION
	FuncWrite32(0x445AC0, 0x0965F9, (DWORD)&h_dialog_sub_win);


	//DIALOG_CHARACTOR_RESPONSE TEXT SCRN POSITION
	FuncWrite32(0x445BA0, 0x096519, (DWORD)&h_dialog_sub_win);


	//CUSTOM COMBAT CHOICE PANEL POSITION
	MemWrite8(0x449809, 0xBA, 0xE8);
	FuncWrite32(0x44980A, 480, (DWORD)&custom_combat_choice);


	//BAKGROUNG CAPTURE YPOS FOR DIALOG_CHARACTOR_RESPONSE TEXT SCRN
	FuncWrite32(0x447125, 0x091D93, (DWORD)&h_dialog_pc_txt_bg);
	FuncWrite32(0x44726B, 0x091C4D, (DWORD)&h_dialog_pc_txt_bg);


	//BARTER STUFF PANEL------
	MemWrite8(0x46E4CE, 0xBB, 0x90);
	MemWrite32(0x46E4CF, 560, 0x90909090);
	MemWrite8(0x46E4D3, 0xB9, 0x90);
	MemWrite32(0x46E4D4, 470, 0x90909090);

	FuncWrite32(0x474573, 0x05791D, (DWORD)&h_dialog_mouse_rec);

	FuncWrite32(0x4745F6, 0x05789A, (DWORD)&h_dialog_mouse_rec);

	FuncWrite32(0x47483E, 0x057652, (DWORD)&h_dialog_mouse_rec);

	FuncWrite32(0x4748BF, 0x0575D1, (DWORD)&h_dialog_mouse_rec);


	pRgtPcTrade_Barter = (LONG*)0x474562;
	pTopPcTrade_Barter = (LONG*)0x474567;
	pLftPcTrade_Barter = (LONG*)0x47456E;

	pRgtNpcTrade_Barter = (LONG*)0x4745E5;
	pTopNpcTrade_Barter = (LONG*)0x4745EA;
	pLftNpcTrade_Barter = (LONG*)0x4745F1;


	pRgtPcInv_Barter = (LONG*)0x47482D;
	pTopPcInv_Barter = (LONG*)0x474832;
	pLftPcInv_Barter = (LONG*)0x474839;

	pRgtNpcInv_Barter = (LONG*)0x4748AE;
	pTopNpcInv_Barter = (LONG*)0x4748B3;
	pLftNpcInv_Barter = (LONG*)0x4748BA;


	if (ConfigReadInt("OTHER_SETTINGS", "BARTER_PC_INV_DROP_FIX", 1)) {
		MemWrite32(0x47482D, 144, 164);
		MemWrite32(0x474839, 80, 100);
	}

	//DIALOG PANEL WIDTH FIXES--ORIGINALY SET TO GLOBAL SCRN SIZE-----------------------
	//7F02 = 639

	DWORD width_fix[51]{ 0 };

	width_fix[0] = 0x445204;
	width_fix[1] = 0x445220;
	width_fix[2] = 0x4455BB;
	width_fix[3] = 0x4455E5;
	width_fix[4] = 0x446A25;
	width_fix[5] = 0x446A6C;
	width_fix[6] = 0x447667;
	width_fix[7] = 0x447699;
	width_fix[8] = 0x447706;
	width_fix[9] = 0x447722;
	width_fix[10] = 0x447B14;
	width_fix[11] = 0x447D6C;
	width_fix[12] = 0x447FBF;
	width_fix[13] = 0x448460;
	width_fix[14] = 0x448F64;
	width_fix[15] = 0x449264;
	width_fix[16] = 0x449E51;
	width_fix[17] = 0x44A174;
	width_fix[18] = 0x44A22A;
	width_fix[19] = 0x44A298;
	width_fix[20] = 0x44A2B6;
	width_fix[21] = 0x44A36F;
	width_fix[22] = 0x44A599;
	width_fix[23] = 0x44A6E0;
	width_fix[24] = 0x44A721;
	width_fix[25] = 0x44A761;
	width_fix[26] = 0x46E504;
	width_fix[27] = 0x46F75A;
	width_fix[28] = 0x46F78F;
	width_fix[29] = 0x46FB01;
	width_fix[30] = 0x46FB3E;
	width_fix[31] = 0x46FF0E;
	width_fix[32] = 0x4729DB;
	width_fix[33] = 0x472A36;
	width_fix[34] = 0x474431;
	width_fix[35] = 0x474472;
	width_fix[36] = 0x4746FA;
	width_fix[37] = 0x474735;
	width_fix[38] = 0x4749BD;
	width_fix[39] = 0x4749FC;
	width_fix[40] = 0x474B9B;
	width_fix[41] = 0x474BF1;
	width_fix[42] = 0x4475D9;
	width_fix[43] = 0x4493F0;
	width_fix[44] = 0x44A4C7;
	width_fix[45] = 0x44A5D4;
	width_fix[46] = 0x44518E;
	width_fix[47] = 0x447AD2;
	width_fix[48] = 0x447F7D;
	width_fix[49] = 0x448F22;
	width_fix[50] = 0x449E0F;

	for (int i = 0; i < 51; i++)
		MemWrite32(width_fix[i], 0x6BCF64, (DWORD)&dialogWidthFix);

	FuncWrite32(0x4469B4, 0x095705, (DWORD)&h_dialog_main_win);

	FuncWrite32(0x446AFD, 0x0958BF, (DWORD)&h_dialog_bg);

	//DIALOG_HISTORY
	FuncWrite32(0x4451AC, 0x096F0D, (DWORD)&h_dialog_sub_win);

	//DIALOG_BARTER
	FuncWrite32(0x447AF4, 0x0945C5, (DWORD)&h_dialog_sub_win);

	//DIALOG_COMBAT
	FuncWrite32(0x447F8F, 0x09412A, (DWORD)&h_dialog_sub_win);

	//DIALOG_CUSTOM
	FuncWrite32(0x448F34, 0x093185, (DWORD)&h_dialog_sub_win);

	//DIALOG_TALK
	FuncWrite32(0x449E31, 0x092288, (DWORD)&h_dialog_sub_win);

	invMouseEdgeRight = (LONG*)0x5AEEF4;
	invMouseEdgeBottom = (LONG*)0x5AEEF0;

	if (ConfigReadInt("OTHER_SETTINGS", "DIALOG_SCRN_ART_FIX", 1)) {

		FuncReplace32(0x44A2C6, 0x08EBF2, (DWORD)&DrawFixedDialogArt);

		MemWrite16(0x446A79, 0x048D, 0xD269);
		MemWrite32(0x446A7B, 0xD5, 14 + 5);
		MemWrite8(0x446A7F, 0x00, 0x90);
		MemWrite16(0x446A80, 0xD029, 0x03EB);

		//mask top right y and x
		MemWrite8(0x44A6E9, 0x0F, 0x0F + 5);
		//mask bottom left ybottom and x
		MemWrite32(0x44A737, 0xD6, 0xD6 + 5);

		//draw_win_area rect
		MemWrite32(0x44A693, 0x0E, 0x0E + 5);
		MemWrite32(0x44A69D, 0xD6, 0xD6 + 5);

		MemWrite32(0x52853C, 0x0E, 0x0E + 5);
		MemWrite32(0x528544, 0x28, 0x28 + 5);

		MemWrite32(0x52854C, 0x0E, 0x0E + 5);
		MemWrite32(0x528554, 0x28, 0x28 + 5);

		MemWrite32(0x52855C, 0xBC, 0xBC + 5);
		MemWrite32(0x528564, 0xD6, 0xD6 + 5);

		MemWrite32(0x52856C, 0xBC, 0xBC + 5);
		MemWrite32(0x528574, 0xD6, 0xD6 + 5);

		MemWrite32(0x52857C, 0x0E, 0x0E + 5);
		MemWrite32(0x528584, 0x18, 0x18 + 5);

		MemWrite32(0x52858C, 0xCC, 0xCC + 5);
		MemWrite32(0x528594, 0xD6, 0xD6 + 5);

		MemWrite32(0x52859C, 0x28, 0x28 + 5);
		MemWrite32(0x5285A4, 0xBC, 0xBC + 5);

		MemWrite32(0x5285AC, 0x28, 0x28 + 5);
		MemWrite32(0x5285B4, 0xBC, 0xBC + 5);
	}
}



//___________________________________
void DialogInventoryFixes_MULTI(void) {

	pInvListItemsVis = (LONG*)FixAddress(0x519054);

	DWORD offset = 0;
	for (int i = 0; i < 6; i++) {
		invDimList[i] = (INVdim*)FixAddress(0x519068 + offset);
		offset += 20;
	}

	MemWrite16(0x46ED13, 0x968B, 0xE890);
	FuncWrite32(0x46ED15, FixAddress(0x51906C), (DWORD)&mouseMaxInvX);

	MemWrite16(0x46ED29, 0x968B, 0xE890);
	FuncWrite32(0x46ED2B, FixAddress(0x519070), (DWORD)&mouseMaxInvY);


	//INVENTORY STUFF----------------------------------------------

	//MOUSE LIFT & DROP RECTS

	//inv item list rect
	FuncReplace32(0x471146, 0x0597EA, (DWORD)&h_inv_mouse_rec);

	//fix for dropping stuff in bags
	FuncReplace32(0x47115B, 0x05987D, (DWORD)&h_inv_mouse_rec_pos);

	//LEFT HAND
	FuncReplace32(0x471271, 0x0596BF, (DWORD)&h_inv_mouse_rec);
	//RIGHT HAND
	FuncReplace32(0x471302, 0x05962E, (DWORD)&h_inv_mouse_rec);
	//ARMOR
	FuncReplace32(0x47138C, 0x0595A4, (DWORD)&h_inv_mouse_rec);
	//rotating pc
	//FuncReplace32(0x471458, 0x0594D8, (DWORD)&h_inv_mouse_rec);
	//CONTAINER RIGHT
	FuncReplace32(0x47495A, 0x055FD6, (DWORD)&h_inv_mouse_rec);
	//CONTAINER LEFT
	FuncReplace32(0x474A24, 0x055F0C, (DWORD)&h_inv_mouse_rec);

	pRgt_Inv = (LONG*)FixAddress(0x471134);
	pTop_Inv = (LONG*)FixAddress(0x47113C);
	pLft_Inv = (LONG*)FixAddress(0x471141);

	pRgtNpc_Loot = (LONG*)FixAddress(0x474949);
	pTopNpc_Loot = (LONG*)FixAddress(0x47494E);
	pLftNpc_Loot = (LONG*)FixAddress(0x474955);

	pRgtPc_Loot = (LONG*)FixAddress(0x474A13);
	pTopPc_Loot = (LONG*)FixAddress(0x474A18);
	pLftPc_Loot = (LONG*)FixAddress(0x474A1F);

	FuncReplace32(0x46ED0F, 0x067525, (DWORD)&h_inventory);

	FuncReplace32(0x46EDCA, 0x06746A, (DWORD)&h_dialog_sub_win_barter);

	FuncReplace32(0x46FCB1, 0x0667B3, (DWORD)&h_inv_win_destroy);

	//DIALOG SCRN STUFF--------------------------------------

	//CUSTOM COMBAT BUTTON MOUSE SELECTION RECTANGLE
	FuncReplace32(0x449662, 0x0812CE, (DWORD)&h_dialog_mouse_rec);

	//DIALOG_NPC_RESPONSE TEXT SCRN POSITION
	FuncReplace32(0x4462A8, 0x08FF8C, (DWORD)&h_dialog_sub_win);

	//DIALOG_CHARACTOR_RESPONSE TEXT SCRN POSITION
	FuncReplace32(0x446388, 0x08FEAC, (DWORD)&h_dialog_sub_win);

	//CUSTOM COMBAT CHOICE PANEL POSITION
	MemWrite8(0x44A03D, 0xBA, 0xE8);
	FuncReplace32(0x44A03E, 480, (DWORD)&custom_combat_choice);

	//BAKGROUNG CAPTURE YPOS FOR DIALOG_CHARACTOR_RESPONSE TEXT SCRN
	FuncReplace32(0x447901, 0x08BDCF, (DWORD)&h_dialog_pc_txt_bg);
	FuncReplace32(0x447A47, 0x08BC89, (DWORD)&h_dialog_pc_txt_bg);

	//BARTER STUFF PANEL------
	//mouseMaxInvX & mouseMaxInvY handled in h_dialog_sub_win
	MemWrite8(0x46EDCE, 0xBB, 0x90);
	MemWrite32(0x46EDCF, 560, 0x90909090);
	MemWrite8(0x46EDD3, 0xB9, 0x90);
	MemWrite32(0x46EDD4, 470, 0x90909090);

	//barter NPC trade
	FuncReplace32(0x474F77, 0x0559B9, (DWORD)&h_dialog_mouse_rec);
	//barter PC trade
	FuncReplace32(0x474FFA, 0x055936, (DWORD)&h_dialog_mouse_rec);
	//barter NPC inv
	FuncReplace32(0x475242, 0x0556EE, (DWORD)&h_dialog_mouse_rec);
	//barter PC inv
	FuncReplace32(0x4752C3, 0x05566D, (DWORD)&h_dialog_mouse_rec);

	pRgtPcTrade_Barter = (LONG*)FixAddress(0x474F66);
	pTopPcTrade_Barter = (LONG*)FixAddress(0x474F6B);
	pLftPcTrade_Barter = (LONG*)FixAddress(0x474F72);

	pRgtNpcTrade_Barter = (LONG*)FixAddress(0x474FE9);
	pTopNpcTrade_Barter = (LONG*)FixAddress(0x474FEE);
	pLftNpcTrade_Barter = (LONG*)FixAddress(0x474FF5);

	pRgtPcInv_Barter = (LONG*)FixAddress(0x475231);
	pTopPcInv_Barter = (LONG*)FixAddress(0x475236);
	pLftPcInv_Barter = (LONG*)FixAddress(0x47523D);

	pRgtNpcInv_Barter = (LONG*)FixAddress(0x4752B2);
	pTopNpcInv_Barter = (LONG*)FixAddress(0x4752B7);
	pLftNpcInv_Barter = (LONG*)FixAddress(0x4752BE);


	if (ConfigReadInt("OTHER_SETTINGS", "BARTER_PC_INV_DROP_FIX", 1)) {
		MemWrite32(0x475231, 144, 164);
		MemWrite32(0x47523D, 80, 100);
	}

	//DIALOG PANEL WIDTH FIXES--ORIGINALY SET TO GLOBAL SCRN SIZE-----------------------
	//7F02 = 639
	DWORD width_fix[51]{ 0 };

	width_fix[0] = 0x4459F0;
	width_fix[1] = 0x445A0C;
	width_fix[2] = 0x445DA7;
	width_fix[3] = 0x445DD1;
	width_fix[4] = 0x447201;
	width_fix[5] = 0x447248;
	width_fix[6] = 0x447E43;
	width_fix[7] = 0x447E75;
	width_fix[8] = 0x447EE2;
	width_fix[9] = 0x447EFE;
	width_fix[10] = 0x448354;
	width_fix[11] = 0x4485AC;
	width_fix[12] = 0x4487FF;
	width_fix[13] = 0x448CA0;
	width_fix[14] = 0x4497A0;
	width_fix[15] = 0x449AA0;
	width_fix[16] = 0x44A705;
	width_fix[17] = 0x44AA28;
	width_fix[18] = 0x44AADE;
	width_fix[19] = 0x44AB4C;
	width_fix[20] = 0x44AB6A;
	width_fix[21] = 0x44AC23;
	width_fix[22] = 0x44AE4D;
	width_fix[23] = 0x44AF94;
	width_fix[24] = 0x44AFD5;
	width_fix[25] = 0x44B015;
	width_fix[26] = 0x46EE04;
	width_fix[27] = 0x47005A;
	width_fix[28] = 0x47008F;
	width_fix[29] = 0x470401;
	width_fix[30] = 0x47043E;
	width_fix[31] = 0x47080E;
	width_fix[32] = 0x4733DF;
	width_fix[33] = 0x47343A;
	width_fix[34] = 0x474E35;
	width_fix[35] = 0x474E76;
	width_fix[36] = 0x4750FE;
	width_fix[37] = 0x475139;
	width_fix[38] = 0x4753C1;
	width_fix[39] = 0x475400;
	width_fix[40] = 0x47559F;
	width_fix[41] = 0x4755F5;
	width_fix[42] = 0x447DB5;
	width_fix[43] = 0x449C2C;
	width_fix[44] = 0x44AD7B;
	width_fix[45] = 0x44AE88;
	width_fix[46] = 0x44597A;
	width_fix[47] = 0x448312;
	width_fix[48] = 0x4487BD;
	width_fix[49] = 0x44975E;
	width_fix[50] = 0x44A6C3;

	for (int i = 0; i < 51; i++)
		MemWrite32(width_fix[i], 0x6AC9F8, (DWORD)&dialogWidthFix);

	FuncReplace32(0x447190, 0x08F0A4, (DWORD)&h_dialog_main_win);

	FuncReplace32(0x4472D9, 0x08F18B, (DWORD)&h_dialog_bg);

	//DIALOG_HISTORY
	FuncReplace32(0x445998, 0x09089C, (DWORD)&h_dialog_sub_win);

	//DIALOG_BARTER
	FuncReplace32(0x448334, 0x08DF00, (DWORD)&h_dialog_sub_win);

	//DIALOG_COMBAT
	FuncReplace32(0x4487CF, 0x08DA65, (DWORD)&h_dialog_sub_win);

	//DIALOG_CUSTOM
	FuncReplace32(0x449770, 0x08CAC4, (DWORD)&h_dialog_sub_win);

	//DIALOG_TALK
	FuncReplace32(0x44A6E5, 0x08BB4F, (DWORD)&h_dialog_sub_win);

	invMouseEdgeRight = (LONG*)FixAddress(0x59E974);
	invMouseEdgeBottom = (LONG*)FixAddress(0x59E970);

	if (ConfigReadInt("OTHER_SETTINGS", "DIALOG_SCRN_ART_FIX", 1)) {

		FuncReplace32(0x44AB7A, 0x088B56, (DWORD)&DrawFixedDialogArt);

		MemWrite16(0x447255, 0x048D, 0xD269);
		MemWrite32(0x447257, 0xD5, 14 + 5);
		MemWrite8(0x44725B, 0x00, 0x90);
		MemWrite16(0x44725C, 0xD029, 0x03EB);

		//mask top right y and x
		MemWrite8(0x44AF9D, 0x0F, 0x0F + 5);
		//mask bottom left ybottom and x
		MemWrite32(0x44AFEB, 0xD6, 0xD6 + 5);
		//draw_win_area rect
		MemWrite32(0x44AF47, 0x0E, 0x0E + 5);
		MemWrite32(0x44AF51, 0xD6, 0xD6 + 5);

		MemWrite32(0x51874C, 0x0E, 0x0E + 5);
		MemWrite32(0x518754, 0x28, 0x28 + 5);

		MemWrite32(0x51875C, 0x0E, 0x0E + 5);
		MemWrite32(0x518764, 0x28, 0x28 + 5);

		MemWrite32(0x51876C, 0xBC, 0xBC + 5);
		MemWrite32(0x518774, 0xD6, 0xD6 + 5);

		MemWrite32(0x51877C, 0xBC, 0xBC + 5);
		MemWrite32(0x518784, 0xD6, 0xD6 + 5);

		MemWrite32(0x51878C, 0x0E, 0x0E + 5);
		MemWrite32(0x518794, 0x18, 0x18 + 5);

		MemWrite32(0x51879C, 0xCC, 0xCC + 5);
		MemWrite32(0x5187A4, 0xD6, 0xD6 + 5);

		MemWrite32(0x5187AC, 0x28, 0x28 + 5);
		MemWrite32(0x5187B4, 0xBC, 0xBC + 5);

		MemWrite32(0x5187BC, 0x28, 0x28 + 5);
		MemWrite32(0x5187C4, 0xBC, 0xBC + 5);
	}
}


//_____________________________________
void DialogInventoryFixes(DWORD region) {

	DIALOG_SCRN_BACKGROUND = ConfigReadInt("OTHER_SETTINGS", "DIALOG_SCRN_BACKGROUND", 0);

	if (region == 4)
		DialogInventoryFixes_CH();
	else
		DialogInventoryFixes_MULTI();
}
