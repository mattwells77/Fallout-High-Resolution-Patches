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

#include "memwrite.h"
#include "convert.h"

#include "F_Mapper.h"
#include "F_Objects.h"
#include "F_Scripts.h"

#include "F_Windows.h"
#include "F_Art.h"
#include "F_Text.h"
#include "F_Msg.h"
#include "F_File.h"

#include "fixes.h"

#include "configTools.h"

#include "WinFall.h"


LONG* pGRID001_LST_NUM = nullptr;

DWORD* pDRAW_VIEW_FLAG = nullptr;

LONG* pAreRoovesVisible = nullptr;

LONG* pAreMapHexesVisible = nullptr;

void* F_GET_SCRN_HEX_POS = nullptr;

void* F_DRAW_MAP_HEXES = nullptr;


void* F_FALLOUT2_SETUP = nullptr;

void* F_INIT_MAP_AREA = nullptr;

void* F_CHECK_GAME_INPUT = nullptr;

void* F_RESET_NEW_MAP = nullptr;

void* F_LOAD_MAP_FROM_LIST = nullptr;

void* F_LOAD_MAP = nullptr;

void* F_SAVE_MAP = nullptr;

void* F_RESET_IFACE_BAR = nullptr;

void* F_SET_MOUSE_MODE_FLAG = nullptr;

void* F_INTFACE_INIT = nullptr;

void* F_INTFACE_DESTROY = nullptr;

void* F_GET_HEX_LIGHT_LEVEL = nullptr;
void* F_DRAW_OBJECT = nullptr;

void* F_CHECK_OBJ_FRM_AT_POS = nullptr;
void* F_PLACE_OBJ_ONTO_MAP = nullptr;
void* F_GET_OBJ_RECT = nullptr;
void* F_DRAW_FLOOR_TILE = nullptr;
void** pF_DRAW_MAP_AREA = nullptr;

void* F_SET_MAP_LEVEL = nullptr;

void* F_SELECT_HEXPOS = nullptr;

void* F_PATHFUNC02 = nullptr;

void* F_SAVE_MAP_DATA = nullptr;

void* F_COPY_SAVE_FILE = nullptr;

int FOG_OF_WAR = false;
LONG fogLight = 0x1000;
bitset* fogHexMapBits = nullptr;
bool isRecordingObjFog = false;//needed to prevent obj discovery before true pcObj position set.

LONG SCROLL_DIST_X = 480;
LONG SCROLL_DIST_Y = 400;


bool EDGE_CLIPPING_ON = false;

LONG* pMOUSE_PIC_NUM = nullptr;

LONG* pMAP_LEVEL = nullptr;

LONG* pNUM_HEX_X = nullptr;
LONG* pNUM_HEX_Y = nullptr;
LONG* pNUM_HEXES = nullptr;

LONG* pNUM_TILE_Y = nullptr;
LONG* pNUM_TILE_X = nullptr;
LONG* pNUM_TILES = nullptr;

LONG* pVIEW_SQU_HEX_X = nullptr;
LONG* pVIEW_SQU_HEX_Y = nullptr;

LONG* pVIEW_SQU_TILE_X = nullptr;
LONG* pVIEW_SQU_TILE_Y = nullptr;
LONG* pVIEW_TILE_X = nullptr;
LONG* pVIEW_TILE_Y = nullptr;

LONG* pVIEW_HEXPOS = nullptr;
LONG* pVIEW_HEX_X = nullptr;
LONG* pVIEW_HEX_Y = nullptr;

BYTE* pHEXPOS_ADJUSTMENT = nullptr;

DWORD* pSCROLL_BLOCK_FLAG = nullptr;

DWORD* pPC_SCROLL_LIMIT_FLAG = nullptr;

LONG* pAmbientLightIntensity = nullptr;
LONG* pLightHexArray = nullptr;


OBJNode** pMapObjNodeArray = 0;
OBJNode* upperMapObjNodeArray[40000];
OBJStruct* pCombatOutlineList[500];
int combatOutlineCount = 0;

DWORD*** pMapTileLevelOffset = nullptr;



BYTE** lpGameWinBuff = nullptr;
RECT* prcGameWin = nullptr;

BYTE** lpViewWinBuff = nullptr;
LONG* pViewWinWidth1 = nullptr;
LONG* pViewWinHeight = nullptr;
LONG* pViewWinWidth2 = nullptr;
RECT* prcViewWin = nullptr;

BYTE** lpObjViewTextBuff = nullptr;
LONG* pObjViewTextWidth = nullptr;
LONG* pObjViewTextHeight = nullptr;

BYTE** lpViewWin2Buff = nullptr;
LONG* pViewWin2Width1 = nullptr;
LONG* pViewWin2Height = nullptr;
LONG* pViewWin2Width2 = nullptr;
RECT* prcViewWin2 = nullptr;
LONG* pViewWin2Size = nullptr;


OBJStruct** lpObj_PC = nullptr;
OBJStruct** lpObj_ActiveCritter = nullptr;
OBJStruct** lpObj_DialogFocus = nullptr;
OBJStruct** lpObjSpecial = nullptr;

//map blocker vars
MAPdata M_CURRENT_MAP;
bool edgeInitiated = false;


LONG EDGE_OFF_X = 0;
LONG EDGE_OFF_Y = 0;
LONG EDGE_X_DIFF = 0;
LONG EDGE_Y_DIFF = 0;
RECT edgeRect;

bool isAngledEdges = false;

///mapper
MAPedge defaultEDGES;
char workingMapName[32];
char* pCurrentMapName = nullptr;


//______________________________________________________
LONG F_GetScrnHexPos(LONG scrnX, LONG scrnY, LONG level) {

	LONG retVal = 0;
	__asm {
		mov ebx, level
		mov edx, scrnY
		mov eax, scrnX
		call F_GET_SCRN_HEX_POS
		mov retVal, eax
	}

	return retVal;
}


//_______________________________________________________
OBJStruct* GetNextEnabledObject(bool isStart, LONG level) {
	//Gets the next enabled object from the current map object array. Run first with isStart true to set level and reset variables.
	static LONG CK_level = 0;
	static  LONG CK_hexNum = 0;
	static  OBJNode* CK_pObjNode = 0;

	OBJNode* pObjNode = nullptr;
	LONG artType = -1;

	if (isStart) {
		CK_hexNum = 0;
		CK_pObjNode = nullptr;
		CK_level = level;
		pObjNode = pMapObjNodeArray[CK_hexNum];
	}
	else
		pObjNode = CK_pObjNode->next;

	while (CK_hexNum < *pNUM_HEXES) {
		while (pObjNode != nullptr) {
			CK_pObjNode = pObjNode;
			if (pObjNode->obj->level == CK_level) {
				artType = (pObjNode->obj->frmID & 0x0F000000) >> 24;
				if (IsArtTypeEnabled(artType)) {
					CK_pObjNode = pObjNode;
					return pObjNode->obj;
				}
			}
			pObjNode = pObjNode->next;
		}
		CK_hexNum++;
		pObjNode = pMapObjNodeArray[CK_hexNum];
	}
	CK_pObjNode = pObjNode;
	return nullptr;
}


//copy a file to or from save game slot  -returns 0 pass, -1 fail.
//_______________________________________________
LONG F_CopySaveFile(char* toPath, char* fromPath) {

	LONG retVal = 0;
	__asm {
		mov edx, fromPath
		mov eax, toPath
		call F_COPY_SAVE_FILE
		mov retVal, eax
	}
	return retVal;
}


//__________________________________
LONG F_SaveMapData(void* fileStream) {

	LONG retVal = 0;
	__asm {
		mov eax, fileStream
		call F_SAVE_MAP_DATA
		mov retVal, eax
	}

	return retVal;
}


//_____________________________________________________________
OBJStruct* F_PathFunc2(OBJStruct* obj, LONG hexNum, LONG level) {

	OBJStruct* pObj = nullptr;
	__asm {
		mov ebx, level
		mov edx, hexNum
		mov eax, obj
		call F_PATHFUNC02
		mov pObj, eax
	}

	return pObj;
}


//_________________________________________
void SetMapObject(DWORD frmID, DWORD proID) {
	LONG xPos = 0, yPos = 0;
	F_GetMousePos(&xPos, &yPos);
	LONG hexNum = F_GetScrnHexPos(xPos, yPos, 0);
	if (hexNum == -1)
		return;

	OBJStruct* pObj = nullptr;

	((OBJStruct*)&lpObj_Mouse2)->flags = ((OBJStruct*)&lpObj_Mouse2)->flags | FLG_Disabled;
	pObj = F_PathFunc2(nullptr, hexNum, *pMAP_LEVEL);
	((OBJStruct*)&lpObj_Mouse2)->flags = ((OBJStruct*)&lpObj_Mouse2)->flags & (~FLG_Disabled);//& 0xFFFFFFFE;

	if (pObj != nullptr && proID != -1 && (proID >> 24) != ART_TILES) {
		PROTO pProto{ nullptr };
		F_GetPro(proID, &pProto.all);
		if (!(pProto.all->lightFlags & FLG_NoBlock))
			return;
	}
	pObj = GetNextEnabledObject(true, *pMAP_LEVEL);
	while (pObj != nullptr) {
		if (hexNum == pObj->hexNum) {
			if (frmID == pObj->frmID) {
				if (pObj != *lpObj_Mouse2 && pObj != *lpObj_Mouse) {
					if (pObj->proID == 41)//money item
						pObj->pud.general.pud.ammo.cur_ammo_quantity = pObj->pud.general.pud.ammo.cur_ammo_quantity++;
					return;
				}
			}

		}
		pObj = GetNextEnabledObject(false, 0);
	}

	F_MapObj_Create(&pObj, frmID, proID);

	RECT rect;
	F_MapObj_Move(pObj, hexNum, *pMAP_LEVEL, &rect);

	DWORD scriptID = -1;
	if (F_GetScriptID(pObj, &scriptID) != -1 || scriptID != -1) {

		SCRIPT_STRUCT* pScript;
		if (F_GetScriptStruct(scriptID, &pScript) == -1)
			return;
		if (scriptID >> 24 == 1)
			pScript->udata.sp.build_tile = hexNum | ((*pMAP_LEVEL << 29) & 0xE0000000);
		pScript->pObj = pObj;
	}

	F_DrawMapArea(&rect, pObj->level);
}



//_______________________________________________________________
DWORD F_CheckObjectFrmAtPos(OBJStruct* obj, LONG xPos, LONG yPos) {

	DWORD flags = 0;
	__asm {
		mov ebx, yPos
		mov edx, xPos
		mov eax, obj
		call F_CHECK_OBJ_FRM_AT_POS
		mov flags, eax
	}

	return flags;
}


//___________________
void F_IntFace_Init() {

	__asm {
		call F_INTFACE_INIT
	}
}


//______________________
void F_IntFace_Destroy() {

	__asm {
		call F_INTFACE_DESTROY
	}
}


//_________________________________
void F_SetMouseModeFlag(DWORD flag) {

	__asm {
		mov eax, flag
		call F_SET_MOUSE_MODE_FLAG
	}
}


//____________________
void F_ResetIfaceBar() {

	__asm {
		call F_RESET_IFACE_BAR
	}
}


//___________________________
void F_LoadMap(char* mapName) {

	__asm {
		mov eax, mapName
		call F_LOAD_MAP
	}
}


//___________________
bool SaveMapCurrent() {


	char* pPatchPath = nullptr;
	if (FReadCfgString(pCFG_FILE_PTR, "system", "master_patches", &pPatchPath) != 1) {
		MessageBoxA(0, "SaveMap - Could not read master_patches path from cfg file", "Hi-Res patch Error", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	char savePath[MAX_PATH];
	ZeroMemory(savePath, MAX_PATH);
	strncat(savePath, pPatchPath, MAX_PATH);

	strncat(savePath, "\\MAPS", 5);
	if (GetFileAttributes(savePath) == INVALID_FILE_ATTRIBUTES) {
		if (!CreateDirectory(savePath, nullptr)) {
			MessageBoxA(0, savePath, "Hi-Res patch Error", MB_OK | MB_ICONINFORMATION);
			MessageBoxA(0, "SaveMap - Could not create MAPS folder", "Hi-Res patch Error", MB_OK | MB_ICONINFORMATION);
			return false;
		}
	}

	if (pCurrentMapName[0] == '\0')
		return false;

	sprintf(savePath, "maps\\%s", pCurrentMapName);

	void* fileStream = F_fopen(savePath, "wb");

	LONG errorVal = 0;
	if (fileStream) {
		errorVal = F_SaveMapData(fileStream);

		F_fclose(fileStream);
	}
	else {
		MessageBoxA(0, "SaveMap - Unable to open map file to write", "Hi-Res patch Error", MB_OK | MB_ICONINFORMATION);
		return false;
	}

	if (errorVal) {
		MessageBoxA(0, "SaveMap - SaveMapData() Failed", "Hi-Res patch Error", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	return true;
}


//_________________________
void SaveMap(char* mapName) {

	*strncpy(pCurrentMapName, mapName, 13);
	SaveMapCurrent();
}


//___________________________
void F_SaveMap(char* mapName) {

	__asm {
		mov eax, mapName
		call F_SAVE_MAP
	}
}


//__________________
void F_ResetNewMap() {

	__asm {
		call F_RESET_NEW_MAP
	}
}


//_____________________________________________
void F_CheckGameInput(int keyCode, DWORD flags) {

	__asm {
		mov edx, flags
		mov eax, keyCode
		call F_CHECK_GAME_INPUT
	}
}


//__________________
void F_InitMapArea() {

	__asm {
		call F_INIT_MAP_AREA
	}
}


//____________________________________________
int F_FalloutSetup(int pathLength, char* path) {

	int retVal = 0;
	__asm {
		mov edx, path
		mov eax, pathLength
		call F_FALLOUT2_SETUP
		mov retVal, eax
	}

	return retVal;
}


//_________________________________________
void F_DrawMapHexes(RECT* rect, LONG level) {

	__asm {
		mov eax, rect
		mov edx, level
		call F_DRAW_MAP_HEXES
	}
}


//__________________________________________________
LONG F_GetHexLightIntensity(LONG level, LONG hexPos) {

	LONG retVal = 0;
	__asm {
		mov edx, hexPos
		mov eax, level
		call F_GET_HEX_LIGHT_LEVEL
		mov retVal, eax
	}
	return retVal;
}


//______________________________________________________________
void F_DrawObj(OBJStruct* obj, RECT* rect, DWORD lightIntensity) {

	__asm {
		mov ebx, lightIntensity
		mov edx, rect
		mov eax, obj
		call F_DRAW_OBJECT
	}

}


//step adjustment array ori 0-5 {evenHex y, oddHex y, x}
LONG hexStep[][3] = {
	{0,-1, -1}, {1, 0,-1},  {1, 1, 0},  {1, 0, 1},  {0,-1, 1}, {-1,-1,0}
};
LONG hexStep2[][3] = {
	{0,-200, -1}, {200, 0,-1},  {200, 200, 0},  {200, 0, 1},  {0,-200, 1}, {-200,-200,0}
};


//____________________________________________________________
LONG GetNextHexPos(LONG hexPos, UINT direction, LONG distance) {
	//if(hexPos<=0 || hexPos>40000) return hexPos;

	LONG xMax = *pNUM_HEX_X;
	LONG yMax = *pNUM_HEX_Y;
	LONG y = hexPos / xMax;
	LONG x = hexPos % xMax;
	LONG hexNew = hexPos;
	while (distance > 0) {

		y += hexStep[direction][(x & 0x01)];
		hexNew += hexStep2[direction][(x & 0x01)];
		x += hexStep[direction][2];
		hexNew += hexStep2[direction][2];

		if (x < 0 || x >= xMax)
			return hexPos;
		if (y < 0 || y >= yMax)
			return hexPos;

		hexPos = hexNew;

		distance--;
	}
	return hexPos;
}



//__________________________________________________
void GetHexSqrXY(LONG hexPos, LONG* px, LONG* py) {
	//grid 1x =16pixels, 1y =12pixels
	//x must be divided by 2 to get hex width of 32pixels
	LONG xMax = *pNUM_HEX_X;
	LONG y = hexPos / xMax;
	LONG x = hexPos % xMax;

	*py = y + (x >> 1);
	*px = (x << 1) - *py;

}


//________________________________
void GetHexHexXY(LONG* px, LONG* py) {
	//converts square coord's in pixels to screen hex grid coord's px py
	LONG x = *px >> 5;///32;
	LONG y = *py / 12;

	*px = x + (y >> 1);
	x = *px & 0xFFFFFFFE;//sub odd num by 1
	*py = y - (x >> 1);
}


//________________________________________________
LONG GetHexDistance(LONG hexStart, LONG hexEnd) {

	if (hexStart == -1)
		return 9999;
	if (hexEnd == -1)
		return 9999;

	LONG xSquStart = 0, ySquStart = 0;
	LONG xSquEnd = 0, ySquEnd = 0;

	GetHexSqrXY(hexStart, &xSquStart, &ySquStart);
	GetHexSqrXY(hexEnd, &xSquEnd, &ySquEnd);

	LONG xDiff = abs(xSquEnd - xSquStart);
	LONG yDiff = abs(ySquEnd - ySquStart);

	if (yDiff >= xDiff)
		return yDiff;
	else
		return (xDiff + yDiff) >> 1;///2;

}


//____________________________________
void ScrnHex2Sqr(LONG* px, LONG* py) {
	//converts screen hex grid coord's to square coord's in pixels px py
	// 1 screen grid = 32x24 pixels
	LONG x = *px;
	LONG y = *py + (x >> 1);

	y = y & 0xFFFFFFFE;//flatten odd y val so that grid height is multiple of 24
	*py = (y << 3) + (y << 2);//y*12;

	x = (x << 1) - y;
	*px = 3200 + (x << 4); // *16
}


//____________________________________
void ScrnSqr2Hex(LONG* px, LONG* py) {
	//converts square coord's in pixels to screen hex grid coord's px py
	// 1 screen grid = 32x24 pixels
	LONG x = *px >> 5;// /32;
	LONG y = *py / 24;

	*px = x + y - 100;//-100 = removing the 3200 pixels (100x32)that were added to fix negetive values in square dimensions

	x = *px & 0xFFFFFFFE;//sub odd num by 1
	*py = (y << 1) - (x >> 1);
}


//_____________________________________________________
void ScrnHexPos2Sqr(LONG hexPos, LONG* px, LONG* py) {

	LONG xMax = *pNUM_HEX_X;
	*py = hexPos / xMax;
	*px = hexPos % xMax;

	ScrnHex2Sqr(px, py);
}


//____________________________________
LONG ScrnSqr2HexPos(LONG x, LONG y) {
	//converts square coord's in pixels to hex grid position
	ScrnSqr2Hex(&x, &y);
	LONG xMax = *pNUM_HEX_X;
	LONG yMax = *pNUM_HEX_Y;

	if (x >= xMax)
		x = xMax - 1;
	else if (x <= 0)
		x = 0;
	if (y >= yMax)
		y = yMax - 1;
	else if (y <= 0)
		y = 0;
	return y * xMax + x;
}


//___________________________________________________
LONG ScrnSqr2HexPosMove(LONG x, LONG y, bool axis) {
	//for scroll bar movement,  axis 0 = x, axis 1 = y
	//converts square coord's in pixels to hex grid position
	ScrnSqr2Hex(&x, &y);
	LONG xMax = *pNUM_HEX_X - 1;
	LONG yMax = *pNUM_HEX_Y - 1;

	if (axis == 0) {
		if (x > xMax)
			x = xMax, y -= 1;
		else if (x < 0)
			x = 0, y += 1;

		if (y > yMax)
			y = yMax;
		else if (y <= 0)
			y = 0;

	}
	else if (axis == 1) {
		if (x > xMax)
			x = xMax, y += 1;
		else if (x < 0)
			x = 0, y -= 1;

		if (y > yMax)
			y = yMax;
		else if (y <= 0)
			y = 0;

	}
	return y * *pNUM_HEX_X + x;
}


//_____________________________________________________
void GetGameWinCentre(LONG* winCentreX, LONG* winCentreY) {

	WinStruct* gameWin = GetWinStruct(*pWinRef_GameArea);

	*winCentreX = gameWin->width >> 1;///2;
	EDGE_X_DIFF = *winCentreX & 0x1F;
	*winCentreX -= EDGE_X_DIFF;

	*winCentreY = gameWin->height >> 1;///2;
	EDGE_Y_DIFF = *winCentreY % 24;
	*winCentreY -= EDGE_Y_DIFF;
}


//__________________________________________________________________
void GetResLevel(MAPedge* mapEdge, LONG winCentreX, LONG winCentreY) {

	LONG x = 0, y = 0, temp = 0, rem = 0;
	RECT* edge = &mapEdge->sqrEdge;

	LONG tmp1 = 0, tmp2 = 0;

	ScrnHexPos2Sqr(mapEdge->tileEdge.left, &x, &y);
	edge->left = x;
	ScrnHexPos2Sqr(mapEdge->tileEdge.right, &x, &y);
	edge->right = x;
	ScrnHexPos2Sqr(mapEdge->tileEdge.top, &x, &y);
	edge->top = y;
	ScrnHexPos2Sqr(mapEdge->tileEdge.bottom, &x, &y);
	edge->bottom = y;

	WinStruct* win = GetWinStruct(*pWinRef_GameArea);

	mapEdge->visEdge.left = edge->left;
	mapEdge->visEdge.right = edge->right;
	mapEdge->visEdge.top = edge->top;
	mapEdge->visEdge.bottom = edge->bottom;

	LONG winX = win->width / 2 - 1;
	LONG winY = win->height / 2 - 1;
	mapEdge->visEdge.left -= winX;
	mapEdge->visEdge.right -= winX;
	mapEdge->visEdge.top += winY;
	mapEdge->visEdge.bottom += winY;



	temp = edge->left - edge->right;//get distance between west and east boarders
	temp = temp / 2;
	tmp1 = tmp2 = temp;
	if (temp & 0x1F) {//check if not a multiple of 32
		temp = (temp & 0xFFFFFFE0);
		tmp1 = temp;
		tmp2 = temp + 32;
	}

	if (tmp1 >= winCentreX) {
		edge->left -= winCentreX;
		edge->right += winCentreX;
	}
	else {
		edge->left -= tmp1;//temp;//expand boarders beyond screen width
		edge->right += tmp2;//temp;
	}

	temp = edge->bottom - edge->top;//get distance between north and south boarders
	temp = temp / 2;
	tmp1 = tmp2 = temp;
	rem = temp % 24;
	if (rem) {
		temp -= rem;
		tmp1 = temp;
		tmp2 = temp + 24;
	}


	if (tmp1 >= winCentreY) {
		edge->bottom -= winCentreY;
		edge->top += winCentreY;
	}
	else {
		edge->top += tmp2;
		edge->bottom -= tmp1;//expand boarders beyond screen height
	}

	//fix rounding errors when boarders overlap
	if (edge->left < edge->right) edge->left = edge->right;
	if (edge->bottom < edge->top) edge->bottom = edge->top;

	if (edge->left - edge->right == 32) edge->left = edge->right;
	if (edge->bottom - edge->top == 24) edge->bottom = edge->top;

	temp = (edge->left - edge->right) / 2 + edge->right;
	if (temp & 0x1F)//check if not a multiple of 32
		temp = (temp & 0xFFFFFFE0);
	mapEdge->x = temp;

	temp = (edge->bottom - edge->top) / 2 + edge->top;
	rem = temp % 24;
	temp -= rem;
	mapEdge->y = temp;
}


//___________________
void InitilizeEdges() {

	LONG winCentreX = 0;
	LONG winCentreY = 0;
	GetGameWinCentre(&winCentreX, &winCentreY);

	MAPedge* mapEdge;
	for (int lev = 0; lev < 3; lev++) {
		mapEdge = &M_CURRENT_MAP.mapEDGE[lev];
		mapEdge->tileEdge.left = 199;
		mapEdge->tileEdge.right = 39800;
		mapEdge->tileEdge.top = 0;
		mapEdge->tileEdge.bottom = 39999;
		mapEdge->nextArea = nullptr;
		GetResLevel(mapEdge, winCentreX, winCentreY);

		mapEdge->angleEdge.left = 99;
		mapEdge->angleEdge.top = 0;
		mapEdge->angleEdge.right = 0;
		mapEdge->angleEdge.bottom = 99;

		mapEdge->angleFlag = 0;

		mapEdge->nextArea = nullptr;
	}
	edgeInitiated = true;
	isAngledEdges = false;//disable angled edges

	defaultEDGES.tileEdge.left = 199;
	defaultEDGES.tileEdge.right = 39800;
	defaultEDGES.tileEdge.top = 0;
	defaultEDGES.tileEdge.bottom = 39999;
	defaultEDGES.nextArea = nullptr;
	defaultEDGES.prevArea = nullptr;
	GetResLevel(&defaultEDGES, winCentreX, winCentreY);

	defaultEDGES.angleEdge.left = 99;
	defaultEDGES.angleEdge.top = 0;
	defaultEDGES.angleEdge.right = 0;
	defaultEDGES.angleEdge.bottom = 99;

	defaultEDGES.angleFlag = 0;


}


//_____________________________________
void DestroyEdgeAreas(MAPedge* mapEdge) {

	if (mapEdge->nextArea == nullptr)
		return;

	MAPedge* tmpEdge1 = mapEdge;
	MAPedge* tmpEdge2 = nullptr;
	while (tmpEdge1->nextArea) {
		tmpEdge2 = tmpEdge1->nextArea;
		delete tmpEdge1;
		tmpEdge1 = tmpEdge2;
	}
	delete tmpEdge1;
	tmpEdge1 = nullptr;
	tmpEdge2 = nullptr;
	mapEdge->nextArea = nullptr;
}


//______________________________
bool LoadMapEdges(char* MapName) {

	char mapPath[32];
	sprintf(mapPath, "maps\\%s", MapName);
	memcpy(strchr(mapPath, '.'), ".edg\0", 5);

	void* FileStream = F_fopen(mapPath, "rb");
	if (FileStream == nullptr)
		return false;
	DWORD isEdge = 0;
	F_fread32(FileStream, &isEdge);
	if (isEdge != 0x45444745)//check if edge file "EDGE"
		return false;
	DWORD version = 0;
	F_fread32(FileStream, &version);
	if (version != 0x01 && version != 0x02)//check file version 1
		return false;

	if (version == 0x02)//get angled edges for level
		isAngledEdges = true;
	else
		isAngledEdges = false;

	MAPedge* mapEdge;
	LONG currentLev = 0;

	LONG winCentreX = 0;
	LONG winCentreY = 0;
	GetGameWinCentre(&winCentreX, &winCentreY);

	F_fread32(FileStream, (DWORD*)&currentLev);
	if (currentLev)//this should be level zero
		return false;
	for (int lev = 0; lev < 3; lev++) {
		mapEdge = &M_CURRENT_MAP.mapEDGE[lev];
		DestroyEdgeAreas(mapEdge);


		if (isAngledEdges) {//get angled edges for level
			if (F_fread32Array(FileStream, (DWORD*)&mapEdge->angleEdge.left, 4))
				return false;
			if (F_fread32(FileStream, &mapEdge->angleFlag))
				return false;
		}
		else {
			mapEdge->angleEdge.left = 99;
			mapEdge->angleEdge.top = 0;
			mapEdge->angleEdge.right = 0;
			mapEdge->angleEdge.bottom = 99;

			mapEdge->angleFlag = 0;
		}

		while (currentLev == lev) {
			if (F_fread32Array(FileStream, (DWORD*)&mapEdge->tileEdge.left, 4))
				return false;
			GetResLevel(mapEdge, winCentreX, winCentreY);

			if (F_fread32(FileStream, (DWORD*)&currentLev)) {
				if (lev == 2)//exit loop if eof and level 3 done
					currentLev = -1;
				else//read has failed
					return false;
			}

			if (currentLev == lev) {//add new area if same level as last
				mapEdge->nextArea = new MAPedge;
				CopyRect(&mapEdge->nextArea->angleEdge, &mapEdge->angleEdge);//copy angled edges from previous area
				mapEdge = mapEdge->nextArea;
				mapEdge->nextArea = nullptr;
			}


		}
	}
	F_fclose(FileStream);

	return true;
}



//________________________________________________________
bool FogOfWarMap_CopyFiles(char* pFromPath, char* pToPath) {
	if (!FOG_OF_WAR)
		return false;

	char* fromPath = new char[256];
	sprintf(fromPath, "%s", pFromPath);
	memcpy(strchr(fromPath, '.'), ".fog\0", 5);
	char* toPath = new char[256];
	sprintf(toPath, "%s", pToPath);
	memcpy(strchr(toPath, '.'), ".fog\0", 5);


	void* FileStream_From = F_fopen(fromPath, "rb");
	if (FileStream_From == nullptr)
		return false;
	void* FileStream_To = F_fopen(toPath, "wb");
	if (FileStream_To == nullptr) {
		F_fclose(FileStream_From);
		return false;
	}

	DWORD dVal = 0;
	DWORD numDwords = 0;

	F_fread32(FileStream_From, &dVal);
	if (dVal == 0x464F474F) {
		F_fwrite32(FileStream_To, 0x464F474F);//"FOGO"
		F_fread32(FileStream_From, &dVal);
		if (dVal == 0x46574152)
			F_fwrite32(FileStream_To, 0x46574152);//"FWAR"
		else {
			F_fclose(FileStream_From);
			F_fclose(FileStream_To);
			return false;
		}
		F_fread32(FileStream_From, &dVal);//version
		if (dVal == 0x00000001)//version1
			F_fwrite32(FileStream_To, 0x00000001);//version1
		else {
			F_fclose(FileStream_From);
			F_fclose(FileStream_To);
			return false;
		}
	}
	F_fread32(FileStream_From, &dVal);//numBits
	F_fwrite32(FileStream_To, dVal);
	F_fread32(FileStream_From, &numDwords);
	F_fwrite32(FileStream_To, numDwords);
	for (DWORD i = 0; i < numDwords; i++) {
		F_fread32(FileStream_From, &dVal);
		F_fwrite32(FileStream_To, dVal);
	}
	F_fclose(FileStream_From);
	F_fclose(FileStream_To);

	return true;
}


//______________________________________
LONG FogOfWarMap_DeleteTmps(char* path) {
	if (!FOG_OF_WAR)
		return false;
	//return numFiles
	return FDeleteTmpSaveFiles(path, "fog");
}


//__________________________________
bool FogOfWarMap_Save(char* MapName) {
	if (!FOG_OF_WAR)
		return false;

	char mapPath[256];
	sprintf(mapPath, "%s", MapName);
	memcpy(strchr(mapPath, '.'), ".fog\0", 5);

	void* FileStream = F_fopen(mapPath, "wb");
	if (FileStream == nullptr)
		return false;

	if (fogHexMapBits) {
		F_fwrite32(FileStream, 0x464F474F);//"FOGO"
		F_fwrite32(FileStream, 0x46574152);//"FWAR"
		F_fwrite32(FileStream, 0x00000001);//version1
		F_fwrite32(FileStream, fogHexMapBits->numBits);
		F_fwrite32(FileStream, fogHexMapBits->numDwords);
		for (DWORD i = 0; i < fogHexMapBits->numDwords; i++)
			F_fwrite32(FileStream, fogHexMapBits->dwords[i]);
	}

	F_fclose(FileStream);
	///MessageBox(nullptr, mapPath, "Fog Saved",MB_ICONEXCLAMATION | MB_OK);
	return true;
}


//__________________________________
bool FogOfWarMap_Load(char* MapName) {
	if (!FOG_OF_WAR)
		return false;
	isRecordingObjFog = true;
	//reset map hex width in case changed.
	hexStep2[0][1] = -*pNUM_HEX_X;
	hexStep2[1][0] = *pNUM_HEX_X;
	hexStep2[2][0] = *pNUM_HEX_X;
	hexStep2[2][1] = *pNUM_HEX_X;
	hexStep2[3][0] = *pNUM_HEX_X;
	hexStep2[4][1] = -*pNUM_HEX_X;
	hexStep2[5][0] = -*pNUM_HEX_X;
	hexStep2[5][1] = -*pNUM_HEX_X;


	if (fogHexMapBits != nullptr) {
		delete fogHexMapBits;
		fogHexMapBits = nullptr;
	}

	char mapPath[256];
	sprintf(mapPath, "maps\\%s", MapName);
	memcpy(strchr(mapPath, '.'), ".fog\0", 5);
	void* FileStream = F_fopen(mapPath, "rb");
	if (FileStream == nullptr) {
		fogHexMapBits = new bitset(*pNUM_HEXES * 3);
		return false;
	}

	DWORD dVal = 0;
	if (!fogHexMapBits) {
		F_fread32(FileStream, &dVal);//0x464F474F);//"FOGO"
		if (dVal == 0x464F474F) {
			F_fread32(FileStream, &dVal);//0x47574152);//"FWAR"
			if (dVal != 0x46574152) {
				F_fclose(FileStream);
				return false;
			}
			F_fread32(FileStream, &dVal);
			if (dVal != 0x00000001) {//version1
				F_fclose(FileStream);
				return false;
			}
		}
		F_fread32(FileStream, &dVal);//fogHexMapBits->numBits);
		fogHexMapBits = new bitset(dVal);
		F_fread32(FileStream, &dVal);//fogHexMapBits->numDwords);
		for (DWORD i = 0; i < fogHexMapBits->numDwords; i++)
			F_fread32(FileStream, &fogHexMapBits->dwords[i]);
	}

	F_fclose(FileStream);
	return true;
}


//_______________________________
void SetMapBorders(char* MapName) {
	isRecordingObjFog = false;
	if (LoadMapEdges(MapName))
		return;

	InitilizeEdges();
}


//___________________________________
bool isHexWithinMapEdges(LONG hexPos) {

	LONG tmp_y = 0;
	LONG tmp_x = 0;

	//converts hex coord's to screen square coord's in pixels
	ScrnHexPos2Sqr(hexPos, &tmp_x, &tmp_y);

	WinStruct* win = GetWinStruct(*pWinRef_GameArea);
	LONG winX = win->width / 2 - EDGE_OFF_X;
	LONG winY = win->height / 2 - EDGE_OFF_Y;

	RECT* visEdge = &M_CURRENT_MAP.currentEDGES->visEdge;
	if (tmp_x < visEdge->left + winX && tmp_x > visEdge->right + winX && tmp_y > visEdge->top - winY && tmp_y < visEdge->bottom - winY)
		return true;
	else
		return false;
}


//____________________________________________________
LONG CheckMapBlockers(LONG hexPos, LONG current_level) {

	LONG xMax = *pNUM_HEX_X;
	LONG tmp_y = hexPos / xMax;
	LONG tmp_x = hexPos % xMax;

	ScrnHex2Sqr(&tmp_x, &tmp_y);//converts hex coord's to screen square coord's in pixels

	RECT* sqrEdge = &M_CURRENT_MAP.currentEDGES->sqrEdge;
	if (tmp_x > sqrEdge->left || tmp_x < sqrEdge->right) return 0;
	else if (tmp_y > sqrEdge->bottom || tmp_y < sqrEdge->top) return 0;

	LONG tmpEdgeX = EDGE_OFF_X;
	LONG tmpEdgeY = EDGE_OFF_Y;
	EDGE_OFF_X = 0;
	EDGE_OFF_Y = 0;

	if (tmp_x == sqrEdge->left) EDGE_OFF_X = -EDGE_X_DIFF;
	else if (tmp_x == sqrEdge->right) EDGE_OFF_X = EDGE_X_DIFF;/// set rect offset here-100;

	if (tmp_y == sqrEdge->top) EDGE_OFF_Y = -EDGE_Y_DIFF;
	else if (tmp_y == sqrEdge->bottom) EDGE_OFF_Y = EDGE_Y_DIFF;

	if (tmpEdgeX != EDGE_OFF_X || tmpEdgeY != EDGE_OFF_Y)
		return 1;

	return -1;
}


//_________________________________________________________
LONG SetViewStartHex(LONG original_pos, LONG current_level) {

	if (!edgeInitiated)
		InitilizeEdges();

	LONG new_pos = original_pos;

	LONG original_x = 0, iso_x = 0, sqr_x = 0;
	LONG original_y = 0, iso_y = 0, sqr_y = 0;

	ScrnHexPos2Sqr(original_pos, &original_x, &original_y);

	MAPedge* current_edges = &M_CURRENT_MAP.mapEDGE[current_level];
	M_CURRENT_MAP.currentEDGES = &M_CURRENT_MAP.mapEDGE[current_level];

	WinStruct* win = GetWinStruct(*pWinRef_GameArea);
	memset(win->buff, '\0', win->width * win->height);//clear win buff

	if (current_edges->nextArea) {//check if other edges defined for this level

		LONG winX = win->width / 2 - 1;
		LONG winY = win->height / 2 + 1;
		MAPedge* tmpEdge = current_edges;//->nextArea;
		while (tmpEdge) {//match edge data with start position
			if (original_x < tmpEdge->visEdge.left + winX && original_x > tmpEdge->visEdge.right + winX && original_y > tmpEdge->visEdge.top - winY && original_y < tmpEdge->visEdge.bottom - winY) {
				current_edges = tmpEdge;
				M_CURRENT_MAP.currentEDGES = current_edges;
				tmpEdge = nullptr;
			}
			else
				tmpEdge = tmpEdge->nextArea;
		}
	}

	if (original_x > current_edges->sqrEdge.left)
		current_edges->x = current_edges->sqrEdge.left;
	else if (original_x < current_edges->sqrEdge.right)
		current_edges->x = current_edges->sqrEdge.right;
	else
		current_edges->x = original_x;


	if (original_y > current_edges->sqrEdge.bottom)
		current_edges->y = current_edges->sqrEdge.bottom;
	else if (original_y < current_edges->sqrEdge.top)
		current_edges->y = current_edges->sqrEdge.top;
	else
		current_edges->y = original_y;


	iso_x = current_edges->x;
	iso_y = current_edges->y;
	ScrnSqr2Hex(&iso_x, &iso_y);//converts square coord's in pixels to isometric coord's
	new_pos = iso_y * *pNUM_HEX_X + iso_x;//store current test position

	EDGE_OFF_X = 0;
	EDGE_OFF_Y = 0;

	if (current_edges->x == current_edges->sqrEdge.left) EDGE_OFF_X = -EDGE_X_DIFF;
	else if (current_edges->x == current_edges->sqrEdge.right) EDGE_OFF_X = +EDGE_X_DIFF;

	if (current_edges->y == current_edges->sqrEdge.top) EDGE_OFF_Y = -EDGE_Y_DIFF;
	else if (current_edges->y == current_edges->sqrEdge.bottom) EDGE_OFF_Y = EDGE_Y_DIFF;

	return new_pos;
}


//__________________
void ReDrawViewWin() {
	if (*pWinRef_GameArea == -1)
		return;
	WinStruct* win = GetWinStruct(*pWinRef_GameArea);
	if (win && *pDRAW_VIEW_FLAG)
		F_DrawMapArea(&win->rect, *pMAP_LEVEL);
}


//_______________________________________
LONG SetViewPos(LONG hexPos, DWORD flags) {

	if (hexPos < 0 || hexPos >= *pNUM_HEXES)
		return -1;

	if (flags)
		hexPos = SetViewStartHex(hexPos, *pMAP_LEVEL);

	if (!(flags & 0x2) && *pPC_SCROLL_LIMIT_FLAG) {
		LONG scrnX = 0, scrnY = 0;
		GetHexSqrXY(hexPos, &scrnX, &scrnY);
		LONG pcX = 0, pcY = 0;
		OBJStruct* pObj_PC = *lpObj_PC;
		GetHexSqrXY(pObj_PC->hexNum, &pcX, &pcY);
		LONG xDiff = abs(scrnX - pcX) << 4;//*16;
		LONG yDiff = abs(scrnY - pcY) * 12;

		if (xDiff >= SCROLL_DIST_X || yDiff >= SCROLL_DIST_Y) {
			LONG scrnOldX = 0, scrnOldY = 0;
			GetHexSqrXY(*pVIEW_HEXPOS, &scrnOldX, &scrnOldY);//fix for scolling back to out of bounds pc.
			LONG xOldDiff = abs(scrnOldX - pcX) << 4;//*16;
			LONG yOldDiff = abs(scrnOldY - pcY) * 12;
			if (xOldDiff < xDiff || yOldDiff < yDiff)
				return -1;
		}
	}

	if (!(flags & 0x2) && (*pSCROLL_BLOCK_FLAG)) {
		LONG edgeCheck = CheckMapBlockers(hexPos, *pMAP_LEVEL);
		if (edgeCheck == 1)
			flags = flags | 1;//redraw
		else if (edgeCheck == 0)
			return -1;//exit edge hit
	}

	LONG xMax = *pNUM_HEX_X;
	LONG hexY = hexPos / xMax;
	LONG rem = hexPos % xMax;
	LONG hexX = xMax - 1 - rem;

	WinStruct* win = GetWinStruct(*pWinRef_GameArea);

	*pVIEW_SQU_HEX_X = ((win->width - 32) >> 1) + EDGE_OFF_X;
	*pVIEW_SQU_HEX_Y = ((win->height - 16) >> 1) + EDGE_OFF_Y;

	*pVIEW_HEX_X = hexX;
	*pVIEW_HEX_Y = hexY;


	if ((*pVIEW_HEX_X & 0x1)) {
		*pVIEW_HEX_X -= 1;
		*pVIEW_SQU_HEX_X -= 32;
	}

	*pVIEW_TILE_X = *pVIEW_HEX_X >> 1;///2;
	*pVIEW_TILE_Y = *pVIEW_HEX_Y >> 1;///2;

	*pVIEW_SQU_TILE_X = *pVIEW_SQU_HEX_X - 16;
	*pVIEW_SQU_TILE_Y = *pVIEW_SQU_HEX_Y - 2;

	if ((*pVIEW_HEX_Y & 0x1)) {
		*pVIEW_SQU_TILE_Y -= 12;
		*pVIEW_SQU_TILE_X -= 16;
	}

	*pVIEW_HEXPOS = hexPos;

#if defined(DX9_ENHANCED)
	ResetZoomLevel();
#endif

	if ((flags & 0x1) && *pDRAW_VIEW_FLAG) {
		F_DrawMapArea(&win->rect, *pMAP_LEVEL);
		return -1;//added for edge check -may cause probs
	}
	return 0;
}


//__________________________________________________________________________________________________________
void DrawDialogView(BYTE* frmBuff, LONG subWidth, LONG subHeight, LONG frmWidth, BYTE* toBuff, LONG toWidth) {

	WinStruct* GameWin = GetWinStruct(*pWinRef_GameArea);
	WinStruct* DialogWin = GetWinStruct(*pWinRef_DialogMain);

	//get centre pos
	LONG viewCentrePos = *pVIEW_HEXPOS;
	LONG view_x = 0, view_y = 0;
	GetHexSqrXY(viewCentrePos, &view_x, &view_y);
	//get char pos
	OBJStruct* pObj_DialogFocus = *lpObj_DialogFocus;
	LONG char_x = 0, char_y = 0;
	GetHexSqrXY(pObj_DialogFocus->hexNum, &char_x, &char_y);

	LONG dialogX = (view_x - char_x) << 4;//*16;
	LONG dialogY = (char_y - view_y) * 12;

	//sub half obj frm height
	DWORD focusFrmObj = 0;
	FRMhead* focusFrm = F_GetFrm(pObj_DialogFocus->frmID, &focusFrmObj);
	dialogY -= F_GetFrmFrameHeight(focusFrm, pObj_DialogFocus->frameNum, pObj_DialogFocus->ori) / 2;
	F_UnloadFrm(focusFrmObj);

	//set y position of view window
	dialogY += (GameWin->height - subHeight) >> 1;//GameWin->height/2 - subHeight/2;
	//make sure y is within view area
	if (dialogY < 0)
		dialogY = 0;
	else if (dialogY > GameWin->height - subHeight)
		dialogY = GameWin->height - subHeight;

	//set x position of view window
	dialogX += (GameWin->width - subWidth) >> 1;//GameWin->width/2 - subWidth/2;
	//make sure x is within view area
	if (dialogX < 0)
		dialogX = 0;
	else if (dialogX > GameWin->width - subWidth)
		dialogX = GameWin->width - subWidth;


	MemBlt8(GameWin->buff + dialogX + dialogY * GameWin->width, subWidth, subHeight, GameWin->width, toBuff, DialogWin->width);

	GameWin = nullptr;
	DialogWin = nullptr;
}


//______________________________________________________
LONG MergeRects(RECT* rect1, RECT* rect2, RECT* newRect) {

	CopyRect(newRect, rect1);

	if (rect1->left > rect2->right)
		return -1;
	if (rect2->left > rect1->right)
		return -1;

	if (rect2->bottom < rect1->top)
		return -1;
	if (rect2->top > rect1->bottom)
		return -1;

	if (rect2->left > rect1->left)
		newRect->left = rect2->left;
	if (rect2->right < rect1->right)
		newRect->right = rect2->right;

	if (rect2->top > rect1->top)
		newRect->top = rect2->top;
	if (rect2->bottom < rect1->bottom)
		newRect->bottom = rect2->bottom;

	return 0;
}


//____________________________
bool CheckDrawRect(RECT* rect) {

	LONG viewX = 0, viewY = 0;
	ScrnHexPos2Sqr(*pVIEW_HEXPOS, &viewX, &viewY);

	WinStruct* win = GetWinStruct(*pWinRef_GameArea);
	viewX += (win->width >> 1);
	viewY -= (win->height >> 1);

	RECT vRect = { viewX - rect->left,
				  viewY + rect->top,
				  viewX - rect->right,
				  viewY + rect->bottom };

	LONG xMax = *pNUM_HEX_X;
	LONG yMax = *pNUM_HEX_Y;

	LONG xPos = vRect.left;
	LONG yPos = vRect.top;
	ScrnSqr2Hex(&xPos, &yPos);
	if (xPos < 0 || xPos >= xMax || yPos < 0 || yPos >= yMax)
		return true;

	xPos = vRect.right;
	yPos = vRect.top;
	ScrnSqr2Hex(&xPos, &yPos);
	if (xPos < 0 || xPos >= xMax || yPos < 0 || yPos >= yMax)
		return true;

	xPos = vRect.left;
	yPos = vRect.bottom;
	ScrnSqr2Hex(&xPos, &yPos);
	if (xPos < 0 || xPos >= xMax || yPos < 0 || yPos >= yMax)
		return true;

	xPos = vRect.right;
	yPos = vRect.bottom;
	ScrnSqr2Hex(&xPos, &yPos);
	if (xPos < 0 || xPos >= xMax || yPos < 0 || yPos >= yMax)
		return true;

	return false;
}


//______________________________________________________
LONG ClipEdgeRect(RECT* rect1, RECT* rect2, RECT* rect3) {

	if (MergeRects(rect1, rect2, rect3) == -1)
		return -1;

	LONG viewX = 0, viewY = 0;
	ScrnHexPos2Sqr(*pVIEW_HEXPOS, &viewX, &viewY);

	viewX += EDGE_OFF_X;
	viewY -= EDGE_OFF_Y;
	edgeRect.left = viewX - M_CURRENT_MAP.currentEDGES->visEdge.left;
	edgeRect.top = M_CURRENT_MAP.currentEDGES->visEdge.top - viewY;
	edgeRect.right = viewX - M_CURRENT_MAP.currentEDGES->visEdge.right;
	edgeRect.bottom = M_CURRENT_MAP.currentEDGES->visEdge.bottom - viewY;

	if (!EDGE_CLIPPING_ON)
		return 0;

	if (CheckDrawRect(rect3)) {
		WinStruct* win = GetWinStruct(*pWinRef_GameArea);
		BYTE* buff = win->buff;
		buff += rect3->top * win->width;
		buff += rect3->left;
		LONG height = rect3->bottom - rect3->top + 1;
		LONG width = rect3->right - rect3->left + 1;
		for (LONG y = 0; y < height; y++) {
			memset(buff, '\0', width);
			buff += win->width;
		}
	}

	return MergeRects(rect3, &edgeRect, rect3);
}


//__________________________________________________________________________
void GetTileXY(LONG scrnX, LONG scrnY, LONG level, LONG* tileX, LONG* tileY) {

	__asm {
		PUSH ESI                                //GET_TILE_COOR(EAX scrnX, EDX scrnY, EBX level, ECX *tileX, Arg1 *tileY)
		PUSH EDI
		//PUSH EBP
		///MOV EBX,tileY
		MOV ECX, tileX
		MOV EDI, scrnX
		MOV ESI, scrnY
		MOV EAX, pVIEW_SQU_TILE_Y
		MOV EBX, DWORD PTR DS : [EAX]            // tileViewYPos
		MOV EAX, pVIEW_SQU_TILE_X
		MOV EDX, DWORD PTR DS : [EAX]            // tileViewXPos
		SUB ESI, EBX                              // scrnY-tileViewYPos
		SUB EDI, EDX                              // scrnX-tileViewXPos
		SUB ESI, 0xC                              // scrnY-12
		LEA EDX, [EDI * 0x4]
		LEA EAX, [ESI * 0x4]
		SUB EDX, EDI
		SUB EDX, EAX
		MOV DWORD PTR DS : [ECX] , EDX               // tileX = ((scrnX-tileViewXPos)*3)-((scrnY-tileViewYPos -12)*4)
		TEST EDX, EDX                             // if(tileX>=0)
		JGE J0x4B1FDA
		INC EDX                                  // tileX++
		MOV EBX, 0xC0
		MOV EAX, EDX
		SAR EDX, 0x1F
		IDIV EBX                                 // tileX=tileX/192
		DEC EAX                                  // tileX--
		JMP J0x4B1FE6
		J0x4B1FDA :
		MOV EBX, 0xC0
		MOV EAX, EDX
		SAR EDX, 0x1F
		IDIV EBX                                 // tileX=tileX/192
		J0x4B1FE6 :
		MOV DWORD PTR DS : [ECX] , EAX               // store tileX
		SHL ESI, 0x2                                // (scrnY-tileViewYPos -12)*4
		ADD EDI, ESI
		MOV EBX, tileY
		MOV DWORD PTR DS : [EBX] , EDI               // tileY = ((scrnX-tileViewXPos)*4) + ((scrnY-tileViewYPos -12)*4)
		TEST EDI, EDI                             // if(tileY>=0)
		JGE J0x4B2006
		LEA EDX, [EDI + 0x1]                          // tileY++
		MOV EAX, EDX
		SAR EDX, 0x1F
		SHL EDX, 0x7
		SBB EAX, EDX
		SAR EAX, 0x7                                // tileY=tileY/128
		DEC EAX                                  // tileY--
		JMP J0x4B2015
		J0x4B2006 :
		MOV EDX, EDI
		MOV EAX, EDI
		SAR EDX, 0x1F
		SHL EDX, 0x7
		SBB EAX, EDX
		SAR EAX, 0x7                                // tileY=tileY/128
		J0x4B2015:
		MOV EBX, tileY
		MOV DWORD PTR DS : [EBX] , EAX               // store tileY
		MOV EAX, pVIEW_TILE_X
		MOV EAX, DWORD PTR DS : [EAX]            // currentTilePosX
		ADD DWORD PTR DS : [ECX] , EAX               // tileX=tileX+currentTilePosX
		MOV EAX, pVIEW_TILE_Y
		MOV EAX, DWORD PTR DS : [EAX]            // currentTilePosY
		MOV EDI, DWORD PTR DS : [EBX]
		ADD EDI, EAX
		MOV EAX, pNUM_TILE_X
		MOV EAX, DWORD PTR DS : [EAX]            // numTilesMapX(100)
		MOV DWORD PTR DS : [EBX] , EDI               // tileY=tileY+currentTilePosY
		DEC EAX
		MOV EBX, DWORD PTR DS : [ECX]
		SUB EAX, EBX
		MOV DWORD PTR DS : [ECX] , EAX               // tileX = numTilesX-1 - tileX
		//POP EBP
		POP EDI
		POP ESI
	}

}


//_________________________________________________________________________
LONG GetScrnXYTile(LONG tileNum, LONG* scrnX, LONG* scrnY, LONG level) {

	LONG retVal = 0;
	__asm {
		PUSH ESI                                 // int GET_SCRN_COOR_TILE(EAX tileOffset, EDX *scrnX, EBX *scrnY, ECX level)
		PUSH EDI
		///PUSH EBP
		MOV EAX, tileNum
		MOV ESI, EAX
		///MOV EDI,scrnX
		///MOV EBX, scrnY
		TEST EAX, EAX
		JL J0x4B1DD3
		MOV EDX, pNUM_TILES
		CMP EAX, DWORD PTR DS : [EDX]            // numTiles
		JL J0x4B1DDC
		J0x4B1DD3 :
		MOV EAX, -1
		JMP endFunc
		//POP EBP
		//POP EDI
		//POP ESI
		//RETN
		J0x4B1DDC:
		MOV EDX, EAX
		MOV ECX, pNUM_TILE_X
		MOV ECX, DWORD PTR DS : [ECX]            // numTilesMapX(100)
		SAR EDX, 0x1F
		IDIV ECX
		DEC ECX
		MOV EAX, ESI
		SUB ECX, EDX
		MOV EDX, ESI
		MOV ESI, pNUM_TILE_X
		MOV ESI, DWORD PTR DS : [ESI]            // numTilesMapX(100)
		SAR EDX, 0x1F
		IDIV ESI
		MOV EDX, EAX
		MOV EAX, pVIEW_SQU_TILE_X
		MOV EAX, DWORD PTR DS : [EAX]            // tileViewXPos
		MOV EBX, pVIEW_TILE_X
		MOV EBX, DWORD PTR DS : [EBX]            // currentTilePosX
		MOV EDI, scrnX
		MOV DWORD PTR DS : [EDI] , EAX
		MOV EAX, pVIEW_SQU_TILE_Y
		MOV EAX, DWORD PTR DS : [EAX]            // tileViewYPos
		SUB ECX, EBX
		MOV EBX, scrnY
		MOV DWORD PTR DS : [EBX] , EAX
		MOV EAX, ECX
		LEA ESI, [ECX * 0x4]
		SUB ESI, ECX
		SHL ESI, 0x4
		MOV ECX, DWORD PTR DS : [EDI]
		ADD ECX, ESI
		MOV ESI, EAX
		MOV DWORD PTR DS : [EDI] , ECX
		SHL ESI, 0x2
		MOV EDI, DWORD PTR DS : [EBX]
		SUB ESI, EAX
		MOV EAX, pVIEW_TILE_Y
		MOV EAX, DWORD PTR DS : [EAX]            // currentTilePosY
		SHL ESI, 0x2
		SUB EDX, EAX
		SUB EDI, ESI
		MOV EAX, EDX
		MOV DWORD PTR DS : [EBX] , EDI
		SHL EDX, 0x5
		MOV ESI, EAX
		MOV EDI, scrnX
		MOV ECX, DWORD PTR DS : [EDI]
		SHL ESI, 0x2
		ADD ECX, EDX
		SUB ESI, EAX
		MOV DWORD PTR DS : [EDI] , ECX
		SHL ESI, 0x3
		MOV EDI, DWORD PTR DS : [EBX]
		ADD EDI, ESI
		XOR EAX, EAX
		MOV DWORD PTR DS : [EBX] , EDI
		endFunc :
		///POP EBP
		POP EDI
		POP ESI
		mov retVal, EAX
	}
	return retVal;
}


//_____________________________________________________________
LONG GetFloorHexLight(LONG elev, LONG hexNum, LONG globalLight) {
	if (elev < 0 || elev >= 3)
		return 0;
	if (hexNum < 0 || hexNum >= *pNUM_HEXES)
		return 0;

	LONG elevOffset = elev * *pNUM_HEXES;
	LONG light = pLightHexArray[elevOffset + hexNum];
	if (light < globalLight)
		light = globalLight;

	if (FOG_OF_WAR && light > fogLight && fogLight != 0) {
		OBJStruct* pObj_PC = *lpObj_PC;
		if (lpObj_PC && pObj_PC->hexNum != -1 && fogHexMapBits) {
			if (fogHexMapBits->get(elevOffset + hexNum) == 0 && IsInLineOfSightBlocked(pObj_PC->hexNum, hexNum))
				return fogLight;
		}
	}

	if (light > 0x10000)
		return 0x10000;
	else
		return light;
}


//_______________________________________________________
DWORD CheckAngledTileEdge(int x, int y, DWORD tileLstNum) {

	if (isAngledEdges) {
		MAPedge* currentEdges = M_CURRENT_MAP.currentEDGES;
		if (x > currentEdges->angleEdge.left || x < currentEdges->angleEdge.right || y > currentEdges->angleEdge.bottom || y < currentEdges->angleEdge.top)
			tileLstNum = 1;
	}

	return F_GetFrmID(ART_TILES, tileLstNum, 0, 0, 0);
}


//_________________________________________
void DrawFloorTiles(RECT* rect, LONG level) {

	RECT rcTileGrid{ 0,0,0,0 };
	LONG tempVal = 0;
	GetTileXY(rect->left, rect->top, level, &tempVal, &rcTileGrid.top);
	GetTileXY(rect->right, rect->top, level, &rcTileGrid.right, &tempVal);
	GetTileXY(rect->left, rect->bottom, level, &rcTileGrid.left, &tempVal);
	GetTileXY(rect->right, rect->bottom, level, &tempVal, &rcTileGrid.bottom);

	if (rcTileGrid.right < 0)
		rcTileGrid.right = 0;
	else if (rcTileGrid.right >= *pNUM_TILE_X)
		rcTileGrid.right = *pNUM_TILE_X - 1;
	if (rcTileGrid.top < 0)
		rcTileGrid.top = 0;
	else if (rcTileGrid.top >= *pNUM_TILE_Y)
		rcTileGrid.top = *pNUM_TILE_Y - 1;

	if (rcTileGrid.left > *pNUM_TILE_X)
		rcTileGrid.left = *pNUM_TILE_X;
	if (rcTileGrid.bottom > *pNUM_TILE_Y)
		rcTileGrid.bottom = *pNUM_TILE_Y;

	if (rcTileGrid.bottom < rcTileGrid.top)
		return;

	DWORD** levelOffset = *pMapTileLevelOffset;
	LONG tileLstNum = 0;
	LONG tilePosNum = 0;
	LONG tileFlag = 0;
	LONG scrnX = 0, scrnY = 0;
	DWORD fID = 0;
	LONG yOff = rcTileGrid.top * *pNUM_TILE_X;
	for (LONG yPos = rcTileGrid.top; yPos <= rcTileGrid.bottom; yPos++) {
		if (rcTileGrid.left >= rcTileGrid.right) {
			for (LONG xPos = rcTileGrid.right; xPos <= rcTileGrid.left; xPos++) {
				tilePosNum = yOff + xPos;
				if (tilePosNum >= 0 && tilePosNum < *pNUM_TILES) {
					tileLstNum = levelOffset[level][tilePosNum];
					tileFlag = (tileLstNum & 0x0000F000) >> 12;
					tileLstNum = tileLstNum & 0x00000FFF;

					if (!(tileFlag & 0x1)) {
						GetScrnXYTile(tilePosNum, &scrnX, &scrnY, level);
						fID = CheckAngledTileEdge(xPos, yPos, tileLstNum);
						F_DrawFloorTile(fID, scrnX, scrnY, rect);
					}
					///else MessageBox(nullptr, "tile flag set", "Hi-Res Patch Error",MB_ICONEXCLAMATION | MB_OK);
				}
			}
		}
		yOff += *pNUM_TILE_X;
	}
}


//________________________________________________
void CheckAngledObjEdge(RECT* rect, DWORD isUpper) {

	RECT rcTileGrid{ 0,0,0,0 };
	LONG tempVal = 0;
	GetTileXY(rect->left, rect->top, 0, &tempVal, &rcTileGrid.top);
	GetTileXY(rect->right, rect->top, 0, &rcTileGrid.right, &tempVal);
	GetTileXY(rect->left, rect->bottom, 0, &rcTileGrid.left, &tempVal);
	GetTileXY(rect->right, rect->bottom, 0, &tempVal, &rcTileGrid.bottom);

	rcTileGrid.left += 1;
	rcTileGrid.top -= 1;
	rcTileGrid.right -= 1;
	rcTileGrid.bottom += 1;

	LONG xMax = *pNUM_TILE_X;
	LONG yMax = *pNUM_TILE_Y;

	if (rcTileGrid.right < 0)
		rcTileGrid.right = 0;
	else if (rcTileGrid.right >= xMax)
		rcTileGrid.right = xMax - 1;
	if (rcTileGrid.top < 0)
		rcTileGrid.top = 0;
	else if (rcTileGrid.top >= yMax)
		rcTileGrid.top = yMax - 1;

	if (rcTileGrid.left > xMax)
		rcTileGrid.left = xMax;
	if (rcTileGrid.bottom > yMax)
		rcTileGrid.bottom = yMax;

	if (rcTileGrid.bottom < rcTileGrid.top)
		return;

	LONG tileLstNum = 0;
	LONG tilePosNum = 0;
	LONG scrnX = 0, scrnY = 0;
	RECT* prcEdge = &M_CURRENT_MAP.currentEDGES->angleEdge;
	DWORD* pFlags = &M_CURRENT_MAP.currentEDGES->angleFlag;
	LONG yOff = rcTileGrid.top * xMax;
	for (LONG yPos = rcTileGrid.top; yPos <= rcTileGrid.bottom; yPos++) {
		if (rcTileGrid.left >= rcTileGrid.right) {
			for (LONG xPos = rcTileGrid.right; xPos <= rcTileGrid.left; xPos++) {
				tilePosNum = yOff + xPos;
				if (xPos > prcEdge->left && (*pFlags & 0x01000000) >> 24 == isUpper
					|| yPos < prcEdge->top && (*pFlags & 0x00010000) >> 16 == isUpper
					|| xPos < prcEdge->right && (*pFlags & 0x00000100) >> 8 == isUpper
					|| yPos > prcEdge->bottom && (*pFlags & 0x00000001) == isUpper) {
					GetScrnXYTile(tilePosNum, &scrnX, &scrnY, 0);
					F_DrawFloorTile(0x04000001, scrnX, scrnY, rect);
				}
			}
		}
		yOff += xMax;
	}
}


//___________________________________________________________________
DWORD CheckAngledRoofTileEdge(LONG xPos, LONG yPos, DWORD tileLstNum) {

	if (yPos < 0 || yPos >= *pNUM_TILE_Y || xPos < 0 || xPos >= *pNUM_TILE_X)//check if tile pos valid
		return -1;

	if (isAngledEdges) {
		RECT* prcEdge = &M_CURRENT_MAP.currentEDGES->angleEdge;
		if (xPos > prcEdge->left + 2 || xPos < prcEdge->right + 2 || yPos > prcEdge->bottom + 3 || yPos < prcEdge->top + 3)
			return -1;
	}
	return F_GetFrmID(ART_TILES, tileLstNum, 0, 0, 0);
}


//_______________
bool ReSizeMaps() {
	char temp_chars[256];

	ConfigReadString("MAPS", "SCROLL_DIST_X", "480", temp_chars, 255);
	if (strncmp(temp_chars, "HALF_SCRN", 9) == 0)
		SCROLL_DIST_X = SCR_WIDTH / 2 + 32;
	else
		SCROLL_DIST_X = atoi(temp_chars);
	if (SCROLL_DIST_X < 480)SCROLL_DIST_X = 480;

	ConfigReadString("MAPS", "SCROLL_DIST_Y", "400", temp_chars, 255);
	if (strncmp(temp_chars, "HALF_SCRN", 9) == 0)
		SCROLL_DIST_Y = SCR_HEIGHT / 2 + 24;
	else
		SCROLL_DIST_Y = atoi(temp_chars);
	if (SCROLL_DIST_Y < 400)SCROLL_DIST_Y = 400;

	if (!edgeInitiated)
		return false;

	if (*pWinRef_GameArea == -1)
		return false;
	WinStruct* gameWin = GetWinStruct(*pWinRef_GameArea);
	if (gameWin == nullptr)
		return false;

	LONG viewHexPos = *pVIEW_HEXPOS;

	LONG scrollFlag = *pSCROLL_BLOCK_FLAG;
	*pSCROLL_BLOCK_FLAG = 0;

	LONG winCentreX = 0;
	LONG winCentreY = 0;
	GetGameWinCentre(&winCentreX, &winCentreY);

	MAPedge* mapEdge;
	for (int lev = 0; lev < 3; lev++) {
		mapEdge = &M_CURRENT_MAP.mapEDGE[lev];
		GetResLevel(mapEdge, winCentreX, winCentreY);


		while (mapEdge->nextArea != nullptr) {
			mapEdge = mapEdge->nextArea;
			GetResLevel(mapEdge, winCentreX, winCentreY);
		}
	}

	//tiles
	*lpGameWinBuff = gameWin->buff;
	*lpViewWinBuff = gameWin->buff;
	*pViewWinWidth1 = gameWin->width;
	*pViewWinHeight = gameWin->height;
	*pViewWinWidth2 = gameWin->width;

	CopyRect(prcGameWin, &gameWin->rect);
	CopyRect(prcViewWin, &gameWin->rect);

	*lpObjViewTextBuff = gameWin->buff;
	*pObjViewTextWidth = gameWin->width;
	*pObjViewTextHeight = gameWin->height;

	*pViewWin2Width1 = gameWin->width;
	*pViewWin2Height = gameWin->height;
	*pViewWin2Width2 = gameWin->width;
	*lpViewWin2Buff = gameWin->buff;
	CopyRect(prcViewWin2, &gameWin->rect);
	*pViewWin2Size = gameWin->width * gameWin->height;

	*pSCROLL_BLOCK_FLAG = scrollFlag;

	SetViewPos(viewHexPos, 0x1);
	ShowWin(*pWinRef_GameArea);
	return true;
}


//____________________________________
bool CheckHexTransparency(LONG hexNum) {
	if (hexNum < 0 || hexNum >= *pNUM_HEXES)
		return true;

	DWORD flags = FLG_LightThru | FLG_ShootThru | FLG_TransNone;//|FLG_Flat;

	OBJNode* objNode = 0;
	LONG objType = 0;

	objNode = pMapObjNodeArray[hexNum];
	objType = 0;

	if (fogHexMapBits && !fogHexMapBits->get(*pMAP_LEVEL * *pNUM_HEXES + hexNum))
		fogHexMapBits->set(*pMAP_LEVEL * *pNUM_HEXES + hexNum);

	while (objNode) {
		if (objNode->obj->level <= *pMAP_LEVEL) {
			if (objNode->obj->level == *pMAP_LEVEL) {
				objType = (objNode->obj->frmID & 0x0F000000) >> 0x18;

				if (objType == ART_WALLS && !(objNode->obj->flags & flags) || objType == ART_SCENERY && !(objNode->obj->flags & flags))// && !(objNode->obj->flags & OBJFLAG_TransNone) && !(objNode->obj->flags & OBJFLAG_Flat))
					return true;
			}
			objNode = objNode->next;
		}
		else
			objNode = nullptr;
	}

	return false;
}


//__________________________________________
bool GetHexXY(LONG hexPos, LONG* x, LONG* y) {
	if (hexPos < 0 || hexPos >= *pNUM_HEXES)
		return false;

	*y = hexPos / *pNUM_HEX_X;
	*x = hexPos % *pNUM_HEX_X;

	return true;
}


//_____________________________________________________
bool IsInLineOfSightBlocked(LONG hexStart, LONG hexEnd) {

	LONG hexCurrent = hexStart;
	LONG ori = 0;

	LONG hexStartX = 0, hexStartY = 0;
	GetHexXY(hexStart, &hexStartX, &hexStartY);
	LONG hexEndX = 0, hexEndY = 0;
	GetHexXY(hexEnd, &hexEndX, &hexEndY);
	LONG hexCurrentX = 0, hexCurrentY = 0;
	GetHexXY(hexCurrent, &hexCurrentX, &hexCurrentY);

	LONG squStartX = 0, squStartY = 0;
	GetHexSqrXY(hexStart, &squStartX, &squStartY);
	LONG squEndX = 0, squEndY = 0;
	GetHexSqrXY(hexEnd, &squEndX, &squEndY);
	LONG squCurrentX = 0, squCurrentY = 0;
	GetHexSqrXY(hexCurrent, &squCurrentX, &squCurrentY);

	LONG hexesW = *pNUM_HEX_X;

	if (hexCurrentX == hexEndX) {//hex grid up and down (diagonal right/down to left/up)
		while (hexCurrent != hexEnd) {
			if (hexCurrentY < hexEndY)
				hexCurrent += hexesW;
			else
				hexCurrent -= hexesW;
			if (CheckHexTransparency(hexCurrent))
				return true;
		}
	}
	else if (hexCurrentY == hexEndY) {//hex grid left and right
		while (hexCurrent != hexEnd) {
			if (hexCurrentX < hexEndX)
				hexCurrent += 1;
			else
				hexCurrent -= 1;
			if (CheckHexTransparency(hexCurrent))
				return true;
		}
	}
	else if (squCurrentY == squEndY) {//square left and right
		while (hexCurrent != hexEnd) {
			if (squCurrentX > squEndX) {
				if (hexCurrent & 0x1)
					hexCurrent -= 1;
				else
					hexCurrent += hexesW - 1;
			}
			else {
				if (hexCurrent & 0x1)
					hexCurrent -= hexesW - 1;
				else
					hexCurrent += 1;
			}
			if (CheckHexTransparency(hexCurrent))
				return true;
		}

	}
	else if (squCurrentX == squEndX) {//square up and down
		while (hexCurrent != hexEnd) {
			if (squCurrentY < squEndY) {
				if (hexCurrent & 0x1) {
					if (CheckHexTransparency(hexCurrent + hexesW) && CheckHexTransparency(hexCurrent + 1))
						return true;
					hexCurrent += hexesW + 1;
				}
				else {
					if (CheckHexTransparency(hexCurrent + hexesW) && CheckHexTransparency(hexCurrent + hexesW + 1))
						return true;
					hexCurrent += hexesW + hexesW + 1;
				}
			}
			else {
				if (hexCurrent & 0x1) {
					if (CheckHexTransparency(hexCurrent - hexesW) && CheckHexTransparency(hexCurrent - hexesW - 1))
						return true;
					hexCurrent -= (hexesW + hexesW + 1);
				}
				else {
					if (CheckHexTransparency(hexCurrent - hexesW) && CheckHexTransparency(hexCurrent - 1))
						return true;
					hexCurrent -= (hexesW + 1);
				}
			}
			if (CheckHexTransparency(hexCurrent))
				return true;
		}
	}

	else if ((squEndY - squCurrentY) - (squEndX - squCurrentX) == 0) {//diagonal left/down to right/up
		while (hexCurrent != hexEnd) {
			if (squCurrentX < squEndX) {
				if (hexCurrent & 0x1)
					hexCurrent += 1;
				else
					hexCurrent += hexesW + 1;
			}
			else {
				if (hexCurrent & 0x1)
					hexCurrent -= (hexesW + 1);
				else
					hexCurrent -= 1;
			}
			if (CheckHexTransparency(hexCurrent))
				return true;
		}
	}
	else {
		LONG distY = abs(squEndY - squCurrentY);
		LONG distX = abs(squEndX - squCurrentX);
		LONG distY2 = 0;//distY*distY;
		LONG distX2 = 0;//distX*distY;
		LONG error = 0;

		if (distY <= distX) {
			distY2 = distY * distY;
			distX2 = distX * distY;
			LONG xPos = 0;
			if (squCurrentY < squEndY && squCurrentX < squEndX) {//square left/down shallow
				bool stepX = true;
				error = distX2 - distY2;
				while (hexCurrent != hexEnd) {
					if (error <= distX2) {
						if (CheckHexTransparency(hexCurrent))
							return true;
					}

					if (error > distX2) {
						error -= distX2;
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent += 1;
							else
								hexCurrent += (hexesW + 1);
							stepX = false;
						}
						else {
							hexCurrent += hexesW;
							stepX = true;
						}
					}
					else {
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW - 1);
							else
								hexCurrent += 1;
							stepX = false;
						}
						else
							stepX = true;

						error += distY2;
					}
				}
			}
			else if (squCurrentY < squEndY && squCurrentX > squEndX) {//square right/down shallow
				bool stepX = false;
				error = distX2 - distY2;
				while (hexCurrent != hexEnd) {
					if (error <= distX2) {
						if (CheckHexTransparency(hexCurrent))
							return true;
					}
					if (error > distX2) {
						error -= distX2;
						if (!stepX) {
							hexCurrent += hexesW;
							stepX = true;
						}
						else {
							if (hexCurrent & 0x1)
								hexCurrent += 1;
							else
								hexCurrent += (hexesW + 1);
							stepX = false;
						}
					}
					else {
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= 1;
							else
								hexCurrent += (hexesW - 1);
							stepX = true;
						}
						else
							stepX = false;
						error += distY2;
					}
				}
			}
			else if (squCurrentY > squEndY && squCurrentX < squEndX) {//square left/up shallow
				bool stepX = true;
				error = distX2 - distY2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distX2) {
						error -= distX2;
						if (stepX) {
							hexCurrent -= hexesW;
							stepX = false;
						}
						else {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW + 1);
							else
								hexCurrent -= 1;
							stepX = true;
						}
					}
					else {
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW - 1);
							else
								hexCurrent += 1;
							stepX = false;
						}
						else
							stepX = true;

						error += distY2;
					}
				}
			}
			else if (squCurrentY > squEndY && squCurrentX > squEndX) {//square right/up shallow
				bool stepX = false;
				error = distX2 - distY2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distX2) {
						error -= distX2;
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW + 1);
							else
								hexCurrent -= 1;
							stepX = true;
						}
						else {
							hexCurrent -= hexesW;
							stepX = false;
						}
					}
					else {
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= 1;
							else
								hexCurrent += (hexesW - 1);
							stepX = true;
						}
						else
							stepX = false;
						error += distY2;
					}
				}
			}
			else
				return true;
		}
		else if (distY > distX) {
			distY2 = distY * distX;
			distX2 = distX * distX;
			if (squCurrentY < squEndY && squCurrentX < squEndX) {//square left/down steep
				bool stepX = true;
				error = distY2 - distX2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distY2) {
						error -= distY2;
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW - 1);
							else
								hexCurrent += 1;
							stepX = false;
						}
						else
							stepX = true;
					}
					else {
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent += 1;
							else
								hexCurrent += (hexesW + 1);
							stepX = false;
						}
						else {
							hexCurrent += hexesW;
							stepX = true;
						}
						error += distX2;
					}
				}
			}
			else if (squCurrentY < squEndY && squCurrentX > squEndX) {//square right/down steep
				bool stepX = false;
				error = distY2 - distX2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distY2) {
						error -= distY2;
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= 1;
							else
								hexCurrent += (hexesW - 1);
							stepX = true;
						}
						else
							stepX = false;
					}
					else {
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent += 1;
							else
								hexCurrent += (hexesW + 1);
							stepX = false;
						}
						else {
							hexCurrent += hexesW;
							stepX = true;
						}
						error += distX2;
					}
				}
			}
			else if (squCurrentY > squEndY && squCurrentX > squEndX) {//square right/up steep
				bool stepX = false;
				error = distY2 - distX2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distY2) {
						error -= distY2;
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= 1;
							else
								hexCurrent += (hexesW - 1);
							stepX = true;
						}
						else
							stepX = false;
					}
					else {
						if (stepX) {
							hexCurrent -= hexesW;
							stepX = false;
						}
						else {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW + 1);
							else
								hexCurrent -= 1;
							stepX = true;
						}
						error += distX2;
					}
				}
			}
			else if (squCurrentY > squEndY && squCurrentX < squEndX) {//square left/up steep
				bool stepX = true;
				error = distY2 - distX2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distY2) {
						error -= distY2;
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW - 1);
							else
								hexCurrent += 1;
							stepX = false;
						}
						else
							stepX = true;
					}
					else {
						if (stepX) {
							hexCurrent -= hexesW;
							stepX = false;
						}
						else {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW + 1);
							else
								hexCurrent -= 1;
							stepX = true;
						}
						error += distX2;
					}
				}
			}
			else
				return true;
		}
		else
			return true;
	}
	return false;
}


//mark and display other wall objects that are 1 hex away from current wall object - to reduce blockiness
//_____________________________________________________________
void MarkVisibleWalls(OBJStruct* objViewer, OBJStruct* objWall) {
	LONG hexPos = objWall->hexNum;
	OBJNode* objNode = nullptr;
	OBJStruct* obj2 = nullptr;
	LONG type = 0;
	for (int ori = 0; ori < 6; ori++) {
		hexPos = GetNextHexPos(objWall->hexNum, ori, 1);
		objNode = pMapObjNodeArray[hexPos];
		while (objNode) {
			obj2 = objNode->obj;
			if (obj2->level <= objWall->level) {
				type = (obj2->frmID & 0x0F000000) >> 24;
				if (obj2->level == objWall->level && !(obj2->flags & FLG_MarkedByPC)) {
					if (type == ART_WALLS) {
						obj2->flags = obj2->flags | FLG_MarkedByPC;
						if (!IsInLineOfSightBlocked(objViewer->hexNum, obj2->hexNum))
							MarkVisibleWalls(objViewer, obj2);
					}
					else if (type == ART_SCENERY) {//treat doors as walls
						PROTOscenery* pro = nullptr;
						F_GetPro(obj2->proID, (PROTOall**)&pro);
						if (pro->sceneryType == FLG_Portal) {
							obj2->flags = obj2->flags | FLG_MarkedByPC;
							if (!IsInLineOfSightBlocked(objViewer->hexNum, obj2->hexNum))
								MarkVisibleWalls(objViewer, obj2);
						}
					}
				}
				objNode = objNode->next;
			}
			else
				objNode = nullptr;
		}
	}
}


//Check is object is visible to PC. returns 1=display normaly, 0=display but darken, -1=dont display.
//_______________________________
int IsVisibleByPC(OBJStruct* obj) {

	if (!FOG_OF_WAR)
		return 1;
	if (!*pDRAW_VIEW_FLAG)
		return 1;

	if (!isRecordingObjFog)
		return 1;

	OBJStruct* pObj_PC = *lpObj_PC;
	if (obj == pObj_PC) {
		obj->flags = obj->flags | FLG_MarkedByPC;
		return 1;
	}
	LONG type = (obj->frmID & 0x0F000000) >> 24;

	if (type != ART_CRITTERS) {
		//display marked objects - already seem by PC
		if (obj->flags & FLG_MarkedByPC)
			return 1;
		//if line of sight between PC and object is not blocked or object is a wall less than 6 hexes away.
		else if (!IsInLineOfSightBlocked(pObj_PC->hexNum, obj->hexNum) || (GetHexDistance(pObj_PC->hexNum, obj->hexNum) < 2 && type == ART_WALLS)) {
			obj->flags = obj->flags | FLG_MarkedByPC;
			//mark and display other wall objects that are 1 hex away from current wall object - to reduce blockiness
			if (type == ART_WALLS)
				MarkVisibleWalls(pObj_PC, obj);
			return 1;
		}
		else if (GetHexDistance(pObj_PC->hexNum, obj->hexNum) < 2 && type == ART_SCENERY) {
			PROTOscenery* pro = nullptr;
			F_GetPro(obj->proID, (PROTOall**)&pro);
			if (pro->sceneryType == 0) {
				obj->flags = obj->flags | FLG_MarkedByPC;
				return 1;
			}
		}
		//display these objects at low light until found - to reduce pop in uglyness
		else if (type == ART_WALLS || type == ART_SCENERY || type == ART_MISC) {
			return 0;
		}
		else
			return -1;
	}
	else if (type == ART_CRITTERS) {
		obj->flags = obj->flags | FLG_MarkedByPC;
		//if is on PC's team.
		if (obj->pud.critter.combat_data.teamNum == 0)
			return 1;
		//if in combat mode and is visible in combat line of sight system.
		else if ((obj->combatFlags & FLG_IsNotPC) && !(obj->combatFlags & FLG_IsNotFightable) && !(obj->combatFlags & FLG_NotVisByPC))
			return 1;
		//if is in line of sight of PC.
		else if (!IsInLineOfSightBlocked(pObj_PC->hexNum, obj->hexNum))
			return 1;
	}
	return -1;
}


//______________________________________
void DrawObjects(RECT* rect, LONG level) {
	OBJNode* mapObj = nullptr;
	LONG globalLight = *pAmbientLightIntensity;
	LONG hexLight = 0;
	int upperObjCount = 0;
	combatOutlineCount = 0;

	int isVisPC = 0;
	LONG hexPos = 0;
	//draw flat objects first
	for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
		hexLight = F_GetHexLightIntensity(level, hexPos);
		if (hexLight < globalLight)
			hexLight = globalLight;
		mapObj = pMapObjNodeArray[hexPos];

		while (mapObj) {
			if (mapObj->obj->level <= level) {
				if (mapObj->obj->level == level) {
					if ((mapObj->obj->flags & FLG_Flat)) {
						if (!(mapObj->obj->flags & 0x1)) {
							isVisPC = IsVisibleByPC(mapObj->obj);
							if (isVisPC == 0 && (hexLight < fogLight || fogLight == 0))
								isVisPC = 1;
							if (isVisPC == 1)
								F_DrawObj(mapObj->obj, rect, hexLight);
							else if (isVisPC == 0)
								F_DrawObj(mapObj->obj, rect, fogLight);

							if ((mapObj->obj->combatFlags & FLG_IsNotPC) && !(mapObj->obj->combatFlags & FLG_IsNotFightable)) {
								if (!FOG_OF_WAR || mapObj->obj->pud.critter.combat_data.teamNum == 0 || !(mapObj->obj->combatFlags & FLG_NotVisByPC)) {
									if (combatOutlineCount < 500) {
										pCombatOutlineList[combatOutlineCount] = mapObj->obj;
										combatOutlineCount++;
									}
								}
							}
						}
					}
					else {
						upperMapObjNodeArray[upperObjCount] = mapObj;
						upperObjCount++;
						mapObj = nullptr;
					}
				}
			}
			else
				mapObj = nullptr;
			if (mapObj)
				mapObj = mapObj->next;
		}
	}

	CheckAngledObjEdge(rect, 0);
	//draw non flat objects
	for (int objNum = 0; objNum < upperObjCount; objNum++) {
		mapObj = upperMapObjNodeArray[objNum];
		if (mapObj)
			hexPos = mapObj->obj->hexNum;

		hexLight = F_GetHexLightIntensity(level, hexPos);
		if (hexLight < globalLight)
			hexLight = globalLight;

		while (mapObj) {
			if (mapObj->obj->level <= level) {
				if (mapObj->obj->level == level) {
					if (!(mapObj->obj->flags & 0x1)) {
						isVisPC = IsVisibleByPC(mapObj->obj);
						if (isVisPC == 0 && (hexLight < fogLight || fogLight == 0))
							isVisPC = 1;
						if (isVisPC == 1)
							F_DrawObj(mapObj->obj, rect, hexLight);
						else if (isVisPC == 0)
							F_DrawObj(mapObj->obj, rect, fogLight);

						if ((mapObj->obj->combatFlags & FLG_IsNotPC) && !(mapObj->obj->combatFlags & FLG_IsNotFightable)) {
							if (!FOG_OF_WAR || mapObj->obj->pud.critter.combat_data.teamNum == 0 || !(mapObj->obj->combatFlags & FLG_NotVisByPC)) {
								if (combatOutlineCount < 500) {
									pCombatOutlineList[combatOutlineCount] = mapObj->obj;
									combatOutlineCount++;
								}
							}
						}
					}
				}
			}
			else
				mapObj = nullptr;
			if (mapObj)
				mapObj = mapObj->next;
		}
	}
	CheckAngledObjEdge(rect, 1);
}


//Check if object is visible -for mouse selection.
//______________________________
bool IsNotFogged(OBJStruct* obj) {

	if (!FOG_OF_WAR)
		return true;

	OBJStruct* pObj_PC = *lpObj_PC;

	LONG objType = (obj->frmID & 0x0F000000) >> 0x18;
	//allow critters if in line of sight for normal or combat modes.
	if (objType == ART_CRITTERS) {
		if (obj->pud.critter.combat_data.teamNum == 0)
			return true;
		else if ((obj->combatFlags & FLG_IsNotPC) && !(obj->combatFlags & FLG_IsNotFightable) && !(obj->combatFlags & FLG_NotVisByPC))
			return true;
		else if (IsInLineOfSightBlocked(pObj_PC->hexNum, obj->hexNum))
			return false;
	}
	//allow wall and scenery if fog is set to 0;
	else if (!fogLight && (objType == ART_WALLS || objType == ART_SCENERY || objType == ART_MISC))
		return true;
	//allow if obj is visible.
	else if (!(obj->flags & FLG_MarkedByPC))
		return false;

	return true;
}


//find the object who's frm lies under mouse cursor.
//_________________________________________________________________________________________
LONG GetObjectsAtPos(LONG xPos, LONG yPos, LONG level, LONG type, OBJInfo** lpObjInfoArray) {

	OBJNode* mapObj = nullptr;
	OBJInfo* pObjInfoArray = nullptr;
	*lpObjInfoArray = pObjInfoArray;

	DWORD objInfoArraySize = 0;

	bool exitLoop = false;
	LONG objType = 0;
	DWORD flags = 0;
	LONG numObjects = 0;
	for (LONG hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {

		mapObj = pMapObjNodeArray[hexPos];
		objInfoArraySize = sizeof(OBJInfo) * (numObjects + 1);

		while (mapObj && !exitLoop) {
			if (mapObj->obj->level <= level) {
				if (mapObj->obj->level == level) {
					if (type != -1) {
						objType = mapObj->obj->frmID & 0x0F000000;
						objType = objType >> 0x18;
					}
					else
						objType = -1;

					if (objType == type && mapObj->obj != *lpObjSpecial && IsNotFogged(mapObj->obj)) {
						flags = F_CheckObjectFrmAtPos(mapObj->obj, xPos, yPos);
						if (flags) {
							pObjInfoArray = (OBJInfo*)FReallocateMemory((BYTE*)pObjInfoArray, objInfoArraySize);
							if (pObjInfoArray) {
								pObjInfoArray[numObjects].obj = mapObj->obj;
								pObjInfoArray[numObjects].flags = flags;
								numObjects++;
								objInfoArraySize += sizeof(OBJInfo);
							}
						}
					}
				}
				mapObj = mapObj->next;
			}
			else
				exitLoop = true;
		}
		exitLoop = false;
	}
	*lpObjInfoArray = pObjInfoArray;

	return numObjects;
}


//_____________________________________________________
LONG Map_GetTileNum(LONG scrnX, LONG scrnY, LONG level) {
	LONG tileX = 0, tileY = 0;
	GetTileXY(scrnX, scrnY, level, &tileX, &tileY);
	if (tileX < 0 || tileX > *pNUM_TILE_X || tileY < 0 || tileY > *pNUM_TILE_Y)
		return -1;
	else
		return tileY * *pNUM_TILE_X + tileX;
}


//_______________________________________
void SetMapTile(DWORD frmID, DWORD proID) {

	LONG xPos = 0, yPos = 0;
	F_GetMousePos(&xPos, &yPos);
	LONG tilePos = Map_GetTileNum(xPos, yPos, *pMAP_LEVEL);
	if (tilePos == -1)
		return;

	DWORD** levelOffset = *pMapTileLevelOffset;
	LONG tileListNum = levelOffset[*pMAP_LEVEL][tilePos];
	DWORD newListNum = frmID & 0x00000FFF;

	if (*pAreRoovesVisible) {
		tileListNum = (tileListNum & 0x0000FFFF) + (newListNum << 16);
	}
	else {
		tileListNum = (tileListNum & 0xFFFF0000) + newListNum;
	}

	levelOffset[*pMAP_LEVEL][tilePos] = tileListNum;
}


//________________________
LONG Get_GRID001_ListNum() {

	return *pGRID001_LST_NUM;
}


//________________________________________________________
void DeleteMapObjs(OBJStruct** lpObj1, OBJStruct** lpObj2) {

	RECT rect;
	int level = -1;
	OBJStruct* obj = nullptr;

	if (lpObj2 && *lpObj2) {
		obj = *lpObj2;
		level = obj->level;
		F_Obj_ClearAnimation(obj);
		F_MapObj_Destroy(obj, &rect);
		F_DrawMapArea(&rect, level);
		*lpObj2 = nullptr;
	}
	if (lpObj1 && *lpObj1) {
		obj = *lpObj1;
		level = obj->level;
		F_Obj_ClearAnimation(obj);
		F_MapObj_Destroy(obj, &rect);
		F_DrawMapArea(&rect, level);
		*lpObj1 = nullptr;
	}
}


//___________________
LONG F_SelectHexpos() {

	LONG retVal = 0;
	__asm {
		call F_SELECT_HEXPOS
	}
	return retVal;
}


//_______________________________________
void F_DrawMapArea(RECT* rect, int level) {

	__asm {
		push ebx
		mov edx, level
		mov eax, rect
		mov ebx, pF_DRAW_MAP_AREA
		call dword ptr ds : [ebx]
		pop ebx
	}
}


//__________________________
LONG SetMapLevel(LONG level) {

	LONG retVal = 0;
	__asm {
		mov eax, level
		call F_SET_MAP_LEVEL
		mov retVal, eax
	}
	return retVal;
}


//____________________
LONG ToggleMapRooves() {

	*pAreRoovesVisible = 1 - *pAreRoovesVisible;

	if (*pDRAW_VIEW_FLAG)
		F_DrawMapArea(SCRN_RECT, *pMAP_LEVEL);
	return *pAreRoovesVisible;
}


//__________________
void ShowMapRooves() {

	*pAreRoovesVisible = 1;
}


//__________________
void HideMapRooves() {

	*pAreRoovesVisible = 0;
}

//________________________
LONG AreMapRoovesVisible() {

	return *pAreRoovesVisible;
}


//___________________
LONG ToggleMapHexes() {

	*pAreMapHexesVisible = 1 - *pAreMapHexesVisible;
	return *pAreMapHexesVisible;
}


//_________________
void ShowMapHexes() {

	*pAreMapHexesVisible = 1;
}


//_________________
void HideMapHexes() {

	*pAreMapHexesVisible = 0;
}


//_______________________
LONG AreMapHexesVisible() {

	return *pAreMapHexesVisible;
}


//___________________________________________________________________________
LONG PlaceObjOntoMap(OBJStruct* obj, LONG hexPos, LONG level, RECT* rcRetObj) {

	LONG retVal = 0;
	__asm {
		mov ecx, rcRetObj
		mov ebx, level
		mov edx, hexPos
		mov eax, obj
		call F_PLACE_OBJ_ONTO_MAP
		mov retVal, eax
	}
	return retVal;
}


//_____________________________________________
void GetObjectRect(OBJStruct* obj, RECT* rcObj) {;

	__asm {
		mov edx, rcObj
		mov eax, obj
		call F_GET_OBJ_RECT
	}
}


//_________________________________________________________
void F_DrawFloorTile(DWORD fid, LONG x, LONG y, RECT* rect) {

	__asm {
		mov ecx, rect
		mov ebx, y
		mov edx, x
		mov eax, fid
		call F_DRAW_FLOOR_TILE
	}
}


//__________________
void SetMapGlobals() {

	char temp_chars[256];

	ConfigReadString("MAPS", "SCROLL_DIST_X", "480", temp_chars, 255);
	if (strncmp(temp_chars, "HALF_SCRN", 9) == 0)
		SCROLL_DIST_X = SCR_WIDTH / 2 + 32;
	else
		SCROLL_DIST_X = atoi(temp_chars);
	if (SCROLL_DIST_X < 480)SCROLL_DIST_X = 480;

	ConfigReadString("MAPS", "SCROLL_DIST_Y", "400", temp_chars, 255);
	if (strncmp(temp_chars, "HALF_SCRN", 9) == 0)
		SCROLL_DIST_Y = SCR_HEIGHT / 2 + 24;
	else
		SCROLL_DIST_Y = atoi(temp_chars);
	if (SCROLL_DIST_Y < 400)SCROLL_DIST_Y = 400;

	if (ConfigReadInt("MAPS", "EDGE_CLIPPING_ON", 0))
		EDGE_CLIPPING_ON = true;

	if (ConfigReadInt("MAPS", "FOG_OF_WAR", 0))
		FOG_OF_WAR = true;
	else
		FOG_OF_WAR = false;
	fogLight = ConfigReadInt("MAPS", "FOG_LIGHT_LEVEL", 0);
	if (fogLight < 1 || fogLight > 10)
		fogLight = 0;
	else
		fogLight = fogLight * 0x1000;

}




//_________________
void FMapperSetup() {

	F_GET_SCRN_HEX_POS = (void*)FixAddress(0x49E864);

	pMapTileLevelOffset = (DWORD***)FixAddress(0x668E5C);

	lpObjSpecial = (OBJStruct**)FixAddress(0x65F620);
	pAmbientLightIntensity = (LONG*)FixAddress(0x5058D4);

	pLightHexArray = (LONG*)FixAddress(0x59CF3C);

	pMapObjNodeArray = (OBJNode**)FixAddress(0x638310);


	pHEXPOS_ADJUSTMENT = (BYTE*)FixAddress(0x668A24);

	pMOUSE_PIC_NUM = (LONG*)FixAddress(0x505360);

	pMAP_LEVEL = (LONG*)FixAddress(0x505BEC);
	pVIEW_HEXPOS = (LONG*)FixAddress(0x668E74);


	pNUM_HEX_X = (LONG*)FixAddress(0x668E58);
	pNUM_HEX_Y = (LONG*)FixAddress(0x668E64);
	pNUM_HEXES = (LONG*)FixAddress(0x668E68);

	pNUM_TILE_Y = (LONG*)FixAddress(0x668E6C);
	pNUM_TILE_X = (LONG*)FixAddress(0x668E70);
	pNUM_TILES = (LONG*)FixAddress(0x668E54);


	pVIEW_SQU_HEX_X = (LONG*)FixAddress(0x668E28);
	pVIEW_SQU_HEX_Y = (LONG*)FixAddress(0x668E24);


	pVIEW_SQU_TILE_X = (LONG*)FixAddress(0x668E34);
	pVIEW_SQU_TILE_Y = (LONG*)FixAddress(0x668E38);
	pVIEW_TILE_X = (LONG*)FixAddress(0x668E30);
	pVIEW_TILE_Y = (LONG*)FixAddress(0x668E2C);


	pVIEW_HEX_X = (LONG*)FixAddress(0x668E44);
	pVIEW_HEX_Y = (LONG*)FixAddress(0x668E48);


	pSCROLL_BLOCK_FLAG = (DWORD*)FixAddress(0x508424);
	pPC_SCROLL_LIMIT_FLAG = (DWORD*)FixAddress(0x508428);

	lpObj_PC = (OBJStruct**)FixAddress(0x65F638);
	lpObj_ActiveCritter = (OBJStruct**)FixAddress(0x56BBD0);
	lpObj_DialogFocus = (OBJStruct**)FixAddress(0x505108);


	lpGameWinBuff = (BYTE**)FixAddress(0x6302EC);
	lpViewWinBuff = (BYTE**)FixAddress(0x668E60);

	prcGameWin = (RECT*)FixAddress(0x6302D4);

	pViewWinWidth1 = (LONG*)FixAddress(0x668E50);
	pViewWinHeight = (LONG*)FixAddress(0x668E3C);
	pViewWinWidth2 = (LONG*)FixAddress(0x668E4C);

	prcViewWin = (RECT*)FixAddress(0x668614);

	//Map text vars
	lpObjViewTextBuff = (BYTE**)FixAddress(0x665288);
	pObjViewTextWidth = (LONG*)FixAddress(0x665280);
	pObjViewTextHeight = (LONG*)FixAddress(0x665284);


	pViewWin2Width1 = (LONG*)FixAddress(0x65F634);
	pViewWin2Height = (LONG*)FixAddress(0x65F62C);
	lpViewWin2Buff = (BYTE**)FixAddress(0x65F628);
	pViewWin2Width2 = (LONG*)FixAddress(0x65F630);

	prcViewWin2 = (RECT*)FixAddress(0x638300);

	pViewWin2Size = (LONG*)FixAddress(0x65F624);


	F_CHECK_OBJ_FRM_AT_POS = (void*)FixAddress(0x47DEC0);

	F_GET_HEX_LIGHT_LEVEL = (void*)FixAddress(0x46CF48);
	F_DRAW_OBJECT = (void*)FixAddress(0x480CF8);
	F_DRAW_FLOOR_TILE = (void*)FixAddress(0x4A0074);

	F_PLACE_OBJ_ONTO_MAP = (void*)FixAddress(0x47F69C);

	F_GET_OBJ_RECT = (void*)FixAddress(0x47D598);


	F_FALLOUT2_SETUP = (void*)FixAddress(0x472D10);

	F_INIT_MAP_AREA = (void*)FixAddress(0x474010);

	F_CHECK_GAME_INPUT = (void*)FixAddress(0x43BA7C);

	F_RESET_NEW_MAP = (void*)FixAddress(0x474924);

	F_LOAD_MAP_FROM_LIST = (void*)FixAddress(0x4749DC);

	F_LOAD_MAP = (void*)FixAddress(0x474A54);

	F_SAVE_MAP = (void*)FixAddress(0x475CBC);

	F_RESET_IFACE_BAR = (void*)FixAddress(0x4547D8);

	F_SET_MOUSE_MODE_FLAG = (void*)FixAddress(0x4448E4);

	F_INTFACE_INIT = (void*)FixAddress(0x453650);

	F_INTFACE_DESTROY = (void*)FixAddress(0x45E440);


	pAreMapHexesVisible = (LONG*)FixAddress(0x508430);
	F_DRAW_MAP_HEXES = (void*)FixAddress(0x49FE48);

	pF_DRAW_MAP_AREA = (void**)FixAddress(0x508434);


	pAreRoovesVisible = (LONG*)FixAddress(0x50842C);

	pDRAW_VIEW_FLAG = (DWORD*)FixAddress(0x508438);

	F_SET_MAP_LEVEL = (void*)FixAddress(0x47419C);


	F_PATHFUNC02 = (void*)FixAddress(0x47D780);



	F_SELECT_HEXPOS = (void*)FixAddress(0x4127E0);

	pGRID001_LST_NUM = (LONG*)FixAddress(0x4FECF0);

	pCurrentMapName = (char*)FixAddress(0x6302F4);


	F_SAVE_MAP_DATA = (void*)FixAddress(0x4759D0);

	F_COPY_SAVE_FILE = (void*)FixAddress(0x471EEC);
}
