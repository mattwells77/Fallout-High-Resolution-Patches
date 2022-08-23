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
#include "F_Objects.h"

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

int invWinNum_current = 0;

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

LONG* pItemListPos = nullptr;
PUD** pPcPud = nullptr;
LONG* pInvListNum = nullptr;

void* F_DRAW_ITEM_LIST = nullptr;


//_______________________________________________________
void F_DrawItemList(LONG listPos, LONG val1, LONG invNum) {

	__asm {
		mov ebx, invNum
		mov edx, val1
		mov eax, listPos
		call F_DRAW_ITEM_LIST
	}
}


//________________________________________________
LONG InvCheckKeys(LONG keyCode, LONG invWindowNum) {

	switch (keyCode) {
	case 0x149: {//PgUn
		pItemListPos[*pInvListNum] -= *pInvListItemsVis;
		if (pItemListPos[*pInvListNum] < 0)
			pItemListPos[*pInvListNum] = 0;
		F_DrawItemList(pItemListPos[*pInvListNum], -1, invWindowNum);
	}
			  break;
	case 0x151: {//PgDn
		PUD* pud = *pPcPud;
		pItemListPos[*pInvListNum] = pItemListPos[*pInvListNum] + *pInvListItemsVis;
		if (pItemListPos[*pInvListNum] + *pInvListItemsVis >= pud->general.inv_size) {
			pItemListPos[*pInvListNum] = pud->general.inv_size - *pInvListItemsVis;
			if (pItemListPos[*pInvListNum] < 0)
				pItemListPos[*pInvListNum] = 0;
		}
		F_DrawItemList(pItemListPos[*pInvListNum], -1, invWindowNum);
	}
			  break;
	case 0x147://Home
		pItemListPos[*pInvListNum] = 0;
		F_DrawItemList(pItemListPos[*pInvListNum], -1, invWindowNum);
		break;
	case 0x14F: {//End
		PUD* pud = *pPcPud;
		pItemListPos[*pInvListNum] = pud->general.inv_size - *pInvListItemsVis;
		if (pItemListPos[*pInvListNum] < 0)
			pItemListPos[*pInvListNum] = 0;
		F_DrawItemList(pItemListPos[*pInvListNum], -1, invWindowNum);
	}
			  break;
	default:
		return keyCode;
	}
	return -1;
}


//_____________________________________
void __declspec(naked) inv_check_keys() {

	__asm {
		push eax
		push edx
		push ecx
		push esi
		push edi
		push ebp

		push ebp//invWindowNum
		push ebx//keyCode
		call InvCheckKeys
		add esp, 0x8
		mov ebx, eax

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop edx
		pop eax

		///cmp ebx, 0x144
		cmp ebx, 0x150
		ret
	}
}


//__________________________________________________________________
void InvInsertItem(OBJStruct* item, LONG numItems, PUD_GENERAL* inv) {

	LONG numNodes = inv->inv_max;
	ITEMnode* itemNode = inv->item;
	if (itemNode && numNodes > 1) {
		for (int i = numNodes - 1; i > 0; i--) {
			itemNode[i].obj = itemNode[i - 1].obj;
			itemNode[i].num = itemNode[i - 1].num;
		}
	}
	itemNode[0].obj = item;
	itemNode[0].num = numItems;
}


//______________________________________
void __declspec(naked) inv_insert_item() {

	__asm {
		mov edx, dword ptr ss : [esp + 0x20]//num items to insert
		pushad
		push ecx//pObjPud_To
		push edx
		push edi//*pObjItem
		call InvInsertItem
		add esp, 0xC
		popad
		ret
	}
}



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
	//centred
	//invDimList[invWinNum]->x = SCR_WIDTH/2-width/2;//+50;
	//normal
	//invDimList[invWinNum]->x = SCR_WIDTH/2-320+80;
	invDimList[invWinNum]->x = SCR_WIDTH / 2 - 240;

	if (*pWinRef_GameArea == -1)
		invDimList[invWinNum]->y = SCR_HEIGHT / 2 - height / 2;
	else {
		AdjustWinPosToGame(width, height, &invDimList[invWinNum]->x, &invDimList[invWinNum]->y);
	}

	int winNum = Win_Create(invDimList[invWinNum]->x, invDimList[invWinNum]->y, width, height, colour, winFlags);
	InvWinStruct = GetWinStruct(winNum);

	//move items window position
	for (int i = 4; i < 6; i++) {
		//centred
		//invDimList[i]->x=InvWinStruct->rect.left+140;
		//normal
		//invDimList[i]->x=SCR_WIDTH/2-320+140;
		//normal but bound to window x pos
		invDimList[i]->x = InvWinStruct->rect.left + 60;
		//normal but bound to window y pos
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


//__________________________________
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


//_________________________
void DialogInventoryFixes() {

	DIALOG_SCRN_BACKGROUND = ConfigReadInt("OTHER_SETTINGS", "DIALOG_SCRN_BACKGROUND", 0);

	pInvListItemsVis = (LONG*)FixAddress(0x505730);

	int offset = 0;
	int i = 0;
	for (i = 0; i < 6; i++) {
		invDimList[i] = (INVdim*)FixAddress(0x505744 + offset);
		offset += 20;
	}

	pItemListPos = (LONG*)FixAddress(0x59CD94);
	pPcPud = (PUD**)FixAddress(0x59CE50);
	pInvListNum = (LONG*)FixAddress(0x59CEE8);
	F_DRAW_ITEM_LIST = (void*)FixAddress(0x463BB8);

	MemWrite16(0x462A6E, 0xFB81, 0xE890);
	FuncWrite32(0x462A70, 0x150, (DWORD)&inv_check_keys);

	MemWrite16(0x4653D3, 0xFB81, 0xE890);
	FuncWrite32(0x4653D5, 0x150, (DWORD)&inv_check_keys);


	if (ConfigReadInt("OTHER_SETTINGS", "INV_ADD_ITEMS_AT_TOP", 0)) {
		MemWrite8(0x46A3BB, 0x8B, 0xE8);
		FuncWrite32(0x46A3BC, 0x08518B01, (DWORD)&inv_insert_item);
		//0046A3C0     /EB 12         JMP SHORT 0046A3D4                       ; invItemList->invItem[invItemList->inv_size].obj = obj_item
		MemWrite16(0x46A3C0, 0x3C89, 0x12EB);
		MemWrite8(0x46A3C2, 0xC2, 0x90);
	}


	MemWrite16(0x462CFB, 0x968B, 0xE890);
	FuncWrite32(0x462CFD, FixAddress(0x505748), (DWORD)&mouseMaxInvX);

	MemWrite16(0x462D11, 0x968B, 0xE890);
	FuncWrite32(0x462D13, FixAddress(0x50574C), (DWORD)&mouseMaxInvY);


	//INVENTORY STUFF----------------------------------------------

	//MOUSE DROP BOXES
	//inv item list rect
 	FuncReplace32(0x464D01, 0x0509EB, (DWORD)&h_inv_mouse_rec);
	//fix for dropping stuff in bags
	FuncReplace32(0x464D16, 0x050A7E, (DWORD)&h_inv_mouse_rec_pos);

	//LEFT HAND
 	FuncReplace32(0x464E2B, 0x0508C1, (DWORD)&h_inv_mouse_rec);
	//RIGHT HAND
	FuncReplace32(0x464EB4, 0x050838, (DWORD)&h_inv_mouse_rec);
	//ARMOR
	FuncReplace32(0x464F3D, 0x0507AF, (DWORD)&h_inv_mouse_rec);
	//use pack on
 	//FuncReplace32(0x465009, 0x0506E3, (DWORD)&h_inv_mouse_rec);
	//CONTAINER RIGHT MOUSE
	FuncReplace32(0x467D6E, 0x04D97E, (DWORD)&h_inv_mouse_rec);
	//CONTAINER LEFT MOUSE
 	FuncReplace32(0x467E31, 0x04D8BB, (DWORD)&h_inv_mouse_rec);

	pRgt_Inv = (LONG*)FixAddress(0x464CEF);
	pTop_Inv = (LONG*)FixAddress(0x464CF7);
	pLft_Inv = (LONG*)FixAddress(0x464CFC);

	pRgtNpc_Loot = (LONG*)FixAddress(0x467D5D);
	pTopNpc_Loot = (LONG*)FixAddress(0x467D62);
	pLftNpc_Loot = (LONG*)FixAddress(0x467D69);

	pRgtPc_Loot = (LONG*)FixAddress(0x467E20);
	pTopPc_Loot = (LONG*)FixAddress(0x467E25);
	pLftPc_Loot = (LONG*)FixAddress(0x467E2C);

	FuncReplace32(0x462CF7, 0x05FB2D, (DWORD)&h_inventory);

	//DIALOG_BARTER_INV_SUB WIN
	FuncReplace32(0x462DB2, 0x05FA72, (DWORD)&h_dialog_sub_win_barter);

	FuncReplace32(0x463A75, 0x05EFDB, (DWORD)&h_inv_win_destroy);



	//DIALOG SCRN STUFF--------------------------------------

	//DIALOG_NPC_RESPONSE TEXT SCRN POSITION
 	FuncReplace32(0x43F504, 0x083320, (DWORD)&h_dialog_sub_win);
	//DIALOG_CHARACTOR_RESPONSE TEXT SCRN POSITION
	FuncReplace32(0x43F5DC, 0x083248, (DWORD)&h_dialog_sub_win);
	//TELL ME ABOUT WINDOW POSITION
	FuncReplace32(0x442653, 0x0801D1, (DWORD)&h_dialog_sub_win);

	//BAKGROUNG CAPTURE YPOS FOR DIALOG_CHARACTOR_RESPONSE TEXT SCRN
 	FuncReplace32(0x43FF19, 0x07E577, (DWORD)&h_dialog_pc_txt_bg);
	FuncReplace32(0x440061, 0x07E42F, (DWORD)&h_dialog_pc_txt_bg);

	//BARTER STUFF PANEL------
	//mouseMaxInvX & mouseMaxInvY handled in h_dialog_sub_win
	MemWrite8(0x462DB6, 0xBB, 0x90);
	MemWrite32(0x462DB7, 560, 0x90909090);
	MemWrite8(0x462DBB, 0xB9, 0x90);
	MemWrite32(0x462DBC, 470, 0x90909090);
	//barter NPC trade
	FuncReplace32(0x4681FB, 0x04D4F1, (DWORD)&h_dialog_mouse_rec);
	//barter PC trade
 	FuncReplace32(0x468274, 0x04D478, (DWORD)&h_dialog_mouse_rec);
	//barter NPC inv
	FuncReplace32(0x4684B0, 0x04D23C, (DWORD)&h_dialog_mouse_rec);
	//barter PC inv
	FuncReplace32(0x46852A, 0x04D1C2, (DWORD)&h_dialog_mouse_rec);


	pRgtPcTrade_Barter = (LONG*)FixAddress(0x4681EA);
	pTopPcTrade_Barter = (LONG*)FixAddress(0x4681EF);
	pLftPcTrade_Barter = (LONG*)FixAddress(0x4681F6);

	pRgtNpcTrade_Barter = (LONG*)FixAddress(0x468263);
	pTopNpcTrade_Barter = (LONG*)FixAddress(0x468268);
	pLftNpcTrade_Barter = (LONG*)FixAddress(0x46826F);

	pRgtPcInv_Barter = (LONG*)FixAddress(0x46849F);
	pTopPcInv_Barter = (LONG*)FixAddress(0x4684A4);
	pLftPcInv_Barter = (LONG*)FixAddress(0x4684AB);

	pRgtNpcInv_Barter = (LONG*)FixAddress(0x468519);
	pTopNpcInv_Barter = (LONG*)FixAddress(0x46851E);
	pLftNpcInv_Barter = (LONG*)FixAddress(0x468525);

	if (ConfigReadInt("OTHER_SETTINGS", "BARTER_PC_INV_DROP_FIX", 1)) {
		MemWrite32(0x46849F, 144, 164);
		MemWrite32(0x4684AB, 80, 100);
	}



	//DIALOG PANEL WIDTH FIXES--ORIGINALY SET TO GLOBAL SCRN SIZE-----------------------
	//7F02 = 639
	DWORD width_fix[44]{0};

	width_fix[0] = 0x44043F;
	width_fix[1] = 0x440471;
	width_fix[2] = 0x4404DE;
	width_fix[3] = 0x4404FA;
	width_fix[4] = 0x440694;
	width_fix[5] = 0x4406B0;
	width_fix[6] = 0x4409A7;
	width_fix[7] = 0x4409D1;
	width_fix[8] = 0x440F54;
	width_fix[9] = 0x440F86;
	width_fix[10] = 0x4411B4;
	width_fix[11] = 0x4415EC;
	width_fix[12] = 0x44161E;
	width_fix[13] = 0x44198A;
	width_fix[14] = 0x4419F8;
	width_fix[15] = 0x441A16;
	width_fix[16] = 0x441AF9;
	width_fix[17] = 0x441B40;
	width_fix[18] = 0x4421FB;
	width_fix[19] = 0x442348;
	width_fix[20] = 0x442389;
	width_fix[21] = 0x4423C9;
	width_fix[22] = 0x462DEC;
	width_fix[23] = 0x463E1D;
	width_fix[24] = 0x463E52;
	width_fix[25] = 0x464103;
	width_fix[26] = 0x464147;
	width_fix[27] = 0x4644C0;
	width_fix[28] = 0x466A54;
	width_fix[29] = 0x466AAF;
	width_fix[30] = 0x4680B9;
	width_fix[31] = 0x4680FA;
	width_fix[32] = 0x468370;
	width_fix[33] = 0x4683AA;
	width_fix[34] = 0x468606;
	width_fix[35] = 0x46863E;
	width_fix[36] = 0x468784;
	width_fix[37] = 0x4687CD;

	width_fix[38] = 0x4403B1;
	width_fix[39] = 0x4418E9;
	width_fix[40] = 0x44212F;
	width_fix[41] = 0x44223C;

	width_fix[42] = 0x4422B7;
	width_fix[43] = 0x44061E;

	for (i = 0; i < 44; i++)
		MemWrite32(width_fix[i], FixAddress(0x672198), (DWORD)&dialogWidthFix);

	FuncReplace32(0x441A88, 0x080D9C, (DWORD)&h_dialog_main_win);

	FuncReplace32(0x441BD1, 0x080E7F, (DWORD)&h_dialog_bg);

	//DIALOG_HISTORY
	FuncReplace32(0x44063C, 0x0821E8, (DWORD)&h_dialog_sub_win);
	//DIALOG_BARTER
	FuncReplace32(0x440F66, 0x0818BE, (DWORD)&h_dialog_sub_win);
	//DIALOG_TALK
	FuncReplace32(0x4415FE, 0x081226, (DWORD)&h_dialog_sub_win);

	invMouseEdgeRight = (LONG*)FixAddress(0x59CF24);
	invMouseEdgeBottom = (LONG*)FixAddress(0x59CF20);

	if (ConfigReadInt("OTHER_SETTINGS", "DIALOG_SCRN_ART_FIX", 1)) {

		FuncReplace32(0x441A26, 0x07CA6A, (DWORD)&DrawFixedDialogArt);

		MemWrite16(0x441B4D, 0x048D, 0xD269);
		MemWrite32(0x441B4F, 0xD5, 14 + 5);
		MemWrite8(0x441B53, 0x00, 0x90);
		MemWrite16(0x441B54, 0xD029, 0x03EB);


		//mask top right y and x
		MemWrite8(0x442351, 0x0F, 0x0F + 5);
		//mask bottom left ybottom and x
		MemWrite32(0x44239F, 0xD6, 0xD6 + 5);

		//draw_win_area rect
		MemWrite32(0x4422FB, 0x0E, 0x0E + 5);
		MemWrite32(0x442305, 0xD6, 0xD6 + 5);

		MemWrite32(0x50512C, 0x0E, 0x0E + 5);
		MemWrite32(0x505134, 0x28, 0x28 + 5);

		MemWrite32(0x50513C, 0x0E, 0x0E + 5);
		MemWrite32(0x505144, 0x28, 0x28 + 5);

		MemWrite32(0x50514C, 0xBC, 0xBC + 5);
		MemWrite32(0x505154, 0xD6, 0xD6 + 5);

		MemWrite32(0x50515C, 0xBC, 0xBC + 5);
		MemWrite32(0x505164, 0xD6, 0xD6 + 5);

		MemWrite32(0x50516C, 0x0E, 0x0E + 5);
		MemWrite32(0x505174, 0x18, 0x18 + 5);

		MemWrite32(0x50517C, 0xCC, 0xCC + 5);
		MemWrite32(0x505184, 0xD6, 0xD6 + 5);

		MemWrite32(0x50518C, 0x28, 0x28 + 5);
		MemWrite32(0x505194, 0xBC, 0xBC + 5);

		MemWrite32(0x50519C, 0x28, 0x28 + 5);
		MemWrite32(0x5051A4, 0xBC, 0xBC + 5);
	}
}
