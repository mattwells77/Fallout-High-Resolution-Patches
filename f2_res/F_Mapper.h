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

#pragma once

#include "F_Objects.h"

class MAPedge {
   public:
   LONG x;
   LONG y;
   RECT sqrEdge;
   RECT visEdge;
   RECT tileEdge;
   RECT angleEdge;
   DWORD angleFlag;
   MAPedge *prevArea;
   MAPedge *nextArea;
};


class MAPdata {
   public:
   MAPedge mapEDGE[3];
   MAPedge *currentEDGES;

};


class OBJInfo {
   public:
   DWORD flags;
   OBJStruct *obj;
};


struct TILE_list {
   LONG level;
   LONG num;
   DWORD mask;
   DWORD tiles;
   TILE_list *next;
};


extern MAPdata M_CURRENT_MAP;
extern LONG *pVIEW_HEXPOS;
extern LONG *pMAP_LEVEL;
extern DWORD *pPC_SCROLL_LIMIT_FLAG;

extern OBJStruct *pCombatOutlineList[500];
extern LONG combatOutlineCount;

extern DWORD *pSCROLL_BLOCK_FLAG;
extern char workingMapName[32];
extern char *pCurrentMapName;

extern LONG EDGE_OFF_X;
extern LONG EDGE_OFF_Y;

extern LONG FOG_OF_WAR;

extern DWORD* pDRAW_VIEW_FLAG;


LONG ScrnSqr2HexPosMove(LONG x, LONG y, bool axis);
void ScrnHexPos2Sqr(LONG hexPos, LONG* px, LONG* py);
void GetHexSqrXY(LONG hexPos, LONG* px, LONG* py);
LONG GetScrnXYTile(LONG tileNum, LONG* scrnX, LONG* scrnY, LONG level);
LONG SetViewPos(LONG hexPos, DWORD flags);
void SetMapGlobals(int region);
void SetMapperScrollInfo();
void ReDrawViewWin();

LONG SetMapLevel(LONG level);

void F_DrawFloorTile(DWORD fid, LONG x, LONG y, RECT *rect);
void GetObjectRect(OBJStruct *obj, RECT *rcObj);
void F_DrawMapArea(RECT *rect, int level);

LONG ToggleMapRooves();
void ShowMapRooves();
void HideMapRooves();
LONG AreMapRoovesVisible();

LONG ToggleMapHexes();
void ShowMapHexes();
void HideMapHexes();
LONG AreMapHexesVisible();

void F_IntFace_Init();
void F_IntFace_Destroy();
void F_SetMouseModeFlag(DWORD flag);
void F_ResetIfaceBar();
void F_LoadMap( char *mapName);
void F_SaveMap( char *mapName);
void SaveMap(char *mapName);
void F_ResetNewMap();
void F_CheckGameInput(int keyCode, DWORD flags);
void F_InitMapArea();
int F_Fallout2Setup(int pathLength, char *path);

LONG F_SelectHexpos();
void DeleteMapObjs(OBJStruct **lpObj1, OBJStruct **lpObj2);
LONG Get_GRID001_ListNum();

LONG GetNextHexPos(LONG hexPos, UINT direction, LONG distance);
LONG GetHexDistance(LONG hexStart, LONG hexEnd);

bool FogOfWarMap_CopyFiles(char *pFromPath, char *pToPath);
LONG FogOfWarMap_DeleteTmps(char *path);
bool FogOfWarMap_Save(char *MapName);
bool FogOfWarMap_Load(char *MapName);

void SetMapBorders(char *MapName);
LONG ClipEdgeRect(RECT *rect1, RECT *rect2, RECT *rect3);

bool isHexWithinMapEdges(LONG hexPos);
LONG SetViewPos(LONG hexPos, DWORD flags);

void GetTileXY(LONG scrnX, LONG scrnY, LONG level, LONG *tileX, LONG *tileY);
LONG GetScrnXYTile(LONG tileNum, LONG *scrnX, LONG *scrnY, LONG level);
LONG GetFloorHexLight(LONG elev, LONG hexNum, LONG globalLight);

void DrawFloorTiles(RECT *rect, LONG level);
DWORD CheckAngledTileEdge(int x, int y, DWORD tileLstNum);

void CheckAngledObjEdge(RECT *rect, DWORD isUpper);
DWORD CheckAngledRoofTileEdge(LONG xPos, LONG yPos, DWORD tileLstNum);

void DrawObjects(RECT *rect, LONG level);
void DrawDialogView(BYTE *frmBuff, LONG subWidth, LONG subHeight, LONG frmWidth, BYTE *toBuff, LONG toWidth);

LONG GetObjectsAtPos(LONG xPos, LONG yPos, LONG level, LONG type, OBJInfo **lpObjInfoArray);

LONG MergeRects(RECT *rect1, RECT *rect2, RECT *newRect);

void SetMapTile(DWORD frmID, DWORD proID);
void SetMapObject(DWORD frmID, DWORD proID);

bool ReSizeMaps();
void FMapperSetup(DWORD region);

LONG F_GetScrnHexPos(LONG scrnX, LONG scrnY, LONG level);

void F_DrawFloatingText();
void F_DrawObjOutlineRed(OBJStruct *obj, RECT *pRect);
bool GetHexXY(LONG hexPos, LONG *x, LONG *y);
bool CreateNewMapObject(LONG hexNum, DWORD frmID, DWORD proID);
bool CreateNewMapObjectFromObject(LONG hexNum, OBJStruct *pObjFrom);
LONG GetTileNum(LONG scrnX, LONG scrnY, LONG level);
bool CreateNewMapTile(LONG xPos, LONG yPos, DWORD frmID);

void DeleteMapTile(LONG tileNum, DWORD mask, LONG level);

DWORD GetTileListNum(LONG tileNum);
bool CreatNewTileFromTile(LONG tileNum, TILE_list *tileNode);

void DrawEdgeLines(RECT *rect, int level);

bool SaveMapEdges(char *MapName);
