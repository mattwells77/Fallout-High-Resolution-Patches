
#pragma once

//#include <windows.h>
#include "commons.h"

/*
class MAPedge {//bytes big-endian
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

class MAPdata {//bytes big-endian
   public:
   MAPedge mapEDGE[3];
   MAPedge *currentEDGES;

};
*/

/*
//bool ReSizeMaps();
LONG ScrnSqr2HexPosMove(LONG x, LONG y, bool axis);
void ScrnHexPos2Sqr(LONG hexPos, LONG* px, LONG* py);
//void ScrnSqr2Hex(LONG* px, LONG* py);
//LONG ScrnSqr2HexPos(LONG x, LONG y);
//void GetGameWinCentre(LONG *winCentreX, LONG *winCentreY);
LONG SetViewPos(LONG hexPos, DWORD flags);
extern MAPdata M_CURRENT_MAP;
//extern MAPedge defaultEDGES;
extern RECT *prcGameWin;
extern LONG *pVIEW_HEXPOS;
extern LONG *pMAP_LEVEL;

//extern DWORD *pDRAW_VIEW_FLAG;
//extern DWORD *pSCROLL_BLOCK_FLAG;
//extern _int32 *pMAP_LEVEL;
//extern bool VIEW_EDGE_LINES;
//extern char mainMapName[32];
//bool MapEdgesWindow(char *mapName, LONG level, MAPedge *mapEdge);
//void DrawGameView(LONG rectNum);
//bool SaveMapEdges(char *MapName);
//bool LoadMapEdges(char *MapName);
void SetMapperScrollInfo();

LONG SetMapLevel(LONG level);
//void DrawMapViewRect();
void ReDrawViewWin();
*/

