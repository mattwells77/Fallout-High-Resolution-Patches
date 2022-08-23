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


//________________________________________________
void __declspec(naked) fog_of_war_copy_files(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push edx//pToPath
		push eax//pFromPath
		call F_CopySaveFile
		cmp eax, -1
		jne saveFog
		add esp, 0x08
		ret
		saveFog :
		call FogOfWarMap_CopyFiles
		add esp, 0x08
		mov eax, 0

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
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
		push edi//dword ptr ss:[esp+0x30]
		call CheckAngledRoofTileEdge
		add esp, 0xC

		cmp eax, -1
		jne exitFunc
		mov eax, esi
		//add dword ptr ss:[esp], 0xF//skip tile draw if pos invalid
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


//________________________________________
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


//______________________
void SetMapPathFinding() {

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

		MemWrite32(0x415C33, 2000, numPathNodes);

		MemWrite32(0x415AB3, 40000, numPathNodes * 20);
		MemWrite32(0x415B84, 40000, numPathNodes * 20);

		MemWrite32(0x415AAE, 0x54CA80, (DWORD)PathFindNodes01);

		MemWrite32(0x415A60, 0x54CA94, 20 + (DWORD)PathFindNodes01);
		MemWrite32(0x415AEB, 0x54CA94, 20 + (DWORD)PathFindNodes01);
		MemWrite32(0x415B33, 0x54CA94, 20 + (DWORD)PathFindNodes01);
		MemWrite32(0x415B51, 0x54CA94, 20 + (DWORD)PathFindNodes01);
		MemWrite32(0x415C12, 0x54CA94, 20 + (DWORD)PathFindNodes01);
		MemWrite32(0x415C53, 0x54CA94, 20 + (DWORD)PathFindNodes01);

		MemWrite32(0x415A73, 0x54CA98, 24 + (DWORD)PathFindNodes01);
		MemWrite32(0x415C67, 0x54CA98, 24 + (DWORD)PathFindNodes01);

		MemWrite32(0x415A8B, 0x54CA9C, 28 + (DWORD)PathFindNodes01);
		MemWrite32(0x415C75, 0x54CA9C, 28 + (DWORD)PathFindNodes01);

		MemWrite32(0x415A9A, 0x54CAA0, 32 + (DWORD)PathFindNodes01);
		MemWrite32(0x415AFE, 0x54CAA0, 32 + (DWORD)PathFindNodes01);
		MemWrite32(0x415B0C, 0x54CAA0, 32 + (DWORD)PathFindNodes01);
		MemWrite32(0x415C84, 0x54CAA0, 32 + (DWORD)PathFindNodes01);

		MemWrite32(0x415AA0, 0x54CAA4, 36 + (DWORD)PathFindNodes01);
		MemWrite32(0x415B04, 0x54CAA4, 36 + (DWORD)PathFindNodes01);
		MemWrite32(0x415B12, 0x54CAA4, 36 + (DWORD)PathFindNodes01);
		MemWrite32(0x415C95, 0x54CAA4, 36 + (DWORD)PathFindNodes01);
		MemWrite32(0x415CAD, 0x54CAA4, 36 + (DWORD)PathFindNodes01);

		MemWrite32(0x415C1E, 0x54CAA8, 40 + (DWORD)PathFindNodes01);

		MemWrite32(0x415B71, 0x5566D4, (DWORD)PathFindNodes02);
		MemWrite32(0x415CEC, 0x5566D4, (DWORD)PathFindNodes02);
		MemWrite32(0x415D18, 0x5566D4, (DWORD)PathFindNodes02);

		MemWrite32(0x415D04, 0x5566E8, 20 + (DWORD)PathFindNodes02);
	}
}


//______________________
void UpdateMapSettings() {
	SetMapGlobals();
	SetMapPathFinding();

	if (ConfigReadInt("MAPS", "IGNORE_MAP_EDGES", 0))
		*pSCROLL_BLOCK_FLAG = 0;
	else
		*pSCROLL_BLOCK_FLAG = 1;

	if (ConfigReadInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 0))
		*pPC_SCROLL_LIMIT_FLAG = 0;
	else
		*pPC_SCROLL_LIMIT_FLAG = 1;
	///#endif

	if (FOG_OF_WAR)
		MemWriteUnSafe8(0x47E314, 0xC3);
	else
		MemWriteUnSafe8(0x47E314, 0x53);

}


//_____________
void MapFixes() {

	SetMapGlobals();

	FuncReplace32(0x444E27, 0x039325, (DWORD)&h_get_objects_at_pos);

	MemWrite8(0x47B7E0, 0x53, 0xE9);
	FuncWrite32(0x47B7E1, 0x55575651, (DWORD)&h_draw_objs);

	MemWrite32(0x47BAAD, FixAddress(0x638170), (DWORD)&pCombatOutlineList);

	MemWrite32(0x47BA9D, FixAddress(0x505C90), (DWORD)&combatOutlineCount);
	MemWrite32(0x47BAB9, FixAddress(0x505C90), (DWORD)&combatOutlineCount);

	//00473EFE  |.  E8 81AE0200   CALL 0049ED84                            ; set_scroll_blocker_flag()
	//0049ED90  /$  52            PUSH EDX                                 ; UNSET_SCROLL_BLOCKER_FLAG()
	if (ConfigReadInt("MAPS", "IGNORE_MAP_EDGES", 0))
		FuncReplace32(0x473EFF, 0x02AE81, (DWORD)0x49ED90);

	//00473F03  |.  E8 9CAE0200   CALL 0049EDA4                            ; [Falloutw.0049EDA4, set_pc_scroll_limit_flag(>
	//0049EDB0  /$  52            PUSH EDX                                 ; UNSET_PC_SCROLL_LIMIT_FLAG()
	if (ConfigReadInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 0))
		FuncReplace32(0x473F04, 0x02AE9C, (DWORD)0x49EDB0);


	//FIX - ORIGINALLY PLAYER POSITION SET TO SCROLL POSITION FOR JUMP TO MAP
	//NOW PLAYER POSITION SET BEFORE SCROLL
	MemWrite32(0x474D9A, FixAddress(0x668E74), FixAddress(0x630304));

	FuncReplace32(0x474AD0, 0x03B420, (DWORD)&set_map_borders);


	MemWrite8(0x4A08D4, 0x53, 0xE9);
	FuncWrite32(0x4A08D5, 0x55575651, (DWORD)&set_view_focus);

	FuncReplace32(0x4422EE, 0x07C1A2, (DWORD)&DrawDialogView);

	MemWrite16(0x49EA80, 0x5651, 0xE990);
	FuncWrite32(0x49EA82, 0xDE895557, (DWORD)&get_next_hex_pos);

	MemWrite8(0x49E96C, 0x53, 0xE9);
	FuncWrite32(0x49E96D, 0x55575651, (DWORD)&get_hex_dist);


	FuncReplace32(0x47609D, 0x03D1A7, (DWORD)&clip_edge_rect_scroll);

	FuncReplace32(0x476100, 0x03D144, (DWORD)&clip_edge_rect);
	FuncReplace32(0x49E65A, 0x014BEA, (DWORD)&clip_edge_rect);
	FuncReplace32(0x49E6EB, 0x014B59, (DWORD)&clip_edge_rect);

	FuncReplace32(0x47BA93, 0x0377B1, (DWORD)&clip_edge_rect_mouse);

	MemWrite8(0x49E3EC, 0x53, 0xE9);
	FuncWrite32(0x49E3ED, 0x55575651, (DWORD)&set_view_pos);


	FuncReplace32(0x49F264, 0xFFF7A1E4, (DWORD)&check_angled_roof_tile_edge);

	FuncReplace32(0x49FBAC, 0xFFF7989C, (DWORD)&check_angled_roof_tile_edge2);


	MemWrite8(0x43BA8E, 0x31, 0xE8);
	FuncWrite32(0x43BA8F, 0x05F883ED, (DWORD)&check_pc_movement);

	MemWrite8(0x49F8FC, 0x53, 0xE9);
	FuncWrite32(0x49F8FD, 0x55575651, (DWORD)&draw_floor_tiles);

	FuncReplace32(0x4A0250, 0xFFFCCCF4, (DWORD)&get_floor_tile_light);
	MemWrite16(0x4A0254, 0xE839, 0x15EB);

	FuncReplace32(0x475961, 0x03A58F, (DWORD)&fog_of_war_save);


	FuncReplace32(0x46DE06, 0x42C2, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x46DE1C, 0x42AC, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x4719F2, 0x06D6, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x471C9D, 0x042B, (DWORD)&fog_of_war_delete_tmps);
	FuncReplace32(0x4720C3, 0x0005, (DWORD)&fog_of_war_delete_tmps);

	//fog files move to save slot
	FuncWrite32(0x471B32, 0x03B6, (DWORD)&fog_of_war_copy_files);

	//fog files move from save slot
	FuncWrite32(0x471D4C, 0x019C, (DWORD)&fog_of_war_copy_files);

	FuncReplace32(0x474AE4, 0x03E0AC, (DWORD)&fog_of_war_load);

	///MARK_VISIBLE_OBJS() within 400 pix from PC
	if (FOG_OF_WAR)
		MemWrite8(0x47E314, 0x53, 0xC3);


	SetMapPathFinding();
}
