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


class ERRORdata {//bytes big-endian
public:
	DWORD offset;
	DWORD length;
	unsigned char* expected_data;
	unsigned char* found_data;
};


std::vector <ERRORdata> errordata_Vector;


int REGION;
const char* regionText[] = {
	"UNKNOWN",
	"TEAMX 1.2",
	"US 1.1",
	"Polish 1.2",
	"Collection Edition 1.2",
};



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


//________________________
void SetAddressDiffsUS11() {

	offsetDiff = new OFFSETdiff;
	offsetDiff->num = 56;
	offsetDiff->address = new DWORD[offsetDiff->num];
	offsetDiff->offset = new DWORD[offsetDiff->num];

	offsetDiff->address[0] = 0;
	offsetDiff->offset[0] = 0;
	offsetDiff->address[1] = 0x4182DC;
	offsetDiff->offset[1] = -100;
	offsetDiff->address[2] = 0x41943D;
	offsetDiff->offset[2] = -388;
	offsetDiff->address[3] = 0x419520;
	offsetDiff->offset[3] = -384;
	offsetDiff->address[4] = 0x42385D;
	offsetDiff->offset[4] = -612;
	offsetDiff->address[5] = 0x4246C0;
	offsetDiff->offset[5] = -624;
	offsetDiff->address[6] = 0x42E3F9;
	offsetDiff->offset[6] = -623;
	offsetDiff->address[7] = 0x42E625;
	offsetDiff->offset[7] = -681;
	offsetDiff->address[8] = 0x42E6DC;
	offsetDiff->offset[8] = -680;
	offsetDiff->address[9] = 0x437E20;
	offsetDiff->offset[9] = -672;
	offsetDiff->address[10] = 0x43B47E;
	offsetDiff->offset[10] = -817;
	offsetDiff->address[11] = 0x43B8DC;
	offsetDiff->offset[11] = -820;
	offsetDiff->address[12] = 0x43BB16;
	offsetDiff->offset[12] = -810;
	offsetDiff->address[13] = 0x43C32E;
	offsetDiff->offset[13] = -818;
	offsetDiff->address[14] = 0x43C60E;
	offsetDiff->offset[14] = -943;
	offsetDiff->address[15] = 0x43C934;
	offsetDiff->offset[15] = -944;
	offsetDiff->address[16] = 0x43D9F1;
	offsetDiff->offset[16] = -1091;
	offsetDiff->address[17] = 0x43DA05;
	offsetDiff->offset[17] = -1089;
	offsetDiff->address[18] = 0x43DA21;
	offsetDiff->offset[18] = -1093;
	offsetDiff->address[19] = 0x43DA40;
	offsetDiff->offset[19] = -1103;
	offsetDiff->address[20] = 0x43DA5D;
	offsetDiff->offset[20] = -1100;
	offsetDiff->address[21] = 0x43DA75;
	offsetDiff->offset[21] = -1113;
	offsetDiff->address[22] = 0x43DAF0;
	offsetDiff->offset[22] = -1120;
	offsetDiff->address[23] = 0x47044E;
	offsetDiff->offset[23] = -1125;
	offsetDiff->address[24] = 0x4707B4;
	offsetDiff->offset[24] = -1124;
	offsetDiff->address[25] = 0x472A10;
	offsetDiff->offset[25] = -1120;
	offsetDiff->address[26] = 0x472ADE;
	offsetDiff->offset[26] = -1122;
	offsetDiff->address[27] = 0x472B14;
	offsetDiff->offset[27] = -1116;
	offsetDiff->address[28] = 0x472BB0;
	offsetDiff->offset[28] = -1113;
	offsetDiff->address[29] = 0x472D10;
	offsetDiff->offset[29] = -1092;
	offsetDiff->address[30] = 0x4733C0;
	offsetDiff->offset[30] = -1088;
	offsetDiff->address[31] = 0x476CF1;
	offsetDiff->offset[31] = -1167;
	offsetDiff->address[32] = 0x476E28;
	offsetDiff->offset[32] = -1168;
	offsetDiff->address[33] = 0x4825AD;
	offsetDiff->offset[33] = -1169;
	offsetDiff->address[34] = 0x48263E;
	offsetDiff->offset[34] = -1170;
	offsetDiff->address[35] = 0x482A40;
	offsetDiff->offset[35] = -1172;
	offsetDiff->address[36] = 0x485530;
	offsetDiff->offset[36] = -1184;
	offsetDiff->address[37] = 0x48AB35;
	offsetDiff->offset[37] = -1176;
	offsetDiff->address[38] = 0x48CD1C;
	offsetDiff->offset[38] = -1184;
	offsetDiff->address[39] = 0x4971E9;
	offsetDiff->offset[39] = -1290;
	offsetDiff->address[40] = 0x497270;
	offsetDiff->offset[40] = -1296;
	offsetDiff->address[41] = 0x4AF1D4;
	offsetDiff->offset[41] = -1316;
	offsetDiff->address[42] = 0x4AF2EF;
	offsetDiff->offset[42] = -1332;
	offsetDiff->address[43] = 0x4AF3C0;
	offsetDiff->offset[43] = -1328;


	offsetDiff->address[44] = 0x4D2B6F;
	offsetDiff->offset[44] = -1399;

	offsetDiff->address[45] = 0x4D2C1F;
	offsetDiff->offset[45] = -1328;

	offsetDiff->address[46] = 0x4FB49C;
	offsetDiff->offset[46] = -192;
	offsetDiff->address[47] = 0x4FC994;
	offsetDiff->offset[47] = -200;

	offsetDiff->address[48] = 0x4FEBA4;
	offsetDiff->offset[48] = -240;

	offsetDiff->address[49] = 0x500000;
	offsetDiff->offset[49] = -244;
	offsetDiff->address[50] = 0x505E00;
	offsetDiff->offset[50] = -240;
	offsetDiff->address[51] = 0x560000;
	offsetDiff->offset[51] = -32;

	offsetDiff->address[52] = 0x570000;
	offsetDiff->offset[52] = 0;
	offsetDiff->address[53] = 0x580000;
	offsetDiff->offset[53] = -32;

	offsetDiff->address[54] = 0x660000;
	offsetDiff->offset[54] = -32;
	offsetDiff->address[55] = 0x6C0000;
	offsetDiff->offset[55] = 0;

	//offsetDiff->address[3] = 0x58E000;//600000;
	//offsetDiff->offset[3] = 0;
}


//_________________________
void SetAddressDiffsPOL12() {

	offsetDiff = new OFFSETdiff;
	offsetDiff->num = 47;
	offsetDiff->address = new DWORD[offsetDiff->num];
	offsetDiff->offset = new DWORD[offsetDiff->num];

	offsetDiff->address[0] = 0;
	offsetDiff->offset[0] = 0;
	offsetDiff->address[1] = 0x42E3F9;
	offsetDiff->offset[1] = +1;
	offsetDiff->address[2] = 0x42E640;
	offsetDiff->offset[2] = -57;
	offsetDiff->address[3] = 0x42E6DC;
	offsetDiff->offset[3] = -56;
	offsetDiff->address[4] = 0x437E20;
	offsetDiff->offset[4] = -48;
	offsetDiff->address[5] = 0x43B47E;
	offsetDiff->offset[5] = -193;
	offsetDiff->address[6] = 0x43B8DC;
	offsetDiff->offset[6] = -196;
	offsetDiff->address[7] = 0x43BB16;
	offsetDiff->offset[7] = -186;
	offsetDiff->address[8] = 0x43C2E0;
	offsetDiff->offset[8] = -191;
	offsetDiff->address[9] = 0x43C31B;
	offsetDiff->offset[9] = -194;
	offsetDiff->address[10] = 0x43C60E;
	offsetDiff->offset[10] = -319;
	offsetDiff->address[11] = 0x43C934;
	offsetDiff->offset[11] = -320;
	offsetDiff->address[12] = 0x43DA0D;
	offsetDiff->offset[12] = -465;
	offsetDiff->address[13] = 0x43DA5D;
	offsetDiff->offset[13] = -486;
	offsetDiff->address[14] = 0x43DA75;
	offsetDiff->offset[14] = -489;
	offsetDiff->address[15] = 0x43DAF0;
	offsetDiff->offset[15] = -496;
	offsetDiff->address[16] = 0x44C8E3;
	offsetDiff->offset[16] = -492;
	offsetDiff->address[17] = 0x44C93D;
	offsetDiff->offset[17] = -424;
	offsetDiff->address[18] = 0x453650;
	offsetDiff->offset[18] = -432;
	offsetDiff->address[19] = 0x472AE3;
	offsetDiff->offset[19] = -434;
	offsetDiff->address[20] = 0x472B14;
	offsetDiff->offset[20] = -428;
	offsetDiff->address[21] = 0x472BB0;
	offsetDiff->offset[21] = -425;
	offsetDiff->address[22] = 0x472CE0;
	offsetDiff->offset[22] = -407;
	offsetDiff->address[23] = 0x472D10;
	offsetDiff->offset[23] = -404;
	offsetDiff->address[24] = 0x4733C0;
	offsetDiff->offset[24] = -400;
	offsetDiff->address[25] = 0x475070;
	offsetDiff->offset[25] = -396;
	offsetDiff->address[26] = 0x476760;
	offsetDiff->offset[26] = -400;
	offsetDiff->address[27] = 0x476CF1;
	offsetDiff->offset[27] = -479;
	offsetDiff->address[28] = 0x476E28;
	offsetDiff->offset[28] = -480;
	offsetDiff->address[29] = 0x4825AD;
	offsetDiff->offset[29] = -481;
	offsetDiff->address[30] = 0x48263E;
	offsetDiff->offset[30] = -482;
	offsetDiff->address[31] = 0x4828F9;
	offsetDiff->offset[31] = -483;
	offsetDiff->address[32] = 0x48295E;
	offsetDiff->offset[32] = -482;
	offsetDiff->address[33] = 0x4829B0;
	offsetDiff->offset[33] = -483;
	offsetDiff->address[34] = 0x482A40;
	offsetDiff->offset[34] = -484;
	offsetDiff->address[35] = 0x485530;
	offsetDiff->offset[35] = -496;
	offsetDiff->address[36] = 0x48AB36;
	offsetDiff->offset[36] = -488;
	offsetDiff->address[37] = 0x48CD1C;
	offsetDiff->offset[37] = -496;
	offsetDiff->address[38] = 0x4971E9;
	offsetDiff->offset[38] = -602;
	offsetDiff->address[39] = 0x497223;
	offsetDiff->offset[39] = -605;
	offsetDiff->address[40] = 0x497270;
	offsetDiff->offset[40] = -608;
	offsetDiff->address[41] = 0x4AF1D4;
	offsetDiff->offset[41] = -628;
	offsetDiff->address[42] = 0x4AF2EF;
	offsetDiff->offset[42] = -644;
	offsetDiff->address[43] = 0x4AF3C0;
	offsetDiff->offset[43] = -640;

	   //offsetDiff->address[43] = 0x500000;
	offsetDiff->address[44] = 0x4FB49C;
	offsetDiff->offset[44] = -160;
	offsetDiff->address[45] = 0x4FEB44;
	offsetDiff->offset[45] = -208;
	offsetDiff->address[46] = 0x560000;
	offsetDiff->offset[46] = 0;
}


//_________________________
void SetAddressDiffsCOL12() {

	offsetDiff = new OFFSETdiff;
	offsetDiff->num = 6;
	offsetDiff->address = new DWORD[offsetDiff->num];
	offsetDiff->offset = new DWORD[offsetDiff->num];

	offsetDiff->address[0] = 0;
	offsetDiff->offset[0] = 0;
	offsetDiff->address[1] = 0x44C939;
	offsetDiff->offset[1] = +72;
	offsetDiff->address[2] = 0x453650;
	offsetDiff->offset[2] = +64;
	offsetDiff->address[3] = 0x475070;
	offsetDiff->offset[3] = +68;
	offsetDiff->address[4] = 0x476760;
	offsetDiff->offset[4] = +64;
	//offsetDiff->address[5] = 0x500000;
	offsetDiff->address[5] = 0x4F0000;
	offsetDiff->offset[5] = 0;
}


//_____________________________________________________________________________________________
void RecordError(DWORD offset, unsigned char* original, unsigned char* change_to, DWORD length) {

	ERRORdata errordata{0};

	errordata.offset = offset;
	errordata.length = length;
	errordata.expected_data = original;
	errordata.found_data = new unsigned char[length];
	memcpy(errordata.found_data, (void*)offset, length);
	errordata_Vector.push_back(errordata);
}


//_______________________________________________________________
void FuncWrite32(DWORD offset, DWORD original, DWORD funcAddress) {

	offset = FixAddress(offset);
	funcAddress = funcAddress - (offset + 4);

	if (*(DWORD*)offset != original)
		RecordError(offset, itoha(original), itoha(funcAddress), 4);
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
		RecordError(offset, itoha(original), itoha(funcAddress), 4);
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
		RecordError(offset, original, change_to, length);
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
		RecordError(offset, itoha(original), itoha(change_to), 4);
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
		RecordError(offset, itoha(original), itoha(change_to), 2);
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
		RecordError(offset, itoha(original), itoha(change_to), 1);
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
			//txPos=(float)((toWidth-(int)toWidthAR)/2);
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
	if (errordata_Vector.size() > 0) {
		MessageBox(nullptr, "Memory Mismatch check f1_res_error.log for details", "Hi-Res patch Error", MB_ICONERROR);
		fstream inFile("f1_res_error.log", fstream::out);// open file
		if (inFile.is_open()) {
			inFile << endl << "Fallout Hi-Res Patch Error Log." << endl;
			inFile << endl << "falloutw.exe version: " << regionText[REGION] << "." << endl;
			inFile << endl << errordata_Vector.size() << " memory mismatch error(s) encountered." << endl << endl << endl;
			for (unsigned int i = 0; i < errordata_Vector.size(); i++) {
				inFile << "Memory offset:  0x" << uppercase << hex << errordata_Vector[i].offset << endl;
				inFile << "Expected data:  ";
				unsigned int e;
				for (e = 0; e < errordata_Vector[i].length; e++) {
					if ((int)errordata_Vector[i].expected_data[e] < 16)
						inFile << "0" << hex << (int)errordata_Vector[i].expected_data[e] << " ";
					else
						inFile << hex << (int)errordata_Vector[i].expected_data[e] << " ";
				}
				inFile << endl;
				inFile << "Found data:     ";
				for (e = 0; e < errordata_Vector[i].length; e++) {
					if ((int)errordata_Vector[i].found_data[e] < 16)
						inFile << "0" << hex << (int)errordata_Vector[i].found_data[e] << " ";
					else
						inFile << hex << (int)errordata_Vector[i].found_data[e] << " ";
				}
				inFile << endl << endl;
			}
		}
		else MessageBox(nullptr, "Can't create f1_res_error.log", "File Error", MB_ICONERROR);
	}
}


//____________________
int region_check(void) {

	if (*(DWORD*)0x469A99 == 0x505754) {
		REGION = 1;
	}
	else if (*(DWORD*)0x469639 == 0x505660) {
		REGION = 2;
		SetAddressDiffsUS11();
	}
	else if (*(DWORD*)0x4698E9 == 0x505684) {
		REGION = 3;
		SetAddressDiffsPOL12();
	}
	else if (*(DWORD*)0x469AD9 == 0x505754) {
		REGION = 4;
		SetAddressDiffsCOL12();
	}
	else {
		REGION = 0;
	}

	return REGION;
}
