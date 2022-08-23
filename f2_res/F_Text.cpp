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

#include "F_Text.h"
#include "F_Windows.h"
#include "memwrite.h"
#include "GVX_multibyteText.h"

int (*pStr_FindLastWord)(const char* msg, int lineWidth);
int (*pStr_FindLastWord_NL)(const char* msg, int lineWidth);

void* SET_FONT = nullptr;
DWORD* GET_FONT = nullptr;
void* PRINT_TEXT = nullptr;
void* GET_TEXT_HEIGHT = nullptr;
void* GET_TEXT_WIDTH = nullptr;
void* GET_CHAR_WIDTH = nullptr;
void* GET_MAX_TEXT_WIDTH = nullptr;
void* GET_CHAR_GAP_WIDTH = nullptr;
void* GET_TEXT_SIZE = nullptr;
void* GET_MAX_CHAR_WIDTH = nullptr;


//__________________________________________________
int Str_FindLastWord(const char* msg, int lineWidth) {

	int msgWidth = 0;
	int count = 0;//char count
	int is_not_spc = 0;//num chars since last space
	char* msgP = (char*)msg;
	int gapWidth = GetCharGapWidth();

	while (*msgP != '\0') {
		if (*msgP == ' ')
			is_not_spc = 0;
		else
			is_not_spc++;

		msgWidth += GetCharWidth(*msgP);
		msgWidth += gapWidth;

		if (msgWidth > lineWidth) {//if message longer than width of line, return number of chars for that line.
			if (is_not_spc <= 1)//subtract any singlebyte char/word from the end of the line to the last space or multibyte char.
				return count;//if space was the last char on line return count
			else if (is_not_spc - 1 == count)
				return count;//else if no space on line return count
			else
				return count - (is_not_spc - 1);//else return count subtracted to the last space detected
		}
		count++;
		msgP++;
	}
	return count;
}


//_______________________________________________________
int Str_FindLastWordMulti(const char* msg, int lineWidth) {

	int count = 0;
	char* msgP = (char*)msg;
	int gapWidth = GetCharGapWidth();
	int msgWidth = 0;
	int is_single = 0;
	while (*msgP != '\0') {
		if (*msgP == ' ' || (unsigned char)*msgP > 160)/// || *msgP=='\n')///NEW!!! check for new line as white space.
			is_single = 0;//reset is_single count if space or multibyte char found.
		else
			is_single++;//count single byte chars since last space or multibyte char.

		msgWidth += GetCharWidth(*msgP);
		msgWidth += gapWidth;

		///if(*msgP=='\n')///NEW!!! end line if new line found. Edit don't bother doesn't work for msg's
		///   lineWidth=msgWidth;

		if (msgWidth > lineWidth) {//if message longer than width of line, return number of chars for that line.
			if (is_single <= 1)//subtract any singlebyte char/word from the end of the line to the last space or multibyte char.
				return count;
			else {
				if (count - (is_single - 1) == 0)///NEW!!! return the line count if there were no spaces.
					return count;
				else
					return count - (is_single - 1);
			}
		}

		if ((unsigned char)*msgP > 160)//if multibyte char, exclude next byte from count.
			count += 2;
		else
			count++;
		msgP = (char*)msg + count;//shift message offset to next char.
	}

	return count;
}


//_____________________________________________________
int Str_FindLastWord_NL(const char* msg, int lineWidth) {

	int msgWidth = 0;
	int count = 0;
	int is_not_spc = 0;
	char* msgP = (char*)msg;
	int gapWidth = GetCharGapWidth();

	while (*msgP != '\0') {
		if (*msgP == ' ' || *msgP == '\n')///NEW!!! check for new line as white space.)
			is_not_spc = 0;
		else
			is_not_spc++;

		msgWidth += GetCharWidth(*msgP);
		msgWidth += gapWidth;

		if (*msgP == '\n')///NEW!!! end line if new line found. Edit don't bother doesn't work for msg's
			lineWidth = msgWidth;

		if (msgWidth > lineWidth) {//if message longer than width of line, return number of chars for that line.
			if (is_not_spc <= 1)//subtract any singlebyte char/word from the end of the line to the last space or multibyte char.
				return count;
			else
				return count - (is_not_spc - 1);
		}
		count++;
		msgP++;
	}
	return count;
}


//__________________________________________________________
int Str_FindLastWordMulti_NL(const char* msg, int lineWidth) {
	int count = 0;
	char* msgP = (char*)msg;
	int gapWidth = GetCharGapWidth();
	int msgWidth = 0;
	int is_single = 0;
	while (*msgP != '\0') {
		if (*msgP == ' ' || (unsigned char)*msgP > 160 || *msgP == '\n')///NEW!!! check for new line as white space.
			is_single = 0;//reset is_single count if space or multibyte char found.
		else
			is_single++;//count single byte chars since last space or multibyte char.

		msgWidth += GetCharWidth(*msgP);
		msgWidth += gapWidth;

		if (*msgP == '\n')///NEW!!! end line if new line found. Edit don't bother doesn't work for msg's
			lineWidth = msgWidth;


		if (msgWidth > lineWidth) {//if message longer than width of line, return number of chars for that line.
			if (is_single <= 1)//subtract any singlebyte char/word from the end of the line to the last space or multibyte char.
				return count;
			else {
				if (count - (is_single - 1) == 0)///NEW!!! return the line count if there were no spaces.
					return count;
				else
					return count - (is_single - 1);
			}
		}

		if ((unsigned char)*msgP > 160)//if multibyte char, exclude next byte from count.
			count += 2;
		else
			count++;
		msgP = (char*)msg + count;//shift message offset to next char.
	}

	return count;
}


//___________________
void SetFont(int ref) {

	__asm {
		mov eax, ref
		call SET_FONT
	}
}


//_______________
int GetFont(void) {

	return *GET_FONT;
}


//______________________________________________________________________________________________
void PrintText(BYTE* toBuff, const char* txtBuff, DWORD txtWidth, DWORD toWidth, BYTE palColour) {

	__asm {
		push esi
		xor eax, eax
		MOV AL, palColour
		push eax
		mov ecx, toWidth
		mov ebx, txtWidth
		mov edx, txtBuff
		mov eax, toBuff
		mov esi, PRINT_TEXT
		call dword ptr ds : [esi]
		pop esi
	}
}


//_________________________________________________________________________________________________________________
void PrintText2(BYTE* toBuff, const char* txtBuff, DWORD txtWidth, int toX, int toY, DWORD toWidth, BYTE palColour) {

	int pos = toY * toWidth + toX;
	if (pos > 0)
		toBuff += pos;
	PrintText(toBuff, txtBuff, txtWidth, toWidth, palColour);
}


//___________________________________________________________________________________________________
void PrintText2Win(LONG winRef, const char* txtBuff, LONG txtWidth, int toX, int toY, BYTE palColour) {

	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;

	if (toX < 0)
		toX = 0;
	else if (toX > win->width)
		toX = win->width - 1;

	if (toX + txtWidth > win->width)
		txtWidth = win->width - toX;

	int txtHeight = GetTextHeight();

	if (toY + txtHeight > win->height)
		toY = win->height - txtHeight;
	if (toY < 0)
		toY = 0;

	int pos = toY * win->width + toX;

	PrintText(win->buff + pos, txtBuff, txtWidth, win->width, palColour);
}


//___________________
DWORD GetTextHeight() {

	DWORD TxtHeight = 0;
	__asm {
		mov eax, GET_TEXT_HEIGHT
		call dword ptr ds : [eax]//get text height
		mov TxtHeight, eax
	}
	return TxtHeight;
}


//_____________________________________
DWORD GetTextWidth(const char* TextMsg) {
	DWORD TxtWidth = 0;
	__asm {
		//      cmp TextMsg, 0
		//    je exitFunc
		mov eax, TextMsg
		push esi
		mov esi, GET_TEXT_WIDTH
		call dword ptr ds : [esi]//get text width
		pop esi
		mov TxtWidth, eax
	}
	return TxtWidth;
}


//______________________________
DWORD GetCharWidth(char CharVal) {

	DWORD charWidth = 0;
	__asm {
		mov al, CharVal
		push esi
		mov esi, GET_CHAR_WIDTH
		call dword ptr ds : [esi]
		pop esi
		mov charWidth, eax
	}
	return charWidth;
}


//________________________________________
DWORD GetMaxTextWidth(const char* TextMsg) {

	DWORD msgWidth = 0;
	__asm {
		mov eax, TextMsg
		push esi
		mov esi, GET_MAX_TEXT_WIDTH
		call dword ptr ds : [esi]
		pop esi
		mov msgWidth, eax
	}
	return msgWidth;
}


//_____________________
DWORD GetCharGapWidth() {

	DWORD gapWidth = 0;
	__asm {
		mov eax, GET_CHAR_GAP_WIDTH
		call dword ptr ds : [eax]
		mov gapWidth, eax
	}
	return gapWidth;
}


//_______________________________________
DWORD GetTextBoxSize(const char* TextMsg) {

	DWORD msgSize = 0;
	__asm {
		mov eax, TextMsg
		push esi
		mov esi, GET_TEXT_SIZE
		call dword ptr ds : [esi]
		pop esi
		mov msgSize, eax
	}
	return msgSize;
}


//_____________________
DWORD GetMaxCharWidth() {

	DWORD charWidth = 0;
	__asm {
		mov eax, GET_MAX_CHAR_WIDTH
		call dword ptr ds : [eax]
		mov charWidth, eax
	}
	return charWidth;
}


//___________________________
void FTextSetup(DWORD region) {

	if (region == 4) {
		pStr_FindLastWord = &Str_FindLastWordMulti;
		pStr_FindLastWord_NL = &Str_FindLastWordMulti_NL;


		SET_FONT = (void*)0x4DB431;
		GET_FONT = (DWORD*)0x52E18C;
		PRINT_TEXT = (void*)0x52E194;
		GET_TEXT_HEIGHT = (void*)0x52E198;
		GET_TEXT_WIDTH = (void*)0x52E19C;
		GET_CHAR_WIDTH = (void*)0x52E1A0;
		GET_MAX_TEXT_WIDTH = (void*)0x52E1A4;
		GET_CHAR_GAP_WIDTH = (void*)0x52E1A8;
		GET_TEXT_SIZE = (void*)0x52E1AC;
		GET_MAX_CHAR_WIDTH = (void*)0x52E1B0;
	}
	else {
		if (pGvx) {
			pStr_FindLastWord = &Str_FindLastWordMulti;
			pStr_FindLastWord_NL = &Str_FindLastWordMulti_NL;
		}
		else {
			pStr_FindLastWord = &Str_FindLastWord;
			pStr_FindLastWord_NL = &Str_FindLastWord_NL;
		}

		SET_FONT = (void*)FixAddress(0x4D58DC);
		GET_FONT = (DWORD*)FixAddress(0x51E3B0);
		PRINT_TEXT = (void*)FixAddress(0x51E3B8);
		GET_TEXT_HEIGHT = (void*)FixAddress(0x51E3BC);
		GET_TEXT_WIDTH = (void*)FixAddress(0x51E3C0);
		GET_CHAR_WIDTH = (void*)FixAddress(0x51E3C4);
		GET_MAX_TEXT_WIDTH = (void*)FixAddress(0x51E3C8);
		GET_CHAR_GAP_WIDTH = (void*)FixAddress(0x51E3CC);
		GET_TEXT_SIZE = (void*)FixAddress(0x51E3D0);
		GET_MAX_CHAR_WIDTH = (void*)FixAddress(0x51E3D4);
	}
}
