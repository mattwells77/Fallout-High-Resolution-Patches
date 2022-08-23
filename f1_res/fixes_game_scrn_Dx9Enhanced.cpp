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


LONG* pfrmObjRef_Mouse = nullptr;
void* pMouseMenuSingle = nullptr;
void* pMouseMenuMulti = nullptr;

bool isMapLevelChanged = false;
bool isZoomBoundByMapEdges = false;


//______________________
DWORD CheckWindowMouse() {

	DWORD flags = 0;

	if (!isGameMode)
		return flags;

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
	GAME_RECT.bottom = rcGameTrue.bottom;///ScaleX_game;

	if (IFACE_BAR_MODE == 0 && isGameMode)
		rcGameTrue.bottom -= 99;///ScaleX_game;

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
		if (!isHexWithinMapEdges(hexMouse))
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
					ScaleX_game = visWidth;  //
				else if (visHeight < visWidth && visHeight < 1.0f)
					ScaleX_game = visHeight;  //
			}
		}
		else if (zDelta < 0 && ScaleX_game == 0.5f && scaler) {
			float visWidth = GAME_WIDTH / (float)(M_CURRENT_MAP.currentEDGES->visEdge.left - M_CURRENT_MAP.currentEDGES->visEdge.right);
			float visHeight = GAME_HEIGHT / (float)(M_CURRENT_MAP.currentEDGES->visEdge.bottom - M_CURRENT_MAP.currentEDGES->visEdge.top);
			if (visWidth < visHeight && visWidth < 1.0f)
				ScaleX_game = visWidth / 2;  //
			else if (visHeight < visWidth && visHeight < 1.0f)
				ScaleX_game = visHeight / 2;  //
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
	return SetMouseImage(0);
}


//______________________________________________
void __declspec(naked) reset_level_scaling(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		call ResetLevelScaling
		pop ebp
		pop edi
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


//_____________________________________________________________
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


//______________________________________________
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
		//add esi, 0x1
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
			//mov ebx, -1
			exitFunc :
		cmp ebx, 0x150
			ret
	}
}


//______________________
void GameScrnFixes(void) {

	MemWrite32(0x473E26, FixAddress(0x672198), (DWORD)&GAME_RECT.right);
	MemWrite8(0x473E35, 0x1, 0x0);

	MemWrite32(0x473E3D, FixAddress(0x67219C), (DWORD)&GAME_RECT.bottom);
	MemWrite16(0x473E49, 0xE883, 0x9090);
	MemWrite8(0x473E4B, 0x63, 0x90);

	MemWrite32(0x473E97, FixAddress(0x672198), (DWORD)&GAME_RECT.right);
	MemWrite8(0x473EA3, 0x1, 0x0);

	MemWrite32(0x473EAB, FixAddress(0x67219C), (DWORD)&GAME_RECT.bottom);
	MemWrite8(0x473EB6, 0x9D, 0x0);

	MemWrite32(0x474791, FixAddress(0x672198), (DWORD)&GAME_RECT.right);
	MemWrite8(0x47479D, 0x1, 0x0);

	MemWrite32(0x4747A5, FixAddress(0x67219C), (DWORD)&GAME_RECT.bottom);
	MemWrite16(0x4747AB, 0xE883, 0x9090);
	MemWrite8(0x4747AD, 0x63, 0x90);

	MemWrite32(0x474847, FixAddress(0x67219C), (DWORD)&GAME_RECT.bottom);
	MemWrite8(0x47484F, 0x64, 0x1);

	MemWrite32(0x4747E6, FixAddress(0x672198), (DWORD)&GAME_RECT.right);
	MemWrite8(0x4747F4, 0x40, 0x90);

	MemWrite32(0x474820, FixAddress(0x672198), (DWORD)&GAME_RECT.right);
	MemWrite8(0x47482A, 0x40, 0x90);

	MemWrite32(0x474870, FixAddress(0x672198), (DWORD)&GAME_RECT.right);
	MemWrite8(0x47487A, 0x40, 0x90);


	//Mouse single choice pointer area show------
	MemWrite8(0x443A70, 0xA1, 0xE8);
	FuncWrite32(0x443A71, FixAddress(0x67219C), (DWORD)&fix_mouse_menu_y);
	MemWrite16(0x443A7B, 0xE883, 0x9090);
	MemWrite8(0x443A7D, 0x63, 0x90);

	MemWrite8(0x443A85, 0xA1, 0xE8);
	FuncWrite32(0x443A86, FixAddress(0x672198), (DWORD)&fix_mouse_menu_x);
	MemWrite8(0x443A95, 0x1, 0x0);

	//Mouse menu choice pointer area show------
	MemWrite8(0x4444A1, 0xA1, 0xE8);
	FuncWrite32(0x4444A2, FixAddress(0x67219C), (DWORD)&fix_mouse_menu_y);
	MemWrite16(0x4444AC, 0xE883, 0x9090);
	MemWrite8(0x4444AE, 0x63, 0x90);

	MemWrite8(0x4444B6, 0xA1, 0xE8);
	FuncWrite32(0x4444B7, FixAddress(0x672198), (DWORD)&fix_mouse_menu_x);
	MemWrite8(0x4444C1, 0x40, 0x90);

	FuncReplace32(0x443F71, 0x07177B, (DWORD)&check_mouse_in_game_rect);

	FuncReplace32(0x473D84, 0x04EAA0, (DWORD)&create_game_win);



	if (graphicsMode > 1) {

		FuncReplace32(0x4741B1, 0xFFFD057F, (DWORD)&reset_level_scaling);

		MemWrite32(0x47C956, FixAddress(0x672190), (DWORD)&GAME_RECT);

		FuncReplace32(0x443ACB, 0x038FE5, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x443BA9, 0x038F07, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x4444FD, 0x0385B3, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x444978, 0x038138, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x445E17, 0x036C99, (DWORD)&set_game_mouse_fid);
		FuncReplace32(0x445E61, 0x036C4F, (DWORD)&set_game_mouse_fid);

		FuncReplace32(0x444540, 0x071314, (DWORD)&hold_multi_menu_pos);

		FuncReplace32(0x4445D1, 0x0711E7, (DWORD)&release_multi_menu_pos);

		FuncReplace32(0x444564, 0x071230, (DWORD)&get_multi_menu_pos);

		FuncReplace32(0x443AA0, 0x13F0, (DWORD)&mouse_menu_single);

		FuncReplace32(0x4444CE, 0x0C36, (DWORD)&mouse_menu_multi);

		pfrmObjRef_Mouse = (LONG*)FixAddress(0x505364);

		pMouseMenuSingle = (void*)FixAddress(0x444E94);

		pMouseMenuMulti = (void*)FixAddress(0x445108);

		MemWrite16(0x43BBA2, 0xFB81, 0xE890);
		FuncWrite32(0x43BBA4, 0x150, (DWORD)&check_zoom_keys);
	}
	MemWrite16(0x44633B, 0xC031, 0xE890);
	FuncWrite32(0x44633D, 0xC931D231, (DWORD)&check_mouse_edges);
}
