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

#pragma pack(2)
class FRMframe {
public:
	WORD width;
	WORD height;
	DWORD size;
	INT16 x;
	INT16 y;
	//BYTE index[];
};


class FRMhead {
public:
	DWORD version;//version num
	WORD FPS;//frames per sec
	WORD actionFrame;
	WORD numFrames;//number of frames per direction
	INT16 xCentreShift[6];//offset from frm centre +=right -=left
	INT16 yCentreShift[6];//offset from frm centre +=down -=up
	DWORD oriOffset[6];//frame area offset for diff orientations
	DWORD frameAreaSize;
};
#pragma pack()


//structures for loading unlisted frms
class UNLSTDframe {
public:
	WORD width;
	WORD height;
	DWORD size;
	INT16 x;
	INT16 y;
	BYTE* buff;
	UNLSTDframe() {
		width = 0;
		height = 0;
		size = 0;
		x = 0;
		y = 0;
		buff = nullptr;
	}
	~UNLSTDframe() {
		if (buff != nullptr)
			delete[] buff;
	}
};


class UNLSTDfrm {
public:
	DWORD version;
	WORD FPS;
	WORD actionFrame;
	WORD numFrames;
	INT16 xCentreShift[6];
	INT16 yCentreShift[6];
	DWORD oriOffset[6];
	DWORD frameAreaSize;
	UNLSTDframe* frames;
	UNLSTDfrm() {
		version = 0;
		FPS = 0;
		actionFrame = 0;
		numFrames = 0;
		for (int i = 0; i < 6; i++) {
			xCentreShift[i] = 0;
			yCentreShift[i] = 0;
			oriOffset[i] = 0;
		}
		frameAreaSize = 0;
		frames = nullptr;
	}
	~UNLSTDfrm() {
		if (frames != nullptr)
			delete[] frames;
	}
};


class FRMnode {
public:
	LONG isInUse;
	DWORD ref;
	UNLSTDfrm* frm;
	FRMnode* prev;
	FRMnode* next;
	FRMnode(UNLSTDfrm* frmNew, DWORD newRef) {
		isInUse = 1;
		ref = newRef;
		frm = frmNew;
		prev = nullptr;
		next = nullptr;
	}
	~FRMnode() {
		if (frm)
			delete frm;
		prev = nullptr;
		next = nullptr;
	}
};


class FRMlist {
	int num;
	FRMnode* list[11];
public:
	FRMlist() {
		num = 0;
		for (int i = 0; i < 11; i++)
			list[i] = nullptr;
	}
	~FRMlist() {
		for (int i = 0; i < 11; i++) {
			if (list[i]) {
				FRMnode* tmpNode = nullptr;
				while (list[i]) {
					tmpNode = list[i]->next;
					delete list[i];
					list[i] = tmpNode;
				}
				tmpNode = nullptr;
				list[i] = nullptr;
			}
		}
	}
	DWORD AddFrmToList(UNLSTDfrm* frm, DWORD folderRef, DWORD ref);
	UNLSTDfrm* GetFrm(const char* frmName, DWORD folderRef);
	UNLSTDfrm* GetFrm(DWORD ref, DWORD folderRef);
	FRMnode* GetFrmNode(DWORD ref, DWORD folderRef);
	void DeleteFrmNode(FRMnode* frmNode, DWORD folderRef);
	UNLSTDfrm* LoadFrm(const char* frmName, DWORD folderRef, DWORD* pRef);
	UNLSTDfrm* NewFrm(LONG width, LONG height, LONG numFrames, LONG numOri, const char* frmName, DWORD folderRef, DWORD* pRef);
	LONG UnLoadFrm(const char* frmName, DWORD folderRef);
};


extern FRMlist* hrFrmList;


struct ARTtype {
	LONG enabled;
	char name[8];
	DWORD unkVal01;
	DWORD unkVal02;
	DWORD unkVal03;
	DWORD unkVal04;
	DWORD totalArt;
};


#define ART_ITEMS 0
#define ART_CRITTERS 1
#define ART_SCENERY 2
#define ART_WALLS 3
#define ART_TILES 4
#define ART_MISC 5
#define ART_INTRFACE 6
#define ART_INVEN 7
#define ART_HEADS 8
#define ART_BACKGRND 9
#define ART_SKILLDEX 10

LONG IsArtTypeEnabled(LONG type);
char* GetArtTypeName(LONG type);
LONG ToggleArtTypeEnabled(LONG type);

UNLSTDfrm* LoadUnlistedFrm(const char* FrmName, unsigned int folderRef);
UNLSTDfrm* CreateUnlistedFrm(int width, int height, int numFrames, int numOri);

DWORD F_GetFrmID(DWORD objType, DWORD lstNum, DWORD id2, DWORD id1, DWORD id3);//(DWORD lstType, DWORD lstNum);
void F_UnloadFrm(DWORD frmObj);
BYTE* F_GetFrmBuff(DWORD FID, DWORD frameNum, DWORD ori, DWORD* frmObj);
FRMhead* F_GetFrm(DWORD FID, DWORD* frmObj);
DWORD F_GetFrmFrameWidth(FRMhead* frm, DWORD frameNum, DWORD ori);
DWORD F_GetFrmFrameHeight(FRMhead* frm, DWORD frameNum, DWORD ori);
BYTE* F_GetFrmFrameBuff(FRMhead* frm, DWORD frameNum, DWORD ori);

int F_CheckFrmFileExists(DWORD frmID);

void FArtSetup(int region);
