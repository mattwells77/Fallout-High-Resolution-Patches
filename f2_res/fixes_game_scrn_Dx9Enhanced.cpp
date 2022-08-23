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
#include "WinFall.h"
#include "Dx9Enhanced.h"
#include "F_Mapper.h"
#include "F_Objects.h"
#include "F_File.h"
#include "F_Art.h"
#include "configTools.h"


RECT GAME_RECT;
DWORD GAME_WIDTH = 0;
DWORD GAME_HEIGHT = 0;


LONG* pfrmObjRef_Mouse;
void* pMouseMenuSingle = nullptr;
void* pMouseMenuMulti = nullptr;

bool isMapLevelChanged = false;
bool isZoomBoundByMapEdges = false;


//______________________
DWORD CheckWindowMouse() {

	DWORD flags = 0;

	if (!isGameMode) {
		ClipCursor(nullptr);
		return flags;
	}

	RECT rcClient;
	POINT p{ 0,0 }, m{ 0,0 };

	GetCursorPos(&m);

	ClientToScreen(hGameWnd, &p);
	GetClientRect(hGameWnd, &rcClient);
	rcClient.left += p.x;
	rcClient.top += p.y;
	rcClient.right += p.x;
	rcClient.bottom += p.y;

	ClipCursor(&rcClient);

	if (m.x <= rcClient.left + 1)
		flags = flags | 0x01;
	if (m.x >= rcClient.right - 1)
		flags = flags | 0x02;
	if (m.y <= rcClient.top + 1)
		flags = flags | 0x04;
	if (m.y >= rcClient.bottom - 1)
		flags = flags | 0x08;

	return flags;

}


//________________________________________________
DWORD CheckMouseScrnRect(LONG mouseX, LONG mouseY) {

	if (isWindowed || (isAltMouseInput && graphicsMode == 0))
		return CheckWindowMouse();

	DWORD flags = 0;
	if (xPosTrue <= SCRN_RECT->left)
		flags = flags | 0x01;
	if (xPosTrue >= SCRN_RECT->right)
		flags = flags | 0x02;
	if (yPosTrue <= SCRN_RECT->top)
		flags = flags | 0x04;
	if (yPosTrue >= SCRN_RECT->bottom)
		flags = flags | 0x08;
	return flags;
}


//____________________________________________
void __declspec(naked) check_mouse_edges(void) {

	//in check scroll direction func.
	//CX = bit1=left, bit2=right, bit3=top, bit4=bottom.
	//ebp == mouseX, esi == mouseY.
	__asm {
		add dword ptr ss : [esp] , 0x31
		push esi
		push ebp
		push esi
		push ebp
		call CheckMouseScrnRect
		add esp, 0x8
		pop ebp
		pop esi
		mov ecx, eax
		xor eax, eax
		xor edx, edx
		cmp ecx, 0
		je checkGameMouse
		ret
		checkGameMouse : //check if mouse pointer on game screen
		pushad
		push esi
		push ebp
		call F_GetWinAtPos
		add esp, 0x8
		mov edx, pWinRef_GameArea
		cmp eax, dword ptr ds : [edx]
		popad
		je checkEdges
		xor eax, eax
		xor edx, edx
		xor ebx, ebx
		ret
		checkEdges : // check if mouse lies within draw area of game win
		cmp ebp, edgeRect.left
		jl drawArrow
		cmp ebp, edgeRect.right
		jg drawArrow
		cmp esi, edgeRect.top
		jl drawArrow
		cmp esi, edgeRect.bottom
		jl exitFunc
		drawArrow :
		mov eax, 40000//x direction -1, 0 , 1// set x out of bounds to fail scroll but draw new mouse pointer.
		xor edx, edx//y direction -1, 0, 1
		mov ebx, -7//mouse pic num, 8 is added later making -7 == 1(default arrow).
		mov ecx, 3//set dirction flag to both left & right, forcing default case use.
		exitFunc :
		ret
	}
}


//______________________________________________________________________________________
LONG CreateGameWin(LONG x, LONG y, DWORD width, DWORD height, DWORD colour, DWORD flags) {

	if (ConfigReadInt("MAPS", "IS_ZOOM_BOUND_BY_EDGES", 0))
		isZoomBoundByMapEdges = true;

	LONG scaleGameInt = ConfigReadInt("MAPS", "ZOOM_LEVEL", 1);
	if (scaleGameInt < 1)
		scaleGameInt = 1;
	if (scaler) {
		scaleGameInt--;
		if (scaleGameInt < 1)
			ScaleX_game = 0.5f;
	}
	else
		ScaleX_game = (float)scaleGameInt;
	if ((float)SCR_WIDTH / ScaleX_game < 128.0f)
		ScaleX_game = (float)SCR_WIDTH / 128.0f;

	if (!scaler && ScaleX_game < 1.0f)
		ScaleX_game = 1.0f;

	rcGameTrue.left = 0;
	rcGameTrue.top = 0;
	rcGameTrue.right = SCR_WIDTH;
	rcGameTrue.bottom = SCR_HEIGHT;

	GAME_RECT.left = rcGameTrue.left;
	GAME_RECT.top = rcGameTrue.top;
	GAME_RECT.right = (LONG)((float)rcGameTrue.right / ScaleX_game);
	GAME_RECT.bottom = rcGameTrue.bottom;


	if (IFACE_BAR_MODE == 0 && isGameMode)
		rcGameTrue.bottom -= 99;

	GAME_RECT.bottom = (LONG)((float)rcGameTrue.bottom / ScaleX_game);

	GAME_WIDTH = GAME_RECT.right - GAME_RECT.left;
	GAME_HEIGHT = GAME_RECT.bottom - GAME_RECT.top;

	LONG winRef = Win_Create(GAME_RECT.left, GAME_RECT.top, GAME_WIDTH, GAME_HEIGHT, colour, flags);
	*pWinRef_GameArea = winRef;
	ResizeGameWin();
	return winRef;
}


//__________________________________________
void __declspec(naked) create_game_win(void) {
	__asm {
		push ebp
		push esi
		push edi

		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push ecx
		push ebx
		push edx
		push eax
		call CreateGameWin
		add esp, 0x18

		pop edi
		pop esi
		pop ebp
		ret 0x8
	}
}



//________________________
int CheckMouseInGameRect() {

	WinStruct* gameWin = GetWinStruct(*pWinRef_GameArea);
	if (IFACE_BAR_MODE == 1) {
		if (*pWinRef_Iface != -1) {
			WinStruct* ifaceWin = GetWinStruct(*pWinRef_Iface);
			return F_CheckMouseInRect(ifaceWin->rect.left, ifaceWin->rect.top, ifaceWin->rect.right, ifaceWin->rect.bottom);
		}
		else
			return F_CheckMouseInRect((SCR_WIDTH >> 1) - 320, SCR_HEIGHT - 100, (SCR_WIDTH >> 1) + 320, SCR_HEIGHT);
	}
	return F_CheckMouseInRect(rcGameTrue.left, rcGameTrue.top, rcGameTrue.right, rcGameTrue.bottom);
}


//___________________________________________________
void __declspec(naked) check_mouse_in_game_rect(void) {

	__asm {
		push ebp
		push esi
		call CheckMouseInGameRect
		cmp IFACE_BAR_MODE, 0
		je normalExit
		test eax, eax
		jne setZero
		mov eax, 1
		pop esi
		pop ebp
		ret
		setZero :
		xor eax, eax
		normalExit :
		pop esi
		pop ebp
		ret
	}
}


//__________________
bool ResizeGameWin() {

	if (isMapperSizing) {
		if (ScaleX_game < 1.0f) {
			if (scaler)
				ScaleX_game = 0.5f;
			else
				ScaleX_game = 1.0f;
		}
		else if ((float)SCR_WIDTH / ScaleX_game < 128.0f)
			ScaleX_game = (float)SCR_WIDTH / 128.0f;
	}

	rcGameTrue.left = 0;
	rcGameTrue.top = 0;
	rcGameTrue.right = SCR_WIDTH;
	rcGameTrue.bottom = SCR_HEIGHT;

	GAME_RECT.left = rcGameTrue.left;
	GAME_RECT.top = rcGameTrue.top;
	GAME_RECT.right = (LONG)((float)rcGameTrue.right / ScaleX_game);
	GAME_RECT.bottom = rcGameTrue.bottom;

	if (IFACE_BAR_MODE == 0 && isGameMode)
		rcGameTrue.bottom -= 99;

	GAME_RECT.bottom = (LONG)((float)rcGameTrue.bottom / ScaleX_game);


	GAME_WIDTH = GAME_RECT.right - GAME_RECT.left;
	GAME_HEIGHT = GAME_RECT.bottom - GAME_RECT.top;


	if (isZoomBoundByMapEdges && M_CURRENT_MAP.currentEDGES) {
		if ((LONG)GAME_WIDTH > M_CURRENT_MAP.currentEDGES->visEdge.left - M_CURRENT_MAP.currentEDGES->visEdge.right || (LONG)GAME_HEIGHT > M_CURRENT_MAP.currentEDGES->visEdge.bottom - M_CURRENT_MAP.currentEDGES->visEdge.top) {
			if (ScaleX_game >= 1.0f)
				ScaleX_game++;
			else if (scaler && ScaleX_game < 0.5f)
				ScaleX_game = 0.5f;
			else
				ScaleX_game = 1.0f;
			if (ResizeGameWin())
				return true;
			else
				return false;
		}
	}

	if (*pWinRef_GameArea == -1)
		return false;
	WinStruct* gameWin = GetWinStruct(*pWinRef_GameArea);
	if (gameWin == nullptr)
		return false;
	gameWin->width = GAME_WIDTH;
	gameWin->height = GAME_HEIGHT;
	gameWin->rect.left = 0;
	gameWin->rect.top = 0;
	gameWin->rect.right = GAME_WIDTH - 1;
	gameWin->rect.bottom = GAME_HEIGHT - 1;
	gameWin->buff = FReallocateMemory(gameWin->buff, GAME_WIDTH * GAME_HEIGHT);

	SetDxWin(gameWin);

	return true;
}


//___________________________________________
void __declspec(naked) fix_mouse_menu_y(void) {

	__asm {
		mov eax, GAME_RECT.bottom
		cmp eax, edgeRect.bottom
		jle exitFunc
		mov eax, edgeRect.bottom
		add eax, 1
		exitFunc:
		ret
	}
}


//___________________________________________
void __declspec(naked) fix_mouse_menu_x(void) {

	__asm {
		mov eax, GAME_RECT.right
		cmp eax, edgeRect.right
		jle exitFunc
		mov eax, edgeRect.right
		add eax, 1
		exitFunc:
		ret
	}
}


//__________________________________________________________
bool CheckMouseInGameRectScroll(int zDelta, bool scrollPage) {

	if (graphicsMode <= 1)
		return false;
	if (*pWinRef_GameArea == -1)
		return false;
	WinStruct* win = GetWinStruct(*pWinRef_GameArea);
	if (!win)
		return false;
	if (win->flags & F_WIN_HIDDEN)
		return false;

	if (pWinArray[*numWindows - 1]->ref != *pWinRef_GameArea &&
		pWinArray[*numWindows - 1]->ref != *pWinRef_Iface &&
		pWinArray[*numWindows - 1]->ref != wRef_ifaceLeft &&
		pWinArray[*numWindows - 1]->ref != wRef_ifaceRight &&
		pWinArray[*numWindows - 1]->ref != *pWinRef_NotifyBar)
		return false;

	LONG xPos = 0, yPos = 0;
	F_GetMousePos(&xPos, &yPos);

	if (F_GetWinAtPos(xPos, yPos) == *pWinRef_GameArea) {

		LONG xMouse = 0, yMouse = 0, hexMouse = 0;
		F_GetMousePos(&xMouse, &yMouse);

		hexMouse = F_GetScrnHexPos(xMouse, yMouse, 0);
		if (!isHexWithinMapEdges(hexMouse) && isGameMode)
			return true;

		SetViewPos(hexMouse, 0x2);

		if (zDelta > 0 && ScaleX_game < 1.0f) {
			if (scaler && ScaleX_game < 0.5f)
				ScaleX_game = 0.5f;
			else
				ScaleX_game = 1.0f;
		}
		else if (zDelta > 0 && (float)SCR_WIDTH / (ScaleX_game + 1.0f) >= 128.0f)
			ScaleX_game += 1.0f;
		else if (zDelta < 0 && ScaleX_game>1.0f)
			ScaleX_game -= 1.0f;
		else if (zDelta < 0 && ScaleX_game == 1.0f) {
			if (scaler)
				ScaleX_game = 0.5f;
			else {
				float visWidth = GAME_WIDTH / (float)(M_CURRENT_MAP.currentEDGES->visEdge.left - M_CURRENT_MAP.currentEDGES->visEdge.right);
				float visHeight = GAME_HEIGHT / (float)(M_CURRENT_MAP.currentEDGES->visEdge.bottom - M_CURRENT_MAP.currentEDGES->visEdge.top);
				if (visWidth < visHeight && visWidth < 1.0f)
					ScaleX_game = visWidth;
				else if (visHeight < visWidth && visHeight < 1.0f)
					ScaleX_game = visHeight;
			}
		}
		else if (zDelta < 0 && ScaleX_game == 0.5f && scaler) {
			float visWidth = GAME_WIDTH / (float)(M_CURRENT_MAP.currentEDGES->visEdge.left - M_CURRENT_MAP.currentEDGES->visEdge.right);
			float visHeight = GAME_HEIGHT / (float)(M_CURRENT_MAP.currentEDGES->visEdge.bottom - M_CURRENT_MAP.currentEDGES->visEdge.top);
			if (visWidth < visHeight && visWidth < 1.0f)
				ScaleX_game = visWidth / 2;
			else if (visHeight < visWidth && visHeight < 1.0f)
				ScaleX_game = visHeight / 2;
		}

		ResizeGameWin();
		ReSizeIface();
		ReSizeMaps();
		isFadeReset = true;

		SetViewPos(hexMouse, 0x3);

		LONG xView = 0, yView = 0;

		GetHexSqrXY(*pVIEW_HEXPOS, &xView, &yView);
		GetHexSqrXY(hexMouse, &xMouse, &yMouse);

		xMouse = (GAME_WIDTH >> 1) - ((xMouse - xView) << 4);
		yMouse = (GAME_HEIGHT >> 1) - ((yView - yMouse) * 12);

		xMouse += EDGE_OFF_X;
		yMouse += EDGE_OFF_Y;

		F_SetMousePos(xMouse, yMouse);
		return true;
	}
	return false;
}


//______________________
LONG ResetLevelScaling() {

	isMapLevelChanged = true;
	return F_GetMousePicNum();
}


//______________________________________________
void __declspec(naked) reset_level_scaling(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		call ResetLevelScaling
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}



//___________________
void ResetZoomLevel() {
	if (graphicsMode < 2)
		return;

	if (!isMapLevelChanged)
		return;
	isMapLevelChanged = false;

	if (isZoomBoundByMapEdges && M_CURRENT_MAP.currentEDGES) {
		if ((LONG)GAME_WIDTH > M_CURRENT_MAP.currentEDGES->visEdge.left - M_CURRENT_MAP.currentEDGES->visEdge.right || (LONG)GAME_HEIGHT > M_CURRENT_MAP.currentEDGES->visEdge.bottom - M_CURRENT_MAP.currentEDGES->visEdge.top) {
			if (ScaleX_game >= 1.0f)
				ScaleX_game++;
			else if (scaler && ScaleX_game < 0.5f)
				ScaleX_game = 0.5f;
			else
				ScaleX_game = 1.0f;
			ResizeGameWin();
			ReSizeIface();
			ReSizeMaps();
		}
	}
	else {
		if (ScaleX_game < 1.0f) {
			if (scaler && ScaleX_game == 0.5f)
				return;
			if (scaler)
				ScaleX_game = 0.5f;
			else
				ScaleX_game = 1.0f;
			ResizeGameWin();
			ReSizeIface();
			ReSizeMaps();
		}
	}
}


//____________________________________________________________
void SetGameMouseFID(OBJStruct* obj, DWORD frmID, RECT* rcOut) {

	if (frmID != 0x060000F9) {
		DWORD frmObj;
		FRMhead* frm = F_GetFrm(frmID, &frmObj);
		if (*pfrmObjRef_Mouse == frmObj) {
			F_UnloadFrm(frmObj);
			return;
		}
		BYTE* buff = F_GetFrmFrameBuff(frm, 0, 0);
		int width = F_GetFrmFrameWidth(frm, 0, 0);
		int height = F_GetFrmFrameHeight(frm, 0, 0);
		int xPos = width / 2 - 1 - frm->xCentreShift[0];
		int yPos = height - 1 - frm->yCentreShift[0];

		F_SetMousePic(buff, width, height, width, xPos, yPos, 0);
		F_Obj_SetFrmId(obj, 0x0600010A, rcOut);

		if (*pfrmObjRef_Mouse != -1)
			F_UnloadFrm(*pfrmObjRef_Mouse);
		*pfrmObjRef_Mouse = frmObj;
	}
	else {
		DWORD frmObj;
		FRMhead* frm = F_GetFrm(0x0600010A, &frmObj);
		if (*pfrmObjRef_Mouse == frmObj) {
			F_UnloadFrm(frmObj);
		}
		else {
			BYTE* buff = F_GetFrmFrameBuff(frm, 0, 0);
			int width = F_GetFrmFrameWidth(frm, 0, 0);
			int height = F_GetFrmFrameHeight(frm, 0, 0);
			F_SetMousePic(buff, width, height, width, 0, 0, 0);
			if (*pfrmObjRef_Mouse != -1)
				F_UnloadFrm(*pfrmObjRef_Mouse);
			*pfrmObjRef_Mouse = frmObj;
		}

		F_Obj_SetFrmId(obj, frmID, rcOut);
	}
}


//_____________________________________________
void __declspec(naked) set_game_mouse_fid(void) {

	__asm {
		pushad
		push ebx//*outRect
		push edx//FID
		push eax//*obj
		call SetGameMouseFID
		add esp, 0xC
		popad
		xor eax, eax
		ret
	}
}


//_____________________________________________
void __declspec(naked) hold_multi_menu_pos(void) {

	__asm {
		push edx
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		mov isMouseHeld, 1
		call GetMouseFlags

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		pop edx
		ret
	}
}


//_________________________________________________
void __declspec(naked) release_multi_menu_pos(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		mov isMouseHeld, 0
		push edx
		push eax
		call F_SetMousePos
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//__________________________________________
void GetMultiMenuPos(LONG* xPtr, LONG* yPtr) {

	F_GetMousePos(xPtr, yPtr);
}


//_____________________________________________
void __declspec(naked) get_multi_menu_pos(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call GetMultiMenuPos
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//____________________________________________
void __declspec(naked) mouse_menu_single(void) {

	__asm {
		push esi
		push edi
		push ebp

		mov esi, rcGameTrue.bottom
		add esi, 0x1
		push esi
		mov ecx, rcGameTrue.right
		mov edx, yPosTrue
		mov eax, xPosTrue
		call pMouseMenuSingle

		pop ebp
		pop edi
		pop esi
		ret 0x4
	}
}



//___________________________________________
void __declspec(naked) mouse_menu_multi(void) {

	__asm {
		push esi
		push edi
		push ebp

		mov esi, rcGameTrue.bottom
		add esi, 0x1
		push esi
		mov esi, rcGameTrue.right

		push esi
		mov edx, yPosTrue
		mov eax, xPosTrue
		call pMouseMenuMulti

		pop ebp
		pop edi
		pop esi
		ret 0x8
	}
}


//__________________________________________
void __declspec(naked) check_zoom_keys(void) {

	__asm {
		cmp ebx, 0x18D  //CTRL-Up
		je zoomIn
		cmp ebx, 0x191  //CTRL-Down
		je zoomOut
		jmp exitFunc
		zoomIn :
		mov eax, 1
		jmp zoom
		zoomOut :
		mov eax, -1
		zoom :
		pushad
		push 0
		push eax
		call CheckMouseInGameRectScroll//(int zDelta, bool scrollPage)
		add esp, 0x8
		popad
		exitFunc :
		cmp ebx, 0x150
		ret
	}
}


//_________________________
void GameScrnFixes_CH(void) {

	MemWrite32(0x4811CE, 0x6BCF64, (DWORD)&GAME_RECT.right);
	MemWrite8(0x4811DD, 0x1, 0x0);

	MemWrite32(0x4811E5, 0x6BCF68, (DWORD)&GAME_RECT.bottom);
	MemWrite16(0x4811F1, 0xE883, 0x9090);
	MemWrite8(0x4811F3, 0x63, 0x90);

	MemWrite32(0x48123F, 0x6BCF64, (DWORD)&GAME_RECT.right);
	MemWrite8(0x48124B, 0x1, 0x0);

	MemWrite32(0x481253, 0x6BCF68, (DWORD)&GAME_RECT.bottom);
	MemWrite8(0x48125E, 0x9D, 0x0);

	MemWrite32(0x481BBD, 0x6BCF64, (DWORD)&GAME_RECT.right);
	MemWrite8(0x481BC9, 0x1, 0x0);

	MemWrite32(0x481BD1, 0x6BCF68, (DWORD)&GAME_RECT.bottom);
	MemWrite16(0x481BD7, 0xE883, 0x9090);
	MemWrite8(0x481BD9, 0x63, 0x90);

	MemWrite32(0x48201D, 0x6BCF68, (DWORD)&GAME_RECT.bottom);
	MemWrite16(0x482023, 0xE883, 0x9090);
	MemWrite8(0x482025, 0x63, 0x90);

	MemWrite32(0x48202E, 0x6BCF64, (DWORD)&GAME_RECT.right);
	MemWrite8(0x482038, 0x1, 0x0);

	MemWrite32(0x481C77, 0x6BCF68, (DWORD)&GAME_RECT.bottom);
	MemWrite8(0x481C7F, 0x64, 0x1);



	MemWrite32(0x481C18, 0x6BCF64, (DWORD)&GAME_RECT.right);
	MemWrite8(0x481C22, 0x40, 0x90);

	MemWrite32(0x481C54, 0x6BCF64, (DWORD)&GAME_RECT.right);
	MemWrite8(0x481C5A, 0x40, 0x90);

	MemWrite32(0x481CA6, 0x6BCF64, (DWORD)&GAME_RECT.right);
	MemWrite8(0x481CAC, 0x40, 0x90);

	//Mouse single choice pointer area show------
	MemWrite8(0x44B190, 0xA1, 0xE8);
	FuncWrite32(0x44B191, 0x6BCF68, (DWORD)&fix_mouse_menu_y);
	MemWrite16(0x44B19B, 0xE883, 0x9090);
	MemWrite8(0x44B19D, 0x63, 0x90);

	MemWrite8(0x44B1A5, 0xA1, 0xE8);
	FuncWrite32(0x44B1A6, 0x6BCF64, (DWORD)&fix_mouse_menu_x);
	MemWrite8(0x44B1B5, 0x1, 0x0);

	//Mouse menu choice pointer area show------
	MemWrite8(0x44BCE7, 0xA1, 0xE8);
	FuncWrite32(0x44BCE8, 0x6BCF68, (DWORD)&fix_mouse_menu_y);
	MemWrite16(0x44BCF2, 0xE883, 0x9090);
	MemWrite8(0x44BCF4, 0x63, 0x90);

	MemWrite8(0x44BCFC, 0xA1, 0xE8);
	FuncWrite32(0x44BCFD, 0x6BCF64, (DWORD)&fix_mouse_menu_x);
	MemWrite8(0x44BD07, 0x40, 0x90);

	FuncWrite32(0x44B769, 0x080727, (DWORD)&check_mouse_in_game_rect);

	FuncWrite32(0x48112C, 0x05AF8D, (DWORD)&create_game_win);

	if (graphicsMode > 1) {
		FuncReplace32(0x4815A2, 0xFFFCAB92, (DWORD)&reset_level_scaling);

		MemWrite32(0x489CDE, 0x6BCF5C, (DWORD)&GAME_RECT);

		FuncReplace32(0x44B1EB, 0x03EC4D, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44B39C, 0x03EA9C, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44BD43, 0x03E0F5, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44C1D4, 0x03DC64, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44D67B, 0x03C7BD, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44D6C5, 0x03C773, (DWORD)&set_game_mouse_fid);

		FuncReplace32(0x44BD86, 0x08039A, (DWORD)&hold_multi_menu_pos);

		FuncReplace32(0x44BE16, 0x0801FD, (DWORD)&release_multi_menu_pos);

		FuncReplace32(0x44BDAA, 0x080224, (DWORD)&get_multi_menu_pos);

		FuncReplace32(0x44B1C0, 0x152C, (DWORD)&mouse_menu_single);

		FuncReplace32(0x44BD14, 0x0C4C, (DWORD)&mouse_menu_multi);

		pfrmObjRef_Mouse = (LONG*)FixAddress(0x528A00);

		pMouseMenuSingle = (void*)FixAddress(0x44C6F0);
		pMouseMenuMulti = (void*)FixAddress(0x44C964);

		MemWrite16(0x4426DA, 0xFB81, 0xE890);
		FuncWrite32(0x4426DC, 0x150, (DWORD)&check_zoom_keys);
	}

	MemWrite16(0x44DB9F, 0xD231, 0xE890);
	FuncWrite32(0x44DBA1, 0xC031C931, (DWORD)&check_mouse_edges);
}

//____________________________
void GameScrnFixes_MULTI(void) {

	MemWrite32(0x481D9E, FixAddress(0x6AC9F8), (DWORD)&GAME_RECT.right);
	MemWrite8(0x481DAD, 0x1, 0x0);

	MemWrite32(0x481DB5, FixAddress(0x6AC9FC), (DWORD)&GAME_RECT.bottom);
	MemWrite16(0x481DC1, 0xE883, 0x9090);
	MemWrite8(0x481DC3, 0x63, 0x90);

	MemWrite32(0x481E0F, FixAddress(0x6AC9F8), (DWORD)&GAME_RECT.right);
	MemWrite8(0x481E1B, 0x1, 0x0);

	MemWrite32(0x481E23, FixAddress(0x6AC9FC), (DWORD)&GAME_RECT.bottom);
	MemWrite8(0x481E2E, 0x9D, 0x0);

	MemWrite32(0x48278D, FixAddress(0x6AC9F8), (DWORD)&GAME_RECT.right);
	MemWrite8(0x482799, 0x1, 0x0);

	MemWrite32(0x4827A1, FixAddress(0x6AC9FC), (DWORD)&GAME_RECT.bottom);
	MemWrite16(0x4827A7, 0xE883, 0x9090);
	MemWrite8(0x4827A9, 0x63, 0x90);

	MemWrite32(0x482BED, FixAddress(0x6AC9FC), (DWORD)&GAME_RECT.bottom);
	MemWrite16(0x482BF3, 0xE883, 0x9090);
	MemWrite8(0x482BF5, 0x63, 0x90);

	MemWrite32(0x482BFE, FixAddress(0x6AC9F8), (DWORD)&GAME_RECT.right);
	MemWrite8(0x482C08, 0x1, 0x0);

	MemWrite32(0x482847, FixAddress(0x6AC9FC), (DWORD)&GAME_RECT.bottom);
	MemWrite8(0x48284F, 0x64, 0x1);

	MemWrite32(0x4827E8, FixAddress(0x6AC9F8), (DWORD)&GAME_RECT.right);
	MemWrite8(0x4827F2, 0x40, 0x90);

	MemWrite32(0x482824, FixAddress(0x6AC9F8), (DWORD)&GAME_RECT.right);
	MemWrite8(0x48282A, 0x40, 0x90);

	MemWrite32(0x482876, FixAddress(0x6AC9F8), (DWORD)&GAME_RECT.right);
	MemWrite8(0x48287C, 0x40, 0x90);

	//Mouse single choice pointer area show------
	MemWrite8(0x44BA40, 0xA1, 0xE8);
	FuncWrite32(0x44BA41, FixAddress(0x6AC9FC), (DWORD)&fix_mouse_menu_y);
	MemWrite16(0x44BA4B, 0xE883, 0x9090);
	MemWrite8(0x44BA4D, 0x63, 0x90);

	MemWrite8(0x44BA55, 0xA1, 0xE8);
	FuncWrite32(0x44BA56, FixAddress(0x6AC9F8), (DWORD)&fix_mouse_menu_x);
	MemWrite8(0x44BA65, 0x1, 0x0);

	//Mouse menu choice pointer area show------
	MemWrite8(0x44C597, 0xA1, 0xE8);
	FuncWrite32(0x44C598, FixAddress(0x6AC9FC), (DWORD)&fix_mouse_menu_y);
	MemWrite16(0x44C5A2, 0xE883, 0x9090);
	MemWrite8(0x44C5A4, 0x63, 0x90);

	MemWrite8(0x44C5AC, 0xA1, 0xE8);
	FuncWrite32(0x44C5AD, FixAddress(0x6AC9F8), (DWORD)&fix_mouse_menu_x);
	MemWrite8(0x44C5B7, 0x40, 0x90);

	FuncReplace32(0x44C019, 0x07E917, (DWORD)&check_mouse_in_game_rect);

	FuncReplace32(0x481CFC, 0x054538, (DWORD)&create_game_win);

	if (graphicsMode > 1) {
		FuncReplace32(0x482172, 0xFFFCA872, (DWORD)&reset_level_scaling);

		MemWrite32(0x48A8DE, FixAddress(0x6AC9F0), (DWORD)&GAME_RECT);

		FuncReplace32(0x44BA9B, 0x03EF9D, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44BC4C, 0x03EDEC, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44C5F3, 0x03E445, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44CA84, 0x03DFB4, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44DF2B, 0x03CB0D, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x44DF75, 0x03CAC3, (DWORD)&set_game_mouse_fid);

		FuncReplace32(0x44C636, 0x07E466, (DWORD)&hold_multi_menu_pos);

		FuncReplace32(0x44C6C6, 0x07E33A, (DWORD)&release_multi_menu_pos);

		FuncReplace32(0x44C65A, 0x07E37E, (DWORD)&get_multi_menu_pos);

		FuncReplace32(0x44BA70, 0x152C, (DWORD)&mouse_menu_single);

		FuncReplace32(0x44C5C4, 0x0C4C, (DWORD)&mouse_menu_multi);

		pfrmObjRef_Mouse = (LONG*)FixAddress(0x518C10);

		pMouseMenuSingle = (void*)FixAddress(0x44CFA0);
		pMouseMenuMulti = (void*)FixAddress(0x44D214);

		MemWrite16(0x442E6A, 0xFB81, 0xE890);
		FuncWrite32(0x442E6C, 0x150, (DWORD)&check_zoom_keys);
	}

	MemWrite16(0x44E44F, 0xD231, 0xE890);
	FuncWrite32(0x44E451, 0xC031C931, (DWORD)&check_mouse_edges);
}


//______________________________
void GameScrnFixes(DWORD region) {

	if (region == 4)
		GameScrnFixes_CH();
	else
		GameScrnFixes_MULTI();
}
