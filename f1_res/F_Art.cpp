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

#include "F_Art.h"
#include "F_File.h"
#include "memwrite.h"
#include "convert.h"

void* F_GET_FRM_FRAME_BUFF = nullptr;
void* F_GET_FRM_FRAME_HEIGHT = nullptr;
void* F_GET_FRM_FRAME_WIDTH = nullptr;
void* F_GET_FRM = nullptr;
void* F_GET_FRM_BUFF = nullptr;
void* F_UNLOAD_FRM = nullptr;
void* F_LOAD_FRM = nullptr;

char* pFRM_TYPE_NAME = nullptr;

ARTtype* pArtTypeArray = nullptr;

void* F_CHECK_FRM_FILE_EXISTS = nullptr;


//__________________________________
LONG ToggleArtTypeEnabled(LONG type) {

	if (type < 0 || type>11)
		return 0;

	pArtTypeArray[type].enabled = 1 - pArtTypeArray[type].enabled;

	return 1 - pArtTypeArray[type].enabled;
}


//______________________________
LONG IsArtTypeEnabled(LONG type) {

	if (type < 0 || type>11)
		return 0;

	return 1 - pArtTypeArray[type].enabled;
}


//______________________________
char* GetArtTypeName(LONG type) {

	if (type < 0 || type>11)
		return nullptr;

	return pArtTypeArray[type].name;

}


//_________________________________
int F_CheckFrmFileExists(DWORD frmID) {
	int retVal = 0;

	__asm {
		push edx
		mov eax, frmID
		call F_CHECK_FRM_FILE_EXISTS
		mov retVal, eax
		pop edx
	}
	return retVal;
}


/////////////////////////////////////////////////////////////////UNLISTED FRM FUNCTIONS////////////////////////////////////////////////////////////////////////
FRMlist* hrFrmList = nullptr;

//______________________________________________________
bool LoadFrmHeader(UNLSTDfrm* frmHeader, void* frmStream) {

	if (F_fread32(frmStream, &frmHeader->version) == -1)return 0;
	else if (F_fread16(frmStream, &frmHeader->FPS) == -1)return 0;
	else if (F_fread16(frmStream, &frmHeader->actionFrame) == -1)return 0;
	else if (F_fread16(frmStream, &frmHeader->numFrames) == -1)return 0;

	else if (F_fread16Array(frmStream, (WORD*)frmHeader->xCentreShift, 6) == -1)return 0;
	else if (F_fread16Array(frmStream, (WORD*)frmHeader->yCentreShift, 6) == -1)return 0;
	else if (F_fread32Array(frmStream, frmHeader->oriOffset, 6) == -1)return 0;
	else if (F_fread32(frmStream, &frmHeader->frameAreaSize) == -1)return 0;

	return 1;
}


//____________________________________________________
bool LoadFrmFrame(UNLSTDframe* frame, void* frmStream) {

	if (F_fread16(frmStream, &frame->width) == -1)return 0;
	else if (F_fread16(frmStream, &frame->height) == -1)return 0;
	else if (F_fread32(frmStream, &frame->size) == -1)return 0;
	else if (F_fread16(frmStream, (WORD*)&frame->x) == -1)return 0;
	else if (F_fread16(frmStream, (WORD*)&frame->y) == -1)return 0;
	frame->buff = new BYTE[frame->size];
	if (FReadString(frmStream, (char*)frame->buff, frame->size, 1) != 1)return 0;

	return 1;
}


//______________________________________________________________
UNLSTDfrm* LoadUnlistedFrm(const char* FrmName, DWORD folderRef) {

	if (folderRef > 10) return nullptr;

	char* artfolder = (char*)(pFRM_TYPE_NAME + (folderRef * 28));//address of art type name
	char FrmPath[MAX_PATH];

	sprintf(FrmPath, "art\\%s\\%s\0", artfolder, FrmName);

	UNLSTDfrm* frm = new UNLSTDfrm;

	void* frmStream = F_fopen(FrmPath, "rb");

	if (frmStream) {
		if (!LoadFrmHeader(frm, frmStream)) {
			F_fclose(frmStream);
			delete frm;
			return nullptr;
		}

		DWORD oriOffset_1st = frm->oriOffset[0];
		DWORD oriOffset_new = 0;
		frm->frames = new UNLSTDframe[6 * frm->numFrames];
		for (int ori = 0; ori < 6; ori++) {
			if (ori == 0 || frm->oriOffset[ori] != oriOffset_1st) {
				frm->oriOffset[ori] = oriOffset_new;
				for (int fNum = 0; fNum < frm->numFrames; fNum++) {
					if (!LoadFrmFrame(&frm->frames[oriOffset_new + fNum], frmStream)) {
						F_fclose(frmStream);
						delete frm;
						return nullptr;
					}
				}
				oriOffset_new += frm->numFrames;
			}
			else frm->oriOffset[ori] = 0;
		}

		F_fclose(frmStream);
	}
	else {
		delete frm;
		return nullptr;
	}
	return frm;
}


//____________________________________________________________________________
UNLSTDfrm* CreateUnlistedFrm(int width, int height, int numFrames, int numOri) {
	if (!width || !height || !numOri || !numFrames) {
		MessageBox(nullptr, "CreateUnlistedFrm failed. All frm vars must be greater than 0.", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return nullptr;
	}
	UNLSTDfrm* frm = new UNLSTDfrm;
	frm->version = 4;
	frm->numFrames = numFrames;
	frm->frameAreaSize = (width * height + 12) * numOri;
	frm->frames = new UNLSTDframe[numOri * numFrames];

	for (int ori = 0; ori < numOri; ori++) {
		for (int frame = 0; frame < numFrames; frame++) {
			frm->frames[ori + frame].width = width;
			frm->frames[ori + frame].height = height;
			frm->frames[ori + frame].size = width * height;
			frm->frames[ori + frame].buff = new BYTE[frm->frames[ori + frame].size];
			memset(frm->frames[ori + frame].buff, '\0', frm->frames[ori + frame].size);
		}
	}

	return frm;
}


//_____________________________________________________________________
DWORD FRMlist::AddFrmToList(UNLSTDfrm* frm, DWORD folderRef, DWORD ref) {
	if (list[folderRef] == nullptr) {
		list[folderRef] = new FRMnode(frm, ref);
	}
	else {
		FRMnode* frmNode = list[folderRef];
		while (frmNode->next)
			frmNode = frmNode->next;
		frmNode->next = new FRMnode(frm, ref);
		frmNode->next->prev = frmNode;
		frmNode = frmNode->next;
	}
	num++;
	return ref;
}


//______________________________________________________________
UNLSTDfrm* FRMlist::GetFrm(const char* frmName, DWORD folderRef) {
	//DWORD ref = hash((unsigned char*)frmName);
	DWORD ref = hash(frmName);
	FRMnode* frmNode = list[folderRef];
	while (frmNode) {
		if (frmNode->ref == ref)
			return frmNode->frm;
		frmNode = frmNode->next;
	}
	return nullptr;
}


//____________________________________________________
UNLSTDfrm* FRMlist::GetFrm(DWORD ref, DWORD folderRef) {
	FRMnode* frmNode = list[folderRef];
	while (frmNode) {
		if (frmNode->ref == ref)
			return frmNode->frm;
		frmNode = frmNode->next;
	}
	return nullptr;
}


//______________________________________________________
FRMnode* FRMlist::GetFrmNode(DWORD ref, DWORD folderRef) {
	FRMnode* frmNode = list[folderRef];
	while (frmNode) {
		if (frmNode->ref == ref)
			return frmNode;
		frmNode = frmNode->next;
	}
	return nullptr;
}


//____________________________________________________________________________
UNLSTDfrm* FRMlist::LoadFrm(const char* frmName, DWORD folderRef, DWORD* pRef) {
	DWORD ref = hash(frmName);
	if (pRef)
		*pRef = ref;

	FRMnode* frmNode = GetFrmNode(ref, folderRef);
	UNLSTDfrm* frm = nullptr;
	if (!frmNode) {
		frm = LoadUnlistedFrm(frmName, folderRef);
		if (frm)
			AddFrmToList(frm, folderRef, ref);
	}
	else {
		frm = frmNode->frm;
		frmNode->isInUse++;
	}
	return frm;
}


//_________________________________________________________________________________________________________________________________
UNLSTDfrm* FRMlist::NewFrm(LONG width, LONG height, LONG numFrames, LONG numOri, const char* frmName, DWORD folderRef, DWORD* pRef) {
	DWORD ref = hash(frmName);
	if (pRef)
		*pRef = ref;

	FRMnode* frmNode = GetFrmNode(ref, folderRef);
	UNLSTDfrm* frm = nullptr;
	if (!frmNode) {
		frm = CreateUnlistedFrm(width, height, numFrames, numOri);
		if (frm)
			AddFrmToList(frm, folderRef, ref);
	}
	else {
		frm = frmNode->frm;
		frmNode->isInUse++;
	}
	return frm;
}


//____________________________________________________________
void FRMlist::DeleteFrmNode(FRMnode* frmNode, DWORD folderRef) {

	if (frmNode) {
		FRMnode* tmpNode = frmNode->prev;
		if (!tmpNode)//if is top of list, list start = next node.
			list[folderRef] = frmNode->next;
		else//set previous nodes NEXT to current nodes NEXT.
			tmpNode->next = frmNode->next;

		tmpNode = frmNode->next;
		if (tmpNode)//set next nodes PREV to current nodes PREV.
			tmpNode->prev = frmNode->prev;
		frmNode->prev = nullptr;
		frmNode->next = nullptr;
		delete frmNode;//delete current node.
	}
}


//___________________________________________________________
LONG FRMlist::UnLoadFrm(const char* frmName, DWORD folderRef) {
	DWORD ref = hash(frmName);

	FRMnode* frmNode = GetFrmNode(ref, folderRef);
	if (frmNode) {
		frmNode->isInUse--;
		if (frmNode->isInUse <= 0) {
			DeleteFrmNode(frmNode, folderRef);
			return 0;
		}
	}
	return frmNode->isInUse;
}


//____________________________________________________________________________
DWORD F_GetFrmID(DWORD objType, DWORD lstNum, DWORD id2, DWORD id1, DWORD id3) {
	DWORD FID = 0;
	__asm {
		push id3
		mov ecx, id1
		mov ebx, id2
		MOV edx, lstNum
		MOV eax, objType
		call F_LOAD_FRM
		mov FID, eax
	}
	return FID;
}


//____________________________
void F_UnloadFrm(DWORD frmObj) {

	__asm {
		mov eax, frmObj
		call F_UNLOAD_FRM//0x419260
	}
}


//_____________________________________________________________________
BYTE* F_GetFrmBuff(DWORD FID, DWORD frameNum, DWORD ori, DWORD* frmObj) {
	BYTE* buff = nullptr;
	__asm {
		mov ecx, frmObj
		mov ebx, ori
		mov edx, frameNum
		mov eax, FID
		call F_GET_FRM_BUFF//0x419188
		mov buff, eax
	}
	return buff;
}


//_________________________________________
FRMhead* F_GetFrm(DWORD FID, DWORD* frmObj) {
	FRMhead* frm = nullptr;
	__asm {
		mov eax, FID
		mov edx, frmObj
		CALL F_GET_FRM//0x419160
		MOV frm, EAX
	}
	return frm;
}


//_______________________________________________________________
DWORD F_GetFrmFrameWidth(FRMhead* frm, DWORD frameNum, DWORD ori) {
	DWORD width = 0;
	__asm {
		mov ebx, ori//0-5
		mov edx, frameNum
		mov eax, frm
		call F_GET_FRM_FRAME_WIDTH//0x4197A0
		mov width, eax
	}
	return width;
}


//________________________________________________________________
DWORD F_GetFrmFrameHeight(FRMhead* frm, DWORD frameNum, DWORD ori) {
	DWORD height = 0;
	__asm {
		mov ebx, ori//0-5
		mov edx, frameNum
		mov eax, frm
		call F_GET_FRM_FRAME_HEIGHT//0x4197B8
		mov height, eax
	}
	return height;
}


//______________________________________________________________
BYTE* F_GetFrmFrameBuff(FRMhead* frm, DWORD frameNum, DWORD ori) {
	BYTE* buff = nullptr;

	__asm {
		mov ebx, ori//0-5
		mov edx, frameNum
		mov eax, frm
		call F_GET_FRM_FRAME_BUFF//0x419870
		mov buff, eax
	}
	return buff;
}


//______________
void FArtSetup() {

	F_GET_FRM_FRAME_BUFF = (void*)FixAddress(0x41905C);
	F_GET_FRM_FRAME_HEIGHT = (void*)FixAddress(0x418FF0);
	F_GET_FRM_FRAME_WIDTH = (void*)FixAddress(0x418FD8);
	F_GET_FRM = (void*)FixAddress(0x418990);
	F_GET_FRM_BUFF = (void*)FixAddress(0x4189B8);
	F_UNLOAD_FRM = (void*)FixAddress(0x418A90);
	F_LOAD_FRM = (void*)FixAddress(0x41944C);
	pFRM_TYPE_NAME = (char*)FixAddress(0x4FEBA8);

	pArtTypeArray = (ARTtype*)FixAddress(0x4FEBA4);

	F_CHECK_FRM_FILE_EXISTS = (void*)FixAddress(0x4190B4);
}
