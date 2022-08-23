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


//___________________________________________________________________________
int WorldMapWinSetup(DWORD width, DWORD height, DWORD colour, DWORD winFlags) {

	HideWin(*pWinRef_GameArea);
	HideIfaceWindows();

	LONG x = 0, y = 0;
	AdjustWinPosToGame(width, height, &x, &y);
	return Win_Create(x, y, width, height, colour, winFlags);
}


//_________________________________________
void __declspec(naked) worldmap_win_setup() {

	__asm {
		push esi
		push edi
		push ebp

		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push ecx
		push ebx
		call WorldMapWinSetup
		add esp, 0x10

		pop ebp
		pop edi
		pop esi
		ret 0x8
	}
}


//_________________________________________
void __declspec(naked) world_map_kill(void) {

	__asm {
		pushad

		push eax
		call DestroyWin
		add esp, 0x4
		mov eax, pWinRef_GameArea
		push dword ptr ds : [eax]
		call ShowWin
		//add esp, 0x4
		//mov eax, pWinRef_GameArea
		//push dword ptr ds:[eax]
		//call RedrawWin
		//add esp, 0x4
		call ShowIfaceWindows
		//mov eax, pWinRef_WorldMap
		//mov dword ptr ds:[eax], -1//worldMapWinRef
		///xor dl, dl

		popad
		ret
	}
}


//__________________________________________
void GetWorldMouse(LONG* pxPos, LONG* pyPos) {

	WinStruct* wWin = GetWinStruct(*pWinRef_WorldMap);

	if (isWindowed) {
		POINT p{ 0,0 }, m{ 0,0 };

		ClientToScreen(hGameWnd, &p);
		GetCursorPos(&m);
		m.x -= p.x;
		m.y -= p.y;
		m.x -= wWin->rect.left << scaler;
		m.y -= wWin->rect.top << scaler;

		RECT rcClient;
		GetClientRect(hGameWnd, &rcClient);
		if (m.x < rcClient.left)
			m.x = rcClient.left;
		else if (m.x > rcClient.right)
			m.x = rcClient.right;
		if (m.y < rcClient.top)
			m.y = rcClient.top;
		else if (m.y > rcClient.bottom)
			m.y = rcClient.bottom;
		*pxPos = m.x >> scaler;
		*pyPos = m.y >> scaler;
	}
	else {
		F_GetMousePos(pxPos, pyPos);
		*pxPos -= wWin->rect.left;
		*pyPos -= wWin->rect.top;
	}
}


//__________________________________________
void __declspec(naked) get_world_mouse(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi

		push edx
		push eax
		call GetWorldMouse
		add esp, 0x8

		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}

}



//__________________
void WorldMapFixes() {

	//world map win
	FuncReplace32(0x4AD574, 0x0152B0, (DWORD)&worldmap_win_setup);

	//town world win
	FuncReplace32(0x4AE3F4, 0x014430, (DWORD)&worldmap_win_setup);

	FuncReplace32(0x4AE9E6, 0x01406A, (DWORD)&world_map_kill);

	FuncReplace32(0x4AAD0B, 0xAA89, (DWORD)&get_world_mouse);
}




