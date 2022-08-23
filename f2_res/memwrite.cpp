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

using namespace std;


int REGION;
const char* regionText[] = {
	"UNKNOWN",
	"US 1.02d",
	"UK 1.02e",
	"French or German 1.02d",
	"Chinese 1.02",
	"Russian Lev Corp"
};


class ERRORdata {
public:
	DWORD type;
	DWORD offset;
	DWORD length;
	unsigned char* expected_data;
	unsigned char* found_data;
	char* msg;
	ERRORdata() {
		type = 0;
		offset = 0;
		length = 0;
		expected_data = nullptr;
		found_data = nullptr;
		msg = nullptr;
	};
	ERRORdata(const ERRORdata& from) {
		type = from.type;
		offset = from.offset;
		length = from.length;
		if (from.expected_data) {
			expected_data = new unsigned char[length];
			memcpy(expected_data, from.expected_data, length);
		}
		if (from.found_data) {
			found_data = new unsigned char[length];
			memcpy(found_data, from.found_data, length);
		}
		if (from.msg) {
			msg = new char[128];
			memset(msg, '/0', 128);
			strncpy(msg, from.msg, 127);
		}
	};
	~ERRORdata() {
		if (expected_data)
			delete[] expected_data;
		expected_data = nullptr;
		if (found_data)
			delete[] found_data;
		found_data = nullptr;
		if (msg)
			delete[] msg;
		msg = nullptr;
	};
};


class ERRORS {
private:
	std::vector <ERRORdata> errorData_Vector;
public:
	void RecordGeneral(const char* msg);
	void RecordMemMisMatch(DWORD offset, unsigned char* original, unsigned char* change_to, DWORD length);
	void PrintAll();
};


//_________________________________________
void ERRORS::RecordGeneral(const char* msg) {

	ERRORdata errordata;

	errordata.type = 0;
	errordata.msg = new char[128];
	memset(errordata.msg, '/0', 128);
	strncpy(errordata.msg, msg, 127);
	errorData_Vector.push_back(errordata);
}


//___________________________________________________________________________________________________________
void ERRORS::RecordMemMisMatch(DWORD offset, unsigned char* original, unsigned char* change_to, DWORD length) {

	ERRORdata errordata;

	errordata.type = 1;
	errordata.offset = offset;
	errordata.length = length;
	errordata.expected_data = original;
	errordata.found_data = new unsigned char[length];
	memcpy(errordata.found_data, (void*)offset, length);
	errorData_Vector.push_back(errordata);
}


//_____________________
void ERRORS::PrintAll() {

	if (errorData_Vector.size() > 0) {
		fstream inFile("f2_res_error.log", fstream::out);// open file
		if (inFile.is_open()) {
			inFile << endl << "Fallout2 Hi-Res Patch Error Log." << endl;
			inFile << endl << "fallout2.exe version: " << regionText[REGION] << "." << endl;

			inFile << endl << errorData_Vector.size() << " Error(s) encountered." << endl << endl << endl;
			for (unsigned int i = 0; i < errorData_Vector.size(); i++) {
				if (errorData_Vector[i].type == 0) {
					if (errorData_Vector[i].msg)
						inFile << "Error: " << errorData_Vector[i].msg << endl;
					inFile << endl << endl;
				}
				else if (errorData_Vector[i].type == 1) {
					inFile << "Memory offset:  0x" << uppercase << hex << errorData_Vector[i].offset << endl;
					inFile << "Expected data:  ";
					unsigned int e;
					for (e = 0; e < errorData_Vector[i].length; e++) {
						if ((int)errorData_Vector[i].expected_data[e] < 16)
							inFile << "0" << hex << (int)errorData_Vector[i].expected_data[e] << " ";
						else
							inFile << hex << (int)errorData_Vector[i].expected_data[e] << " ";
					}
					inFile << endl;
					inFile << "Found data:     ";
					for (e = 0; e < errorData_Vector[i].length; e++) {
						if ((int)errorData_Vector[i].found_data[e] < 16)
							inFile << "0" << hex << (int)errorData_Vector[i].found_data[e] << " ";
						else
							inFile << hex << (int)errorData_Vector[i].found_data[e] << " ";
					}
					inFile << endl << endl;
				}
			}
		}
		else MessageBox(nullptr, "Can't create f2_res_error.log", "File Error", MB_ICONERROR);
	}
}


ERRORS errors;


struct OFFSETdiff {
	int num;
	DWORD* address;
	DWORD* offset;
	OFFSETdiff() {
		num = 0;
		address = nullptr;
		offset = nullptr;
	}
	~OFFSETdiff() {
		num = 0;
		delete[]address;
		delete[]offset;
	}

};

OFFSETdiff* offsetDiff = nullptr;


//________________________________
DWORD FixAddress(DWORD memAddress) {

	if (!offsetDiff)return memAddress;

	DWORD tmpMemAddess = memAddress;

	if (memAddress < 0x410000)
		tmpMemAddess += 0x410000;

	int next = offsetDiff->num - 1;
	while (tmpMemAddess < offsetDiff->address[next] && next>0) {
		next--;
	}
	memAddress = memAddress + offsetDiff->offset[next];

	return memAddress;
}


//______________________
void SetAddressDiffsUK() {

	offsetDiff = new OFFSETdiff;
	offsetDiff->num = 6;
	offsetDiff->address = new DWORD[offsetDiff->num];
	offsetDiff->offset = new DWORD[offsetDiff->num];

	offsetDiff->address[0] = 0;
	offsetDiff->offset[0] = 0;
	offsetDiff->address[1] = 0x442B84;
	offsetDiff->offset[1] = +432;
	offsetDiff->address[2] = 0x500000;
	offsetDiff->offset[2] = +80;

	offsetDiff->address[3] = 0x50C3A8;
	offsetDiff->offset[3] = +92;
	offsetDiff->address[4] = 0x50C3AC;
	offsetDiff->offset[4] = +80;

	offsetDiff->address[5] = 0x56DBAC;
	offsetDiff->offset[5] = 0;

	//offsetDiff->address[3] = 0x570000;
	//offsetDiff->offset[3] = 0;

	//offsetDiff->address[3] = 0x58E000;//600000;
	//offsetDiff->offset[3] = 0;
}


//_________________________
void SetAddressDiffsFR_GR() {

	offsetDiff = new OFFSETdiff;
	offsetDiff->num = 8;
	offsetDiff->address = new DWORD[offsetDiff->num];
	offsetDiff->offset = new DWORD[offsetDiff->num];

	offsetDiff->address[0] = 0;
	offsetDiff->offset[0] = 0;
	offsetDiff->address[1] = 0x442B84;
	offsetDiff->offset[1] = +432;
	offsetDiff->address[2] = 0x4551E1;
	offsetDiff->offset[2] = +500;
	offsetDiff->address[3] = 0x45D880;
	offsetDiff->offset[3] = +496;
	offsetDiff->address[4] = 0x483188;
	offsetDiff->offset[4] = +728;
	offsetDiff->address[5] = 0x4845B0;
	offsetDiff->offset[5] = +720;
	offsetDiff->address[6] = 0x500000;
	offsetDiff->offset[6] = +128;
	offsetDiff->address[7] = 0x56DBAC;
	offsetDiff->offset[7] = 0;
	//offsetDiff->address[7] = 0x570000;
	//offsetDiff->offset[7] = 0;
	//offsetDiff->address[7] = 0x58E000;//6A0000;
	//offsetDiff->offset[7] = 0;
}


//_________________________
void SetAddressDiffsRU_LC() {

	offsetDiff = new OFFSETdiff;
	offsetDiff->num = 22;
	offsetDiff->address = new DWORD[offsetDiff->num];
	offsetDiff->offset = new DWORD[offsetDiff->num];

	offsetDiff->address[0] = 0;
	offsetDiff->offset[0] = 0;
	offsetDiff->address[1] = 0x452804;
	offsetDiff->offset[1] = -196;
	offsetDiff->address[2] = 0x452970;
	offsetDiff->offset[2] = -192;
	offsetDiff->address[3] = 0x4541C8;
	offsetDiff->offset[3] = -188;
	offsetDiff->address[4] = 0x45967D;
	offsetDiff->offset[4] = -190;
	offsetDiff->address[5] = 0x4598BC;
	offsetDiff->offset[5] = -192;
	offsetDiff->address[6] = 0x4836DC;
	offsetDiff->offset[6] = -199;
	offsetDiff->address[7] = 0x483784;
	offsetDiff->offset[7] = -200;
	offsetDiff->address[8] = 0x4845B0;
	offsetDiff->offset[8] = -208;
	offsetDiff->address[9] = 0x488DAA;
	offsetDiff->offset[9] = -226;
	offsetDiff->address[10] = 0x48909C;
	offsetDiff->offset[10] = -224;
	offsetDiff->address[11] = 0x494504;
	offsetDiff->offset[11] = -234;
	offsetDiff->address[12] = 0x49460C;
	offsetDiff->offset[12] = -232;
	offsetDiff->address[13] = 0x494D7C;
	offsetDiff->offset[13] = -296;
	offsetDiff->address[14] = 0x4961B0;
	offsetDiff->offset[14] = -304;
	offsetDiff->address[15] = 0x4B1BA2;
	offsetDiff->offset[15] = -311;
	offsetDiff->address[16] = 0x4B1D20;
	offsetDiff->offset[16] = -312;
	offsetDiff->address[17] = 0x4B39F0;
	offsetDiff->offset[17] = -304;
	offsetDiff->address[18] = 0x500000;
	offsetDiff->offset[18] = -16;
	offsetDiff->address[19] = 0x50C3A8;
	offsetDiff->offset[19] = 0;
	offsetDiff->address[20] = 0x50C804;
	offsetDiff->offset[20] = -16;
	offsetDiff->address[21] = 0x56DBAC;
	offsetDiff->offset[21] = 0;
	//offsetDiff->address[19] = 0x570000;
	//offsetDiff->offset[19] = 0;
	//offsetDiff->address[19] = 0x58E000;//6A0000;
	//offsetDiff->offset[19] = 0;
}


//_______________________________________________________________
void FuncWrite32(DWORD offset, DWORD original, DWORD funcAddress) {

	offset = FixAddress(offset);
	funcAddress = funcAddress - (offset + 4);

	if (*(DWORD*)offset != original)
		errors.RecordMemMisMatch(offset, itoha(original), itoha(funcAddress), 4);
	else {
		DWORD oldProtect;
		VirtualProtect((LPVOID)offset, 4 + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
		*(DWORD*)offset = funcAddress;
		VirtualProtect((LPVOID)offset, 4 + 1, oldProtect, &oldProtect);
	}
}


//_____________________________________________________
void FuncWriteUnSafe32(DWORD offset, DWORD funcAddress) {

	offset = FixAddress(offset);
	funcAddress = funcAddress - (offset + 4);

	DWORD oldProtect;
	VirtualProtect((LPVOID)offset, 4 + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	*(DWORD*)offset = funcAddress;
	VirtualProtect((LPVOID)offset, 4 + 1, oldProtect, &oldProtect);
}


//_________________________________________________________________
void FuncReplace32(DWORD offset, DWORD original, DWORD funcAddress) {

	original = original + (offset + 4);
	offset = FixAddress(offset);
	funcAddress = FixAddress(funcAddress) - (offset + 4);
	original = FixAddress(original) - (offset + 4);


	if (*(DWORD*)offset != original)
		errors.RecordMemMisMatch(offset, itoha(original), itoha(funcAddress), 4);
	else {
		DWORD oldProtect;
		VirtualProtect((LPVOID)offset, 4 + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
		*(DWORD*)offset = funcAddress;
		VirtualProtect((LPVOID)offset, 4 + 1, oldProtect, &oldProtect);
	}
}


//_________________________________________________________________________________________________
void MemWriteString(DWORD  offset, unsigned char* original, unsigned char* change_to, DWORD length)
{
	offset = FixAddress(offset);

	if (memcmp((void*)offset, original, length) != 0)
		errors.RecordMemMisMatch(offset, original, change_to, length);
	else {
		DWORD oldProtect;
		VirtualProtect((LPVOID)offset, length + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy((void*)offset, change_to, length);
		VirtualProtect((LPVOID)offset, length + 1, oldProtect, &oldProtect);
	}
}


//____________________________________________________________
void MemWrite32(DWORD offset, DWORD original, DWORD change_to) {

	offset = FixAddress(offset);

	if (*(DWORD*)offset != original)
		errors.RecordMemMisMatch(offset, itoha(original), itoha(change_to), 4);
	else {
		DWORD oldProtect;
		VirtualProtect((LPVOID)offset, 4 + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
		*(DWORD*)offset = change_to;
		VirtualProtect((LPVOID)offset, 4 + 1, oldProtect, &oldProtect);
	}
}


//__________________________________________________
void MemWriteUnSafe32(DWORD offset, DWORD change_to) {

	offset = FixAddress(offset);

	DWORD oldProtect;
	VirtualProtect((LPVOID)offset, 4 + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	*(DWORD*)offset = change_to;
	VirtualProtect((LPVOID)offset, 4 + 1, oldProtect, &oldProtect);
}


//__________________________________________________________
void MemWrite16(DWORD offset, WORD original, WORD change_to) {

	offset = FixAddress(offset);

	if (*(WORD*)offset != original)
		errors.RecordMemMisMatch(offset, itoha(original), itoha(change_to), 2);
	else {
		DWORD oldProtect;
		VirtualProtect((LPVOID)offset, 2 + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
		*(WORD*)offset = change_to;
		VirtualProtect((LPVOID)offset, 2 + 1, oldProtect, &oldProtect);
	}
}


//_________________________________________________________
void MemWrite8(DWORD offset, BYTE original, BYTE change_to) {

	offset = FixAddress(offset);

	if (*(BYTE*)offset != original)
		errors.RecordMemMisMatch(offset, itoha(original), itoha(change_to), 1);
	else {
		DWORD oldProtect;
		VirtualProtect((LPVOID)offset, 1 + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
		*(BYTE*)offset = change_to;
		VirtualProtect((LPVOID)offset, 1 + 1, oldProtect, &oldProtect);
	}
}


//________________________________________________
void MemWriteUnSafe8(DWORD offset, BYTE change_to) {

	offset = FixAddress(offset);

	DWORD oldProtect;
	VirtualProtect((LPVOID)offset, 1 + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	*(BYTE*)offset = change_to;
	VirtualProtect((LPVOID)offset, 1 + 1, oldProtect, &oldProtect);

}


//_________________________________________________________________________________________________
void MemBlt8(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth) {

	for (int h = 0; h < subHeight; h++) {
		memcpy(toBuff, fromBuff, subWidth);
		fromBuff += fromWidth;
		toBuff += toWidth;
	}
}


//_______________________________________________________________________________________________________
void MemBltMasked8(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth) {

	for (int h = 0; h < subHeight; h++) {
		//memcpy(toBuff, fromBuff, subWidth);
		for (int w = 0; w < subWidth; w++) {
			if (fromBuff[w] != 0)
				toBuff[w] = fromBuff[w];
		}

		fromBuff += fromWidth;
		toBuff += toWidth;
	}
}


//_________________________________________________________________________________________________________________________________________________
void MemBlt8Stretch(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth, int toHeight, bool ARatio, bool centred) {

	float toWidthAR = (float)toWidth, toHeightAR = (float)toHeight;

	if (ARatio) {

		float imageRO = (float)subWidth / subHeight;
		float winRO = (float)toWidth / toHeight;

		if (winRO > imageRO) {
			toWidthAR = toHeightAR / subHeight * subWidth;
			if (centred)
				toBuff += (toWidth - (int)toWidthAR) / 2;
		}
		else if (winRO < imageRO) {
			toHeightAR = toWidthAR / subWidth * subHeight;
			if (centred)
				toBuff += ((toHeight - (int)toHeightAR) / 2) * toWidth;
		}
	}

	float pWidth = subWidth / toWidthAR;
	float pHeight = subHeight / toHeightAR;

	float fx = 0, fy = 0;
	int fyMul = 0;

	for (int ty = 0; ty < (int)toHeightAR; ty++) {
		fx = 0;
		for (int tx = 0; tx < (int)toWidthAR; tx++) {//draw stretched line
			toBuff[tx] = fromBuff[fyMul + (int)fx];
			fx += pWidth;
			if (fx >= subWidth)
				fx = (float)subWidth - 1;
		}
		fy += pHeight;
		if ((int)fy >= subHeight)
			fy = (float)subHeight - 1;
		fyMul = (int)fy * fromWidth;
		toBuff += toWidth;
	}

}


//_____________________
void print_mem_errors() {
	errors.PrintAll();
}


//____________________
int region_check(void) {

	if (*(DWORD*)0x476B0D == 0x519078) {
		REGION = 1;
	}
	else if (*(DWORD*)0x476CBD == 0x5190C8) {
		REGION = 2;
		SetAddressDiffsUK();
	}
	else if (*(DWORD*)0x476CFD == 0x5190F8) {
		REGION = 3;
		SetAddressDiffsFR_GR();
	}
	else if (*(DWORD*)0x476109 == 0x528E68) {
		REGION = 4;
	}
	else if (*(DWORD*)0x476A4D == 0x519068) {
		REGION = 5;
		SetAddressDiffsRU_LC();
	}
	else {
		REGION = 0;
	}

	return REGION;
}
