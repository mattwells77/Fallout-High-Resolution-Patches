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
#include "F_File.h"
#include "convert.h"
#include "memwrite.h"
#include "F_Windows.h"
#include "configTools.h"
#include "WinFall.h"


LONG splashWinRef = -1;
int SPLASH_SCRN_SIZE = 0;

int SPLASH_DELAY = 0;

class IMAGE8 {
public:
	DWORD width;
	DWORD height;
	DWORD size;//in bytes
	BYTE pal[768];
	BYTE* buff;
	IMAGE8() {
		width = 0;
		height = 0;
		size = 0;
		memset(pal, 0, 768);
		buff = nullptr;
	}
	~IMAGE8() {
		delete[] buff;
	}
};


//__________________________________________
bool LoadRIX(IMAGE8* rixIMG, char* fileName) {

	void* FileStream = F_fopen(fileName, "rb");
	if (FileStream == nullptr)return false;

	DWORD RixType = 0;
	F_fread32(FileStream, &RixType);
	if (RixType != 0x52495833) {//"RIX3"
		F_fclose(FileStream);
		//MessageBox( nullptr, "Not RIX3", fileName, MB_ICONERROR);
		return false;
	}

	WORD wordVal = 0;
	F_fread16(FileStream, &wordVal);
	rixIMG->width = ByteSwap16(wordVal);
	F_fread16(FileStream, &wordVal);
	rixIMG->height = ByteSwap16(wordVal);
	F_fread16(FileStream, &wordVal);//unknown

	rixIMG->size = rixIMG->width * rixIMG->height;

	FReadCharArray(FileStream, (char*)&rixIMG->pal[0], 768);

	rixIMG->buff = new BYTE[rixIMG->size];
	FReadCharArray(FileStream, (char*)rixIMG->buff, rixIMG->size);

	F_fclose(FileStream);

	return true;
}


//__________________________________________
bool LoadBMP(IMAGE8* bmpIMG, char* fileName) {

	void* FileStream = F_fopen(fileName, "rb");
	if (FileStream == nullptr)return false;

	BYTE byteVal = 0;
	WORD wordVal = 0;
	DWORD dwordVal = 0;

	//read bmp header
	F_fread16(FileStream, &wordVal);
	if (wordVal != 0x424D) {//"BM"
		F_fclose(FileStream);
		//MessageBox( nullptr, "Not a BMP", fileName, MB_ICONERROR);
		return false;
	}
	DWORD buffSize = 0;
	F_fread32(FileStream, &buffSize);//bmp size
	buffSize = ByteSwap32(buffSize);

	F_fseek(FileStream, 10, 0);
	DWORD buffOffset = 0;
	F_fread32(FileStream, &buffOffset);//data offset
	buffOffset = ByteSwap32(buffOffset);

	buffSize -= buffOffset;

	DWORD dibHeaderSize = 0;
	//read dib header
	F_fread32(FileStream, &dibHeaderSize);
	dibHeaderSize = ByteSwap32(dibHeaderSize);

	F_fread32(FileStream, &bmpIMG->width);
	bmpIMG->width = ByteSwap32(bmpIMG->width);

	F_fread32(FileStream, &bmpIMG->height);
	bmpIMG->height = ByteSwap32(bmpIMG->height);

	bmpIMG->size = bmpIMG->width * bmpIMG->height;


	F_fread16(FileStream, &wordVal);//colour planes
	WORD pixelSize = 0;
	F_fread16(FileStream, &pixelSize);//bits per pixel
	pixelSize = ByteSwap16(pixelSize);


	if (pixelSize != 8) {
		F_fclose(FileStream);
		//MessageBox( nullptr, "Must be 8bit BMP", fileName, MB_ICONERROR);
		return false;
	}

	//load palette
	F_fseek(FileStream, (LONG)(0x0E + dibHeaderSize), 0);

	int i = 0;
	for (i = 0; i < 768; i += 3) {
		F_fread8(FileStream, &bmpIMG->pal[i + 2]);
		F_fread8(FileStream, &bmpIMG->pal[i + 1]);
		F_fread8(FileStream, &bmpIMG->pal[i + 0]);
		F_fread8(FileStream, &byteVal);

		//divide pal colours by 4 to darken for fallout fade effects
		bmpIMG->pal[i + 2] = bmpIMG->pal[i + 2] / 4;
		bmpIMG->pal[i + 1] = bmpIMG->pal[i + 1] / 4;
		bmpIMG->pal[i + 0] = bmpIMG->pal[i + 0] / 4;
	}

	//load image buff
	F_fseek(FileStream, (LONG)buffOffset, 0);

	bmpIMG->buff = new BYTE[bmpIMG->size];

	int pitch = bmpIMG->width;
	if (pitch & 3)
		pitch = (pitch & 0xFFFFFFFC) + 4;
	int pitchDiff = pitch - bmpIMG->width;


	for (i = bmpIMG->size - bmpIMG->width; i > 0; i -= bmpIMG->width) {
		FReadCharArray(FileStream, (char*)&bmpIMG->buff[i], bmpIMG->width);
		for (int p = 0; p < pitchDiff; p++)
			F_fread8(FileStream, &byteVal);
	}

	F_fclose(FileStream);
	return true;
}


//________________________________
void fwrite8(FILE* file, BYTE val) {

	fputc(val, file);
}


//_________________________________
void fwrite16(FILE* file, WORD val) {

	fputc(val, file);
	fputc(val >> 8, file);
}


//__________________________________
void fwrite32(FILE* file, DWORD val) {

	fputc(val, file);
	fputc(val >> 8, file);
	fputc(val >> 16, file);
	fputc(val >> 24, file);
}


//_________________________________________________________________
LONG SaveBMP8(DWORD width, DWORD height, BYTE* buff, BYTE* palette) {

	FILE* FileStream = nullptr;
	char scrPath[256];
	sprintf(scrPath, "scr%.5d.bmp\0", 0);
	LONG scrNum = 0;
	while (FileStream = fopen(scrPath, "rb")) {
		fclose(FileStream);
		scrNum++;
		sprintf(scrPath, "scr%.5d.bmp\0", scrNum);
	}

	FileStream = fopen(scrPath, "wb");
	if (FileStream == nullptr)return -1;

	//read bmp header
	fwrite16(FileStream, 0x4D42);

	DWORD dibHeaderSize = 54;
	DWORD dibPaletteSize = 1024;

	DWORD pitch = width;
	if (pitch & 3)
		pitch = (pitch & 0xFFFFFFFC) + 4;
	DWORD pitchDiff = pitch - width;
	DWORD dibBuffSize = pitch * height;

	fwrite32(FileStream, dibHeaderSize + dibPaletteSize + dibBuffSize);//size of BMP file in bytes (unreliable)

	fwrite16(FileStream, 0x0000);//reserved, must be zero
	fwrite16(FileStream, 0x0000);//reserved, must be zero

	fwrite32(FileStream, dibHeaderSize + dibPaletteSize);//data offset
	fwrite32(FileStream, 40);//headersize
	fwrite32(FileStream, width);//image width in pixels
	fwrite32(FileStream, height);//image height in pixels
	fwrite16(FileStream, 1);//number of planes in the image, must be 1
	fwrite16(FileStream, 8);//number of bits per pixel (1, 4, 8, or 24)
	fwrite32(FileStream, 0);//compression type (0=none, 1=RLE-8, 2=RLE-4)
	fwrite32(FileStream, dibBuffSize);//size of image data in bytes (including padding)
	fwrite32(FileStream, 0);//horizontal resolution in pixels per meter (unreliable)
	fwrite32(FileStream, 0);//vertical resolution in pixels per meter (unreliable)
	fwrite32(FileStream, 256);//number of colors in image, or zero
	fwrite32(FileStream, 0);//number of important colors, or zero

	//save palette
	for (DWORD i = 0; i < 768; i += 3) {
		fwrite8(FileStream, palette[i + 2] << 2);
		fwrite8(FileStream, palette[i + 1] << 2);
		fwrite8(FileStream, palette[i + 0] << 2);
		fwrite8(FileStream, 0);
	}

	//save image buff
	DWORD buffSize = width * height;
	DWORD buffOff = buffSize - width;
	for (DWORD yPos = 0; yPos < height; yPos++) {
		for (DWORD xPos = 0; xPos < width; xPos++)
			fwrite8(FileStream, buff[buffOff + xPos]);
		for (DWORD p = 0; p < pitchDiff; p++)
			fwrite8(FileStream, 0);
		buffOff -= width;
	}

	fclose(FileStream);
	return 0;
}


//____________________________________________________________________________________________
LONG SaveBMP24(LONG srcWidth, LONG srcHeight, LONG srcPixelSize, LONG srcPitch, BYTE* srcBuff) {

	if (srcPixelSize < 3)
		return -1;

	FILE* FileStream = nullptr;
	char scrPath[256];
	sprintf(scrPath, "scr%.5d.bmp\0", 0);
	LONG scrNum = 0;
	while (FileStream = fopen(scrPath, "rb")) {
		fclose(FileStream);
		scrNum++;
		sprintf(scrPath, "scr%.5d.bmp\0", scrNum);
	}

	FileStream = fopen(scrPath, "wb");
	if (FileStream == nullptr)return -1;

	//read bmp header
	fwrite16(FileStream, 0x4D42);

	DWORD dibHeaderSize = 54;
	DWORD dibPaletteSize = 0;

	LONG widthBytes = srcWidth * 3;
	LONG dstPitch = widthBytes;
	if (dstPitch & 3)
		dstPitch = (dstPitch & 0xFFFFFFFC) + 4;
	LONG pitchDiff = dstPitch - widthBytes;
	LONG dibBuffSize = dstPitch * srcHeight;

	fwrite32(FileStream, dibHeaderSize + dibPaletteSize + dibBuffSize);//size of BMP file in bytes (unreliable)

	fwrite16(FileStream, 0x0000);//reserved, must be zero
	fwrite16(FileStream, 0x0000);//reserved, must be zero

	fwrite32(FileStream, dibHeaderSize + dibPaletteSize);//data offset
	fwrite32(FileStream, 40);//headersize
	fwrite32(FileStream, srcWidth);//image width in pixels
	fwrite32(FileStream, srcHeight);//image height in pixels
	fwrite16(FileStream, 1);//number of planes in the image, must be 1
	fwrite16(FileStream, 24);//number of bits per pixel (1, 4, 8, or 24)
	fwrite32(FileStream, 0);//compression type (0=none, 1=RLE-8, 2=RLE-4)
	fwrite32(FileStream, dibBuffSize);//size of image data in bytes (including padding)
	fwrite32(FileStream, 0);//horizontal resolution in pixels per meter (unreliable)
	fwrite32(FileStream, 0);//vertical resolution in pixels per meter (unreliable)
	fwrite32(FileStream, 0);//number of colors in image, or zero
	fwrite32(FileStream, 0);//number of important colors, or zero

	//save image buff
	LONG buffSize = srcPitch * srcHeight;
	LONG buffOff = buffSize - srcPitch;
	for (LONG yPos = 0; yPos < srcHeight; yPos++) {

		for (LONG xPos = 0; xPos < srcWidth * srcPixelSize; xPos += srcPixelSize) {
			for (DWORD cVal = 0; cVal < 3; cVal++)
				fwrite8(FileStream, srcBuff[buffOff + xPos + cVal]);
		}
		for (LONG p = 0; p < pitchDiff; p++)
			fwrite8(FileStream, 0);
		buffOff -= srcPitch;
	}

	fclose(FileStream);
	return 0;
}


//__________________________________________
void __declspec(naked)save_screen_shot(void) {

	__asm {
		push esi
		push edi
		push ebp

		push ecx
		push ebx
		push edx
		push eax
		call SaveBMP8
		add esp, 0x10

		pop ebp
		pop edi
		pop esi
		ret
	}
}


//________________
void SplashPanel() {

	char splashPath1[128], splashPath2[128];

	int splashNum = 0;
	FReadCfgInt(pCFG_FILE_PTR, "system", "splash", &splashNum);
	char* language = nullptr;
	if (FReadCfgString(pCFG_FILE_PTR, "system", "language", &language)) {
		int l = 0;
		while (language[l] != '\0') {
			language[l] = tolower(language[l]);
			l++;
		}
	}

	if (strncmp("english", language, 7) && strncmp("cht", language, 3))
		sprintf(splashPath1, "art\\%s\\splash\\\0", language);
	else sprintf(splashPath1, "art\\splash\\\0");

	IMAGE8* splashIMG = new IMAGE8;
	int splashMax = 20;
	for (int i = 0; i <= splashMax; i++) {
		sprintf(splashPath2, "%ssplash%d.bmp\0", splashPath1, splashNum);
		if (!LoadBMP(splashIMG, splashPath2)) {
			sprintf(splashPath2, "%ssplash%d.rix\0", splashPath1, splashNum);
			if (!LoadRIX(splashIMG, splashPath2)) {
				splashNum++;
				if (splashNum > splashMax)
					splashNum = 0;
			}
			else i = splashMax + 1;
		}
		else i = splashMax + 1;
	}

	FSetPalette(pBLACK_PAL);

	int colour = FindDarkPalRef(splashIMG->pal);

	splashWinRef = Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, 0xC);
	BYTE* wBuff = GetWinBuff(splashWinRef);

	if (splashIMG->buff && wBuff) {
		Fix8BitColours(splashIMG->buff, splashIMG->width, splashIMG->height, splashIMG->width, colour);
		if (SPLASH_SCRN_SIZE == 2)
			MemBlt8Stretch(splashIMG->buff, splashIMG->width, splashIMG->height, splashIMG->width, wBuff, SCR_WIDTH, SCR_HEIGHT, false, false);
		else if (SPLASH_SCRN_SIZE == 1 || splashIMG->width > (UINT)SCR_WIDTH || splashIMG->height > (UINT)SCR_HEIGHT)
			MemBlt8Stretch(splashIMG->buff, splashIMG->width, splashIMG->height, splashIMG->width, wBuff, SCR_WIDTH, SCR_HEIGHT, true, true);
		else {
			wBuff += ((SCR_WIDTH >> 1) - (splashIMG->width >> 1)) + (((SCR_HEIGHT >> 1) - (splashIMG->height >> 1)) * SCR_WIDTH);
			MemBlt8(splashIMG->buff, splashIMG->width, splashIMG->height, splashIMG->width, wBuff, SCR_WIDTH);
		}
	}
	ShowWin(splashWinRef);

	FFadeToPalette(splashIMG->pal);

	DWORD oldTick = GetTickCount();
	DWORD newTick = oldTick;
	while (oldTick + SPLASH_DELAY > newTick) {
		newTick = GetTickCount();
		if (newTick < oldTick)
			oldTick = newTick;
	}

	delete splashIMG;

	FWriteCfgInt(pCFG_FILE_PTR, "system", "splash", splashNum + 1);
}


//______________________________________
void __declspec(naked)splash_panel(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		call SplashPanel

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//________________________________
void SplashPanelDestroy(BYTE* pal) {

	FFadeToPalette(pal);
	if (splashWinRef != -1) {
		DestroyWin(splashWinRef);
		splashWinRef = -1;
	}
}


//______________________________________________
void __declspec(naked)splash_panel_destroy(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		push eax
		call SplashPanelDestroy
		add esp, 0x4

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//___________________________
void SplashScrnFixes_CH(void) {

	MemWrite8(0x443BF4, 0x53, 0xE9);
	FuncWrite32(0x443BF5, 0x57565251, (DWORD)&splash_panel);


	//movie fade in
	FuncWrite32(0x44DEBB, 0x0449C5, (DWORD)&splash_panel_destroy);

	//main-menu fade in
	FuncWrite32(0x480EAC, 0x0119D4, (DWORD)&splash_panel_destroy);

	MemWrite8(0x4C9A86, 0x56, 0xE9);
	FuncWrite32(0x4C9A87, 0xE5895557, (DWORD)&save_screen_shot);
}


//______________________________
void SplashScrnFixes_MULTI(void) {

	MemWrite8(0x444384, 0x53, 0xE9);
	FuncWrite32(0x444385, 0x57565251, (DWORD)&splash_panel);

	//movie fade in
	FuncReplace32(0x44E7C3, 0x04530D, (DWORD)&splash_panel_destroy);

	//main-menu fade in
	FuncReplace32(0x481A7C, 0x012054, (DWORD)&splash_panel_destroy);

	MemWrite8(0x4C9048, 0x56, 0xE9);
	FuncWrite32(0x4C9049, 0xEC835557, (DWORD)&save_screen_shot);
}


//________________________________
void SplashScrnFixes(DWORD region) {

	SPLASH_SCRN_SIZE = ConfigReadInt("STATIC_SCREENS", "SPLASH_SCRN_SIZE", 0);
	SPLASH_DELAY = ConfigReadInt("OTHER_SETTINGS", "SPLASH_SCRN_TIME", 0) * 1000;
	if (SPLASH_DELAY > 20000)
		SPLASH_DELAY = 20000;

	if (region == 4)
		SplashScrnFixes_CH();
	else
		SplashScrnFixes_MULTI();
}
