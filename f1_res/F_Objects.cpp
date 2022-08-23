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

#include "F_Objects.h"
#include "memwrite.h"


struct PROlist {
	LONG size;
	LONG unknown1;
	LONG unknown2;
	LONG unknown3;
};

OBJStruct** lpObj_Mouse2 = nullptr;
OBJStruct** lpObj_Mouse = nullptr;


void* F_GET_PRO_LIST_SIZE = nullptr;
void* F_GET_PRO_STRUCT = nullptr;;

void* GET_MAP_OBJ_UNDER_MOUSE = nullptr;

PROlist* proList;


void* F_OBJ_CLEAR_ANIMATION = nullptr;

void* F_MAP_OBJ_CREATE = nullptr;
void* F_MAP_OBJ_MOVE = nullptr;
void* F_MAP_OBJ_DESTROY = nullptr;

void* F_SET_OBJ_FID = nullptr;




//___________________________________________________________
LONG F_Obj_SetFrmId(OBJStruct* obj, DWORD frmID, RECT* rcOut) {

	__asm {
		mov ebx, rcOut
		mov edx, frmID
		mov eax, obj
		call F_SET_OBJ_FID
	}
}


//_________________________________________________________________
OBJStruct* FGetMapObjUnderMouse(int objType, DWORD flag, int level) {

	OBJStruct* retObj = nullptr;
	__asm {
		mov ebx, level
		mov edx, flag
		mov eax, objType
		call GET_MAP_OBJ_UNDER_MOUSE
		mov retObj, eax
	}
	return retObj;
}


//_______________________________
LONG GetProListSize(LONG proType) {

	return proList[proType].size;
}


//__________________________________________
LONG F_GetPro(DWORD proID, PROTOall** proto) {

	LONG retVal = 0;
	__asm {
		mov edx, proto
		mov eax, proID
		call F_GET_PRO_STRUCT
		mov retVal, eax
	}
	return retVal;
}


//________________________________________
DWORD GetProID(LONG proType, LONG listNum) {

	listNum++;
	if (listNum >= GetProListSize(proType))
		return -1;
	proType = proType << 24;
	return proType | listNum;
}


//______________________________________
int F_Obj_ClearAnimation(OBJStruct* obj) {

	int retVal = 0;
	__asm {
		mov eax, obj
		call F_OBJ_CLEAR_ANIMATION
		mov retVal, eax
	}
	return retVal;
}


//_______________________________________________________________________
LONG F_MapObj_Move(OBJStruct* obj, DWORD hexPos, LONG level, RECT* pRect) {

	LONG retVal = 0;
	__asm {
		mov ecx, pRect
		mov ebx, level
		mov edx, hexPos
		mov eax, obj
		call F_MAP_OBJ_MOVE
		mov retVal, eax
	}
	return retVal;
}


//________________________________________________
LONG F_MapObj_Destroy(OBJStruct* obj, RECT* pRect) {

	LONG retVal = 0;
	__asm {
		mov edx, pRect
		mov eax, obj
		call F_MAP_OBJ_DESTROY
		mov retVal, eax
	}
	return retVal;
}


//_______________________________________________________________
LONG F_MapObj_Create(OBJStruct** lpObj, DWORD frmID, DWORD proID) {

	LONG retVal = -1;
	__asm {
		mov ebx, proID
		mov edx, frmID
		mov eax, lpObj
		call F_MAP_OBJ_CREATE
		mov retVal, eax
	}
	return retVal;
}


//___________________
void F_ObjectsSetup() {

	lpObj_Mouse2 = (OBJStruct**)FixAddress(0x59522C);
	lpObj_Mouse = (OBJStruct**)FixAddress(0x595230);

	F_GET_PRO_LIST_SIZE = (void*)FixAddress(0x490A08);

	F_GET_PRO_STRUCT = (void*)FixAddress(0x49094C);

	GET_MAP_OBJ_UNDER_MOUSE = (void*)FixAddress(0x444DB8);

	proList = (PROlist*)FixAddress(0x50754C);

	F_OBJ_CLEAR_ANIMATION = (void*)FixAddress(0x413708);

	F_MAP_OBJ_CREATE = (void*)FixAddress(0x47BD08);

	F_MAP_OBJ_MOVE = (void*)FixAddress(0x47C6B8);

	F_MAP_OBJ_DESTROY = (void*)FixAddress(0x47D174);

	F_SET_OBJ_FID = (void*)FixAddress(0x47CAB4);
}
