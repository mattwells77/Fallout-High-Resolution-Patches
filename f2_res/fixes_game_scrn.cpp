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

	FuncWrite32(0x44B769, 0x080727, (DWORD)&h_mouse_area);

	FuncWrite32(0x48112C, 0x05AF8D, (DWORD)&h_view_area);

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

	FuncReplace32(0x44C019, 0x07E917, (DWORD)&h_mouse_area);

	FuncReplace32(0x481CFC, 0x054538, (DWORD)&h_view_area);

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
