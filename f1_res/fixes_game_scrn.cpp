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


RECT GAME_RECT;
DWORD GAME_WIDTH = 0;
DWORD GAME_HEIGHT = 0;


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
	if (mouseX <= SCRN_RECT->left)
		flags = flags | 0x01;
	if (mouseX >= SCRN_RECT->right)
		flags = flags | 0x02;
	if (mouseY <= SCRN_RECT->top)
		flags = flags | 0x04;
	if (mouseY >= SCRN_RECT->bottom)
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


//____________________________________________________
int _stdcall h_view_area(DWORD colour, DWORD winflags) {

	GAME_RECT.left = 0;
	GAME_RECT.top = 0;
	GAME_RECT.right = SCR_WIDTH;
	GAME_RECT.bottom = SCR_HEIGHT;

	if (IFACE_BAR_MODE == 0 && isGameMode)
		GAME_RECT.bottom -= 99;

	GAME_WIDTH = GAME_RECT.right - GAME_RECT.left;
	GAME_HEIGHT = GAME_RECT.bottom - GAME_RECT.top;

	return Win_Create(GAME_RECT.left, GAME_RECT.top, GAME_RECT.right, GAME_RECT.bottom, colour, winflags);
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
	return F_CheckMouseInRect(gameWin->rect.left, gameWin->rect.top, gameWin->rect.right, gameWin->rect.bottom);

}


//_______________________________________
void __declspec(naked) h_mouse_area(void) {

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

	GAME_RECT.left = 0;
	GAME_RECT.top = 0;
	GAME_RECT.right = SCR_WIDTH;
	GAME_RECT.bottom = SCR_HEIGHT;


	if (IFACE_BAR_MODE == 0 && isGameMode)
		GAME_RECT.bottom -= 99;

	GAME_WIDTH = GAME_RECT.right - GAME_RECT.left;
	GAME_HEIGHT = GAME_RECT.bottom - GAME_RECT.top;

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


//__________________
void GameScrnFixes() {

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

	FuncReplace32(0x443F71, 0x07177B, (DWORD)&h_mouse_area);

	FuncReplace32(0x473D84, 0x04EAA0, (DWORD)&h_view_area);

	MemWrite16(0x44633B, 0xC031, 0xE890);
	FuncWrite32(0x44633D, 0xC931D231, (DWORD)&check_mouse_edges);
}
