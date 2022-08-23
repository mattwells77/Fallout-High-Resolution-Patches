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
#include "convert.h"

#include "F_File.h"
#include "F_Art.h"
#include "F_Objects.h"
#include "F_Mapper.h"

#include "configTools.h"

#include "WinFall.h"
#include "Graphics.h"


LONG numPathNodes = 2000;
DWORD* PathFindNodes01 = nullptr;
DWORD* PathFindNodes02 = nullptr;


//___________________________________________
void __declspec(naked) get_next_hex_pos(void) {

	__asm {
		push ecx
		push esi
		push edi
		push ebp

		push ebx
		push edx
		push eax
		call GetNextHexPos
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop ecx
		ret
	}
}


//_______________________________________
void __declspec(naked) get_hex_dist(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call GetHexDistance
		add esp, 0x8


		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}




//____________________________________________________________________________
void __declspec(naked) fog_of_war_move_to_slot(char* pFromPath, char* pToPath) {

	__asm {
		push eax

		push ebx
		push esi
		push edi
		push ebp

		push dword ptr ss : [esp + 0x1C] //pToPath
		push dword ptr ss : [esp + 0x1C] //pFromPath
		call FogOfWarMap_CopyFiles
		add esp, 0x08

		pop ebp
		pop edi
		pop esi
		pop ebx

		pop eax
		cmp eax, -1
		ret 0x8
	}
}


//______________________________________________________________________________
void __declspec(naked) fog_of_war_move_from_slot(char* pFromPath, char* pToPath) {

	__asm {
		push eax

		push ebx
		push esi
		push edi
		push ebp

		push dword ptr ss : [esp + 0x1C] //pToPath
		push dword ptr ss : [esp + 0x1C] //pFromPath
		call FogOfWarMap_CopyFiles
		add esp, 0x08

		pop ebp
		pop edi
		pop esi
		pop ebx

		pop eax
		cmp eax, -1
		ret 0x8
	}
}


//_________________________________________________
void __declspec(naked) fog_of_war_delete_tmps(void) {

	__asm {
		push ebx
		push ecx
		push esi

		push eax

		push edx// extension
		push eax//savePath
		call FDeleteTmpSaveFiles
		add esp, 0x8

		pop ebx

		push eax

		push ebx//mapName (+ extension)
		call FogOfWarMap_DeleteTmps
		add esp, 0x4

		pop eax

		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//__________________________________________
void __declspec(naked) fog_of_war_save(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		push eax

		push edx// "rb"
		push eax//maps\(mapName).ext
		call F_fopen
		add esp, 0x8

		pop ebx

		cmp eax, 0
		je endFunc

		push eax

		push ebx//mapName (+ extension)
		call FogOfWarMap_Save
		add esp, 0x4

		pop eax

		endFunc :
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//__________________________________________
void __declspec(naked) fog_of_war_load(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		push eax//FileStream
		call F_fclose
		add esp, 0x4

		push eax

		push ebx//mapName (+ extension)
		call FogOfWarMap_Load
		add esp, 0x4

		pop eax

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//__________________________________________
void __declspec(naked) set_map_borders(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx// "rb"
		push eax//maps\(mapName).ext
		call F_fopen
		add esp, 0x8
		cmp eax, 0
		je endFunc

		push eax

		push ebx//mapName (+ extension)
		call SetMapBorders
		add esp, 0x4

		pop eax

		endFunc :
		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}




//_______________________________________
void __declspec(naked) set_view_pos(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx//current level
		push eax//current view hex pos
		call SetViewPos
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//_________________________________________
LONG SetViewFocus(LONG hexPos, DWORD flags) {

	SetViewPos(hexPos, 3);
	return 0;
}


//_________________________________________
void __declspec(naked) set_view_focus(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call SetViewFocus
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//____________________________________________________________
LONG ClipEdgeRectScroll(RECT* rect1, RECT* rect2, RECT* rect3) {

	WinStruct* win = GetWinStruct(*pWinRef_GameArea);
	BYTE* buff = win->buff;
	buff += rect1->top * win->width;
	buff += rect1->left;
	LONG height = rect1->bottom - rect1->top + 1;
	LONG width = rect1->right - rect1->left + 1;
	for (LONG y = 0; y < height; y++) {
		memset(buff, '\0', width);
		buff += win->width;
	}

	return ClipEdgeRect(rect1, rect2, rect3);
}


//________________________________________________
void __declspec(naked) clip_edge_rect_scroll(void) {

	__asm {
		push ecx
		push esi
		push edi
		push ebp

		push ebx
		push edx
		push eax
		call ClipEdgeRectScroll
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop ecx
		ret
	}
}


//___________________________________________________________
LONG ClipEdgeRectMouse(RECT* rect1, RECT* rect2, RECT* rect3) {

	if (MergeRects(rect1, rect2, rect3) == -1)
		return -1;
	RECT mouseEdgeRect = { edgeRect.left + 1, edgeRect.top + 1, edgeRect.right - 1, edgeRect.bottom - 1 };
	return MergeRects(rect3, &mouseEdgeRect, rect3);
}


//_______________________________________________
void __declspec(naked) clip_edge_rect_mouse(void) {

	__asm {
		push ecx
		push esi
		push edi
		push ebp

		push ebx
		push edx
		push eax
		call ClipEdgeRectMouse
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop ecx
		ret
	}
}


//_________________________________________
void __declspec(naked) clip_edge_rect(void) {

	__asm {
		push ecx
		push esi
		push edi
		push ebp

		push ebx
		push edx
		push eax
		call ClipEdgeRect
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop ecx
		ret
	}
}


//_______________________________________________
void __declspec(naked) get_floor_tile_light(void) {

	//eax == elevation, edx == hexNum
	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push ebp //global light
		push edx //hexNum
		push eax //elevation
		call GetFloorHexLight
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//___________________________________________
void __declspec(naked) draw_floor_tiles(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call DrawFloorTiles
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}

}


//______________________________________________________
void __declspec(naked) check_angled_roof_tile_edge(void) {

	__asm {
		push edx
		push dword ptr ss : [esp + 0x28]
		push edi
		call CheckAngledRoofTileEdge
		add esp, 0xC

		cmp eax, -1
		jne exitFunc
		mov eax, esi
		exitFunc :
		ret 4
	}
}


//_______________________________________________________
void __declspec(naked) check_angled_roof_tile_edge2(void) {

	__asm {
		push edx
		push dword ptr ss : [esp + 0x10]
		push dword ptr ss : [esp + 0x10]
		call CheckAngledRoofTileEdge
		add esp, 0xC

		cmp eax, -1
		jne exitFunc
		xor eax, eax
		add dword ptr ss : [esp] , 0x9//skip tile draw if pos invalid
		exitFunc :
		ret 4
	}
}


//____________________
void CheckPcMovement() {

	if (!FOG_OF_WAR)
		return;

	static LONG pcHexNum = 0;
	LONG objType = 0;
	///RECT rcObj;
	OBJStruct* pcObj = *lpObj_PC;

	if (pcObj->hexNum != pcHexNum) {
		pcHexNum = pcObj->hexNum;

		ReDrawViewWin();
	}
}


//_________________________________________
void __declspec(naked) check_pc_movement() {
	__asm {
		push eax
		push ebx
		push edx

		call CheckPcMovement

		pop edx
		pop ebx
		pop eax

		xor ebp, ebp
		cmp eax, 0x5
		ret
	}
}


//__________________________________
void __declspec(naked) h_draw_objs() {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call DrawObjects
		add esp, 0x8

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}
}




//___________________________________________
void __declspec(naked) h_get_objects_at_pos() {

	__asm {
		push esi
		push edi
		push ebp

		push dword ptr ss : [esp + 0x10]
		push ecx
		push ebx
		push edx
		push eax
		call GetObjectsAtPos
		add esp, 0x14

		pop ebp
		pop edi
		pop esi
		ret 0x4
	}
}


//________________
void MapFixes_CH() {

	FuncReplace32(0x44C683, 0x03F33D, (DWORD)&h_get_objects_at_pos);

	MemWrite8(0x488950, 0x53, 0xE9);
	FuncWrite32(0x488951, 0x55575651, (DWORD)&h_draw_objs);

	MemWrite32(0x488C1D, 0x64A180, (DWORD)&pCombatOutlineList);
	MemWrite32(0x488C0D, 0x529414, (DWORD)&combatOutlineCount);
	MemWrite32(0x488C29, 0x529414, (DWORD)&combatOutlineCount);

	if (ConfigReadInt("MAPS", "IGNORE_MAP_EDGES", 0))
		FuncWrite32(0x4812A7, 0x02F665, (DWORD)0x4B091C);

	if (ConfigReadInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 0))
		FuncWrite32(0x4812AC, 0x02F680, (DWORD)0x4B093C);

	//FIX - ORIGINALLY PLAYER POSITION SET TO SCROLL POSITION FOR JUMP TO MAP
	//NOW PLAYER POSITION SET BEFORE SCROLL
	MemWrite32(0x482260, 0x67C3B4, 0x52934C);

	FuncWrite32(0x481F12, 0x043076, (DWORD)&set_map_borders);

	MemWrite8(0x4B24B4, 0x53, 0xE9);
	FuncWrite32(0x4B24B5, 0x55575651, (DWORD)&set_view_focus);

	FuncWrite32(0x44A686, 0x08E832, (DWORD)&DrawDialogView);

	MemWrite16(0x4B05FC, 0x5651, 0xE990);
	FuncWrite32(0x4B05FE, 0xDE895557, (DWORD)&get_next_hex_pos);

	MemWrite8(0x4B03EC, 0x53, 0xE9);
	FuncWrite32(0x4B03ED, 0x55575651, (DWORD)&get_hex_dist);


	FuncWrite32(0x483321, 0x0433A3, (DWORD)&clip_edge_rect_scroll);
	FuncWrite32(0x483384, 0x043340, (DWORD)&clip_edge_rect);
	FuncWrite32(0x4B00F6, 0x0165CE, (DWORD)&clip_edge_rect);
	FuncWrite32(0x4B0187, 0x01653D, (DWORD)&clip_edge_rect);
	FuncWrite32(0x488C03, 0x03DAC1, (DWORD)&clip_edge_rect_mouse);

	MemWrite8(0x4AFE88, 0x53, 0xE9);
	FuncWrite32(0x4AFE89, 0x55575651, (DWORD)&set_view_pos);


	FuncWrite32(0x4B0DF2, 0xFFF68E2A, (DWORD)&check_angled_roof_tile_edge);

	FuncWrite32(0x4B1788, 0xFFF68494, (DWORD)&check_angled_roof_tile_edge2);

	MemWrite8(0x4425C6, 0x31, 0xE8);
	FuncWrite32(0x4425C7, 0x05F883ED, (DWORD)&check_pc_movement);

	MemWrite8(0x4B14D4, 0x53, 0xE9);
	FuncWrite32(0x4B14D5, 0x55575651, (DWORD)&draw_floor_tiles);

	FuncReplace32(0x4B1E30, 0xFFFC814C, (DWORD)&get_floor_tile_light);
	MemWrite16(0x4B1E34, 0xE839, 0x15EB);

	FuncReplace32(0x482D41, 0x042247, (DWORD)&fog_of_war_save);

	FuncReplace32(0x47AE36, 0x467A, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x47AE68, 0x4648, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x47EB36, 0x097A, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x47EF28, 0x0588, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x47F4A6, 0x0A, (DWORD)&fog_of_war_delete_tmps);

	MemWrite16(0x47ECA9, 0xC483, 0xE890);
	FuncWrite32(0x47ECAB, 0xFFF88308, (DWORD)&fog_of_war_move_to_slot);

	MemWrite16(0x47F10C, 0xC483, 0xE890);
	FuncWrite32(0x47F10E, 0xFFF88308, (DWORD)&fog_of_war_move_from_slot);

	FuncReplace32(0x481F26, 0x04301F, (DWORD)&fog_of_war_load);


	//block fallout method for marking objects as visible.
	if (FOG_OF_WAR)
		MemWrite8(0x48BBA0, 0x53, 0xC3);
}



//_____________________________
void MapFixes_MULTI(int region) {

	FuncReplace32(0x44CF33, 0x03F68D, (DWORD)&h_get_objects_at_pos);

	MemWrite8(0x489550, 0x53, 0xE9);
	FuncWrite32(0x489551, 0x55575651, (DWORD)&h_draw_objs);

	MemWrite32(0x48981D, FixAddress(0x639C00), (DWORD)&pCombatOutlineList);

	MemWrite32(0x48980D, FixAddress(0x519624), (DWORD)&combatOutlineCount);
	MemWrite32(0x489829, FixAddress(0x519624), (DWORD)&combatOutlineCount);

	if (ConfigReadInt("MAPS", "IGNORE_MAP_EDGES", 0))
		FuncReplace32(0x481E77, 0x02FF05, (DWORD)0x4B1D8C);

	if (ConfigReadInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 0))
		FuncReplace32(0x481E7C, 0x02FF20, (DWORD)0x4B1DAC);

	//FIX - ORIGINALLY PLAYER POSITION SET TO SCROLL POSITION FOR JUMP TO MAP
	//NOW PLAYER POSITION SET BEFORE SCROLL
	MemWrite32(0x482E30, 0x66BE34, FixAddress(0x51955C));

	FuncReplace32(0x482AE2, 0x0433E2, (DWORD)&set_map_borders);

	MemWrite8(0x4B3924, 0x53, 0xE9);
	FuncWrite32(0x4B3925, 0x55575651, (DWORD)&set_view_focus);

	FuncReplace32(0x44AF3A, 0x088796, (DWORD)&DrawDialogView);

	MemWrite16(0x4B1A6C, 0x5651, 0xE990);
	FuncWrite32(0x4B1A6E, 0xDE895557, (DWORD)&get_next_hex_pos);

	MemWrite8(0x4B185C, 0x53, 0xE9);
	FuncWrite32(0x4B185D, 0x55575651, (DWORD)&get_hex_dist);


	FuncReplace32(0x483EF1, 0x042D73, (DWORD)&clip_edge_rect_scroll);
	FuncReplace32(0x483F54, 0x042D10, (DWORD)&clip_edge_rect);
	FuncReplace32(0x4B1566, 0x0156FE, (DWORD)&clip_edge_rect);
	FuncReplace32(0x4B15F7, 0x01566D, (DWORD)&clip_edge_rect);
	FuncReplace32(0x489803, 0x03D461, (DWORD)&clip_edge_rect_mouse);

	MemWrite8(0x4B12F8, 0x53, 0xE9);
	FuncWrite32(0x4B12F9, 0x55575651, (DWORD)&set_view_pos);

	FuncReplace32(0x4B2262, 0xFFF67A22, (DWORD)&check_angled_roof_tile_edge);

	FuncReplace32(0x4B2BF8, 0xFFF6708C, (DWORD)&check_angled_roof_tile_edge2);

	MemWrite8(0x442D56, 0x31, 0xE8);
	FuncWrite32(0x442D57, 0x05F883ED, (DWORD)&check_pc_movement);

	MemWrite8(0x4B2944, 0x53, 0xE9);
	FuncWrite32(0x4B2945, 0x55575651, (DWORD)&draw_floor_tiles);

	FuncReplace32(0x4B32A0, 0xFFFC76DC, (DWORD)&get_floor_tile_light);
	MemWrite16(0x4B32A4, 0xE839, 0x15EB);

	FuncReplace32(0x483911, 0x0425B3, (DWORD)&fog_of_war_save);

	FuncReplace32(0x47B836, 0x4806, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x47B868, 0x47D4, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x47F6C2, 0x097A, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x47FAB4, 0x0588, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x480032, 0x0A, (DWORD)&fog_of_war_delete_tmps);

	MemWrite16(0x47F835, 0xC483, 0xE890);
	FuncWrite32(0x47F837, 0xFFF88308, (DWORD)&fog_of_war_move_to_slot);

	MemWrite16(0x47FC98, 0xC483, 0xE890);
	FuncWrite32(0x47FC9A, 0xFFF88308, (DWORD)&fog_of_war_move_from_slot);

	FuncReplace32(0x482AF6, 0x0433BA, (DWORD)&fog_of_war_load);

	//block fallout method for marking objects as visible.
	if (FOG_OF_WAR)
		MemWrite8(0x48C7A0, 0x53, 0xC3);
}


//________________________________
void SetMapPathFinding(int region) {

	//path finding/////////////////////////////////////////////////////////////////
	int multi = ConfigReadInt("MAPS", "NumPathNodes", 1);

	if (multi == numPathNodes / 2000)
		return;

	if (multi > 0 && multi <= 20) {
		if (PathFindNodes01)
			delete PathFindNodes01;
		PathFindNodes01 = nullptr;
		if (PathFindNodes02)
			delete PathFindNodes02;
		PathFindNodes02 = nullptr;

		numPathNodes = multi * 2000;

		PathFindNodes01 = new DWORD[numPathNodes * 20];
		PathFindNodes02 = new DWORD[numPathNodes * 20];

		if (region == 4) {
			MemWriteUnSafe32(0x416177, numPathNodes);

			MemWriteUnSafe32(0x415FE4, numPathNodes * 20);
			MemWriteUnSafe32(0x4160D8, numPathNodes * 20);

			MemWriteUnSafe32(0x415FDF, (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415F8D, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x416035, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x416070, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x41607A, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x416153, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x416189, 20 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415FB6, 24 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415FA2, 28 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415FC9, 32 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415FD6, 36 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x416160, 40 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x4160B9, (DWORD)PathFindNodes02);
			MemWriteUnSafe32(0x41628E, (DWORD)PathFindNodes02);
			MemWriteUnSafe32(0x4162BA, (DWORD)PathFindNodes02);

			MemWriteUnSafe32(0x4162A6, 20 + (DWORD)PathFindNodes02);
		}
		else {
			MemWriteUnSafe32(0x416177, numPathNodes);

			MemWriteUnSafe32(0x415FE4, numPathNodes * 20);
			MemWriteUnSafe32(0x4160D8, numPathNodes * 20);

			MemWriteUnSafe32(0x415FDF, (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415F8D, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x416035, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x416070, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x41607A, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x416153, 20 + (DWORD)PathFindNodes01);
			MemWriteUnSafe32(0x416189, 20 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415FB6, 24 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415FA2, 28 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415FC9, 32 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x415FD6, 36 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x416160, 40 + (DWORD)PathFindNodes01);

			MemWriteUnSafe32(0x4160B9, (DWORD)PathFindNodes02);
			MemWriteUnSafe32(0x41628E, (DWORD)PathFindNodes02);
			MemWriteUnSafe32(0x4162BA, (DWORD)PathFindNodes02);

			MemWriteUnSafe32(0x4162A6, 20 + (DWORD)PathFindNodes02);
		}
	}
}


//________________________________
void UpdateMapSettings(int region) {
	SetMapGlobals(region);
	SetMapPathFinding(region);

	if (ConfigReadInt("MAPS", "IGNORE_MAP_EDGES", 0))
		*pSCROLL_BLOCK_FLAG = 0;
	else
		*pSCROLL_BLOCK_FLAG = 1;

	if (ConfigReadInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 0))
		*pPC_SCROLL_LIMIT_FLAG = 0;
	else
		*pPC_SCROLL_LIMIT_FLAG = 1;
	///#endif
	if (FOG_OF_WAR) {
		if (region == 4)
			MemWriteUnSafe8(0x48BBA0, 0xC3);
		else
			MemWriteUnSafe8(0x48C7A0, 0xC3);
	}
	else {
		if (region == 4)
			MemWriteUnSafe8(0x48BBA0, 0x53);
		else
			MemWriteUnSafe8(0x48C7A0, 0x53);
	}
}


//_________________________
void MapFixes(DWORD region) {

	SetMapGlobals(region);

	if (region == 4)
		MapFixes_CH();
	else
		MapFixes_MULTI(region);

	SetMapPathFinding(region);
}
