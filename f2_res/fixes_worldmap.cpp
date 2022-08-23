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


//____________________________________________________________________________
LONG WorldMapWinSetup(DWORD width, DWORD height, DWORD colour, DWORD winFlags) {

   HideWin(*pWinRef_GameArea);
   HideIfaceWindows();

   LONG x=0, y=0;
   AdjustWinPosToGame(width, height, &x, &y);
   return Win_Create(x, y, width, height, colour, winFlags);
}


//_________________________________________
void __declspec(naked) worldmap_win_setup() {

   __asm {
	 push dword ptr ss:[esp+0x8]
	 push dword ptr ss:[esp+0x8]
     push ecx
     push ebx
     call WorldMapWinSetup
     add esp, 0x10
     ret 0x8
   }
}


//_________________________________________
void __declspec(naked) world_map_kill(void) {

   __asm {
      push eax
      call DestroyWin
      add esp, 0x4
      mov eax, pWinRef_GameArea
      push dword ptr ds:[eax]
      call ShowWin
      //add esp, 0x4
      //mov eax, pWinRef_GameArea
      //push dword ptr ds:[eax]
      //call RedrawWin
      //add esp, 0x4
      call ShowIfaceWindows
      mov eax, pWinRef_WorldMap
      mov dword ptr ds:[eax], -1
      ret
   }
}


//_______________________________________________________________________
DWORD CheckMouseInWorldRect(LONG left, LONG top, LONG right, LONG bottom) {

   WinStruct *worldWin = GetWinStruct(*pWinRef_WorldMap);
   return F_CheckMouseInRect(worldWin->rect.left+left, worldWin->rect.top+top, worldWin->rect.left+right, worldWin->rect.top+bottom);
}

//____________________________________________________
void __declspec(naked) check_mouse_in_world_rect(void) {

   __asm {
      push esi
      push edi
      push ebp

      push ecx
      push ebx
      push edx
      push eax
      call CheckMouseInWorldRect
      add esp, 0x10

      pop ebp
      pop edi
      pop esi
      ret
   }
}


//PREVENTS DRAWING OF BLACK WORLDMAP RECTANGLE OVER HAKUNIN MOVIES
//____________________________________________
void __declspec(naked) h_world_movie_fix(void) {

//window is hidden when showing movie
   __asm {
      mov esi, pWinRef_WorldMap
      mov esi, dword ptr ds:[esi]
      mov edx, 0x0B
      ret
   }

}


//__________________________________________
void GetWorldMouse(LONG *pxPos, LONG *pyPos) {

   WinStruct *wWin= GetWinStruct(*pWinRef_WorldMap);

   if(isWindowed) {
      POINT p{ 0,0 }, m{ 0,0 };

      ClientToScreen(hGameWnd, &p);
      GetCursorPos(&m);
      m.x-=p.x;
      m.y-=p.y;
      m.x-=wWin->rect.left<<scaler;
      m.y-=wWin->rect.top<<scaler;

      RECT rcClient;
      GetClientRect(hGameWnd, &rcClient);
      if(m.x < rcClient.left)
         m.x = rcClient.left;
      else if(m.x > rcClient.right)
         m.x = rcClient.right;
      if(m.y < rcClient.top)
         m.y = rcClient.top;
      else if(m.y > rcClient.bottom)
         m.y = rcClient.bottom;
      *pxPos = m.x>>scaler;
      *pyPos = m.y>>scaler;
   }
   else {
      F_GetMousePos(pxPos, pyPos);
      *pxPos-=wWin->rect.left;
      *pyPos-=wWin->rect.top;
   }
}


//__________________________________________
void __declspec(naked) get_world_mouse(void) {

   __asm {
      push ebx
      push ecx
      push esi

      push edx
      push eax
      call GetWorldMouse
      add esp, 0x8

      pop esi
      pop ecx
      pop ebx
      ret
   }
}


//_________________________
void WorldMapFixes_CH(void) {

   //if mouseX > world.left - don't scroll
   MemWrite8(0x4C1FC6, 0x75, 0x7F);//jg
   //if mouseX < world.right - don't scroll
   MemWrite8(0x4C1FD5, 0x75, 0x7C);//jl
   //if mouseY > world.top - don't scroll
   MemWrite8(0x4C1FE1, 0x75, 0x7F);//jg
   //if mouseY < world.bottom - don't scroll
   MemWrite8(0x4C1FEF, 0x75, 0x7C);//jl

   FuncWrite32(0x4BEE18, 0xD078,  (DWORD)&check_mouse_in_world_rect);

   FuncWrite32(0x4BEF7E, 0xCF12,  (DWORD)&check_mouse_in_world_rect);

   FuncWrite32(0x4C105B, 0x01B05E,  (DWORD)&worldmap_win_setup);

   FuncWrite32(0x4C1B3B, 0x01A881,  (DWORD)&world_map_kill);

   MemWrite8(0x4A239C, 0xBA, 0xE8);
   FuncWrite32(0x4A239D, 0x0B, (DWORD)&h_world_movie_fix);

   FuncWrite32(0x4C1FBA, 0xA014,  (DWORD)&get_world_mouse);
   FuncWrite32(0x4BEB26, 0xD4A8,  (DWORD)&get_world_mouse);
}


//____________________________
void WorldMapFixes_MULTI(void) {

   //if mouseX > world.left - don't scroll
   MemWrite8(0x4C3312, 0x75, 0x7F);//jg
   //if mouseX < world.right - don't scroll
   MemWrite8(0x4C3321, 0x75, 0x7C);//jl
   //if mouseY > world.top - don't scroll
   MemWrite8(0x4C332D, 0x75, 0x7F);//jg
   //if mouseY < world.bottom - don't scroll
   MemWrite8(0x4C333B, 0x75, 0x7C);//jl

   FuncReplace32(0x4C0168, 0xA7C8,  (DWORD)&check_mouse_in_world_rect);

   FuncReplace32(0x4C02CE, 0xA662,  (DWORD)&check_mouse_in_world_rect);

   FuncReplace32(0x4C23A8, 0x013E8C,  (DWORD)&worldmap_win_setup);

   FuncReplace32(0x4C2E87, 0x0135DD,  (DWORD)&world_map_kill);

   MemWrite8(0x4A369C, 0xBA, 0xE8);
   FuncWrite32(0x4A369D, 0x0B, (DWORD)&h_world_movie_fix);

   FuncReplace32(0x4C3306, 0x76D2,  (DWORD)&get_world_mouse);
   FuncReplace32(0x4BFE76, 0xAB62,  (DWORD)&get_world_mouse);
}


//______________________________
void WorldMapFixes(DWORD region) {

   if(region==4)
      WorldMapFixes_CH();
   else
      WorldMapFixes_MULTI();
}
