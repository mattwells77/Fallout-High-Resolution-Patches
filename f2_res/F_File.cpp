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

#include "F_File.h"
#include "memwrite.h"

void* F_OPEN_FILE = nullptr;
void* F_CLOSE_FILE = nullptr;

void* F_FSEEK = nullptr;

void* F_READ_BYTE = nullptr;
void* F_READ_WORD = nullptr;
void* F_READ_DWORD = nullptr;
void* F_READ_WORD_ARRAY = nullptr;
void* F_READ_DWORD_ARRAY = nullptr;
void* F_READ_STRING = nullptr;
void* F_READ_CHAR_ARRAY = nullptr;

void* F_READ_CFG_INT = nullptr;
void* F_READ_CFG_STRING = nullptr;
void* F_WRITE_CFG_INT = nullptr;

void* F_SET_DATA_PATH = nullptr;

void* F_DELETE_SAVE_FILES = nullptr;
void* F_WRITE_BYTE = nullptr;
void* F_WRITE_WORD = nullptr;
void* F_WRITE_DWORD = nullptr;
void* F_WRITE_DWORD_ARRAY = nullptr;

void* pCFG_FILE_PTR = nullptr;
char* pCFG_FILE_NAME = nullptr;

void* F_GET_FILE_LIST = nullptr;
void* F_RELEASE_FILE_LIST = nullptr;


//_______________________________________________
LONG F_GetFileList(const char* path, char*** list) {

	LONG numItems = 0;
	__asm {
		push ecx
		push ebx

		mov edx, list
		mov eax, path
		call F_GET_FILE_LIST
		mov numItems, eax

		pop ebx
		pop ecx
	}
	return numItems;
}


//__________________________________
LONG F_ReleaseFileList(char*** list) {

	LONG retVal = 0;
	__asm {
		xor edx, edx
		mov eax, list
		call F_RELEASE_FILE_LIST
		mov retVal, eax
	}
	return retVal;
}


//_________________________________________
LONG F_fwrite8(void* FileStream, BYTE val8) {

	LONG retVal = 0;
	__asm {
		xor edx, edx
		mov dl, val8
		mov eax, FileStream
		call F_WRITE_BYTE
		mov retVal, eax
	}
	return retVal;
}


//___________________________________________
LONG F_fwrite16(void* FileStream, WORD val16) {

	LONG retVal = 0;
	__asm {
		xor edx, edx
		mov dx, val16
		mov eax, FileStream
		call F_WRITE_WORD
		mov retVal, eax
	}
	return retVal;
}


//____________________________________________
LONG F_fwrite32(void* FileStream, DWORD val32) {

	LONG retVal = 0;
	__asm {
		mov edx, val32
		mov eax, FileStream
		call F_WRITE_DWORD
		mov retVal, eax
	}
	return retVal;
}


//__________________________________________________________________________
LONG F_fwrite32Array(void* FileStream, DWORD* val32Array, DWORD numElements) {

	LONG retVal = 0;
	__asm {
		mov ebx, numElements
		mov edx, val32Array
		mov eax, FileStream
		call F_WRITE_DWORD_ARRAY
		mov retVal, eax
	}
	return retVal;
}


//________________________________________________________
LONG FDeleteTmpSaveFiles(const char* path, const char* ext) {

	LONG retVal = 0;
	__asm {
		mov edx, ext
		mov eax, path
		call F_DELETE_SAVE_FILES
		mov retVal, eax
	}
	return retVal;
}


//____________________________________________________________________________________
LONG FSetDataPath(const char* path1, int isFolder1, const char* path2, int isFolder2) {

	LONG retVal = 0;
	__asm {
		mov ecx, isFolder2
		mov ebx, path2
		mov edx, isFolder1
		mov eax, path1
		CALL F_SET_DATA_PATH
		mov retVal, eax
	}
	return retVal;
}



//_________________________________________________________________________________________
LONG FReadCfgInt(void* FileStream, const char* secName, const char* keyName, int* pIntVal) {

	LONG retVal = 0;
	__asm {
		mov ecx, pIntVal
		mov ebx, keyName
		mov edx, secName
		mov eax, FileStream
		CALL F_READ_CFG_INT
		mov retVal, eax
	}
	return retVal;
}


//_____________________________________________________________________________________________
LONG FReadCfgString(void* FileStream, const char* secName, const char* keyName, char** ppString) {

	LONG retVal = 0;
	__asm {
		mov ecx, ppString
		mov ebx, keyName
		mov edx, secName
		mov eax, FileStream
		CALL F_READ_CFG_STRING
		mov retVal, eax
	}
	return retVal;
}


//________________________________________________________________________________________
LONG FWriteCfgInt(void* FileStream, const char* secName, const char* keyName, int IntVal) {
	LONG retVal = 0;
	__asm {
		mov ecx, IntVal
		mov ebx, keyName
		mov edx, secName
		mov eax, FileStream
		CALL F_WRITE_CFG_INT
		mov retVal, eax
	}
	return retVal;
}


//____________________________________________________
void* F_fopen(const char* FileName, const char* flags) {

	void* retVal = nullptr;
	__asm {
		mov edx, flags
		mov eax, FileName
		CALL F_OPEN_FILE
		mov retVal, eax
	}
	return retVal;
}


//_____________________________
LONG F_fclose(void* FileStream) {

	LONG retVal = 0;
	__asm {
		mov eax, FileStream
		CALL F_CLOSE_FILE
		mov retVal, eax
	}
	return retVal;
}


//_______________________________________________________
LONG F_fseek(void* FileStream, LONG fOffset, LONG origin) {

	LONG retVal = 0;
	__asm {
		mov ebx, origin
		mov edx, fOffset
		mov eax, FileStream
		CALL F_FSEEK
		mov retVal, eax
	}
	return retVal;
}


//__________________________________________
LONG F_fread8(void* FileStream, BYTE* toMem) {

	LONG retVal = 0;
	__asm {
		mov edx, toMem
		mov eax, FileStream
		CALL F_READ_BYTE
		mov retVal, eax
	}
	return retVal;
}


//__________________________________________
LONG F_fread16(void* FileStream, WORD* toMem) {

	LONG retVal = 0;
	__asm {
		mov edx, toMem
		mov eax, FileStream
		CALL F_READ_WORD
		mov retVal, eax
	}
	return retVal;
}


//____________________________________________
LONG F_fread32(void* FileStream, DWORD* toMem) {

	LONG retVal = 0;
	__asm {
		mov edx, toMem
		mov eax, FileStream
		CALL F_READ_DWORD
		mov retVal, eax
	}
	return retVal;
}


//___________________________________________________________________
LONG F_fread16Array(void* FileStream, WORD* toMem, DWORD NumElements) {

	LONG retVal = 0;
	__asm {
		mov ebx, NumElements
		mov edx, toMem
		mov eax, FileStream
		CALL F_READ_WORD_ARRAY
		mov retVal, eax
	}
	return retVal;
}


//____________________________________________________________________
LONG F_fread32Array(void* FileStream, DWORD* toMem, DWORD NumElements) {

	LONG retVal = 0;
	__asm {
		mov ebx, NumElements
		mov edx, toMem
		mov eax, FileStream
		CALL F_READ_DWORD_ARRAY
		mov retVal, eax
	}
	return retVal;
}


//_________________________________________________________________________________
LONG FReadString(void* FileStream, char* toMem, DWORD charLength, DWORD NumStrings) {

	LONG retVal = 0;
	__asm {
		mov ecx, FileStream
		mov ebx, NumStrings
		mov edx, charLength
		mov eax, toMem
		CALL F_READ_STRING
		mov retVal, eax
	}
	return retVal;
}


//___________________________________________________________________
LONG FReadCharArray(void* FileStream, char* toMem, DWORD NumElements) {

	LONG retVal = 0;
	__asm {
		mov ebx, NumElements
		mov edx, toMem
		mov eax, FileStream
		CALL F_READ_CHAR_ARRAY
		mov retVal, eax
	}
	return retVal;
}



//___________________________
void FFileSetup(DWORD region) {

	if (region == 4) {
		F_OPEN_FILE = (void*)0x4C4F8C;
		F_CLOSE_FILE = (void*)0x4C4F49;
		F_FSEEK = (void*)0x4C537A;
		F_READ_BYTE = (void*)0x4C5444;
		F_READ_WORD = (void*)0x4C5495;
		F_READ_DWORD = (void*)0x4C5513;
		F_READ_WORD_ARRAY = (void*)0x4C5812;
		F_READ_DWORD_ARRAY = (void*)0x4C58BB;
		F_READ_STRING = (void*)0x4C524A;
		F_READ_CHAR_ARRAY = (void*)0x4C57A2;

		pCFG_FILE_PTR = (void*)0x59EED0;
		pCFG_FILE_NAME = (char*)0x59EEF8;

		F_READ_CFG_INT = (void*)0x42BC9C;
		F_READ_CFG_STRING = (void*)0x42BB88;
		F_WRITE_CFG_INT = (void*)0x42BDA0;

		F_SET_DATA_PATH = (void*)0x4C4BB9;

		F_WRITE_BYTE = (void*)0x4C561A;
		F_WRITE_WORD = (void*)0x4C5664;
		F_WRITE_DWORD = (void*)0x4C56C6;

		F_WRITE_DWORD_ARRAY = (void*)0x4C5B56;

		F_DELETE_SAVE_FILES = (void*)0x47F4B4;

		F_GET_FILE_LIST = (void*)0x4C5CCE;

		F_RELEASE_FILE_LIST = (void*)0x4C5F32;
	}
	else {
		F_OPEN_FILE = (void*)FixAddress(0x4C5EC8);
		F_CLOSE_FILE = (void*)FixAddress(0x4C5EB4);

		F_FSEEK = (void*)FixAddress(0x4DF5D8);


		F_READ_BYTE = (void*)FixAddress(0x4C60E0);
		F_READ_WORD = (void*)FixAddress(0x4C60F4);
		F_READ_DWORD = (void*)FixAddress(0x4C614C);
		F_READ_WORD_ARRAY = (void*)FixAddress(0x4C6330);
		F_READ_DWORD_ARRAY = (void*)FixAddress(0x4C63BC);
		F_READ_STRING = (void*)FixAddress(0x4C5FFC);

		F_READ_CHAR_ARRAY = (void*)FixAddress(0x4C62FC);

		pCFG_FILE_PTR = (void*)FixAddress(0x58E950);
		pCFG_FILE_NAME = (char*)FixAddress(0x58E978);

		F_READ_CFG_INT = (void*)FixAddress(0x42C05C);
		F_READ_CFG_STRING = (void*)FixAddress(0x42BF48);
		F_WRITE_CFG_INT = (void*)FixAddress(0x42C160);
		F_SET_DATA_PATH = (void*)FixAddress(0x4C5D30);

		F_WRITE_BYTE = (void*)FixAddress(0x4C61AC);
		F_WRITE_WORD = (void*)FixAddress(0x4C61C8);
		F_WRITE_DWORD = (void*)FixAddress(0x4C6214);

		F_WRITE_DWORD_ARRAY = (void*)FixAddress(0x4C64F8);

		F_DELETE_SAVE_FILES = (void*)FixAddress(0x480040);

		F_GET_FILE_LIST = (void*)FixAddress(0x4C6628);

		F_RELEASE_FILE_LIST = (void*)FixAddress(0x4C6868);
	}
}
