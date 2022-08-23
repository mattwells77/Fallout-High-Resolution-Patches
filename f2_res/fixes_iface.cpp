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
#include "memwrite.h"
#include "F_Art.h"
#include "F_Windows.h"
#include "F_Objects.h"
#include "F_Text.h"
#include "configTools.h"

#include "GVX_multibyteText.h"


UINT IFACE_BAR_MODE = 0;
UINT IFACE_BAR_SIDE_ART = 0;
UINT IFACE_BAR_SIDES_ORI = 0;

RECT* icombatRect = nullptr;
RECT* imonitorRect = nullptr;

RECT* iactionRect = nullptr;
RECT* iitemSlotRect = nullptr;

LONG wRef_ifaceLeft = -1, wRef_ifaceRight = -1;

LONG IFACE_BAR_WIDTH = 640;
LONG IFACE_BAR_GRAPHIC_WIDTH = 640;
BYTE** IMONITOR_COLOUR = nullptr;//0xD7;
LONG* IMONITOR_FONT = nullptr;//101;

int imonitor_numCharsWide = 256;
int imonitor_numLines = 100;

char imonitorTxtBuff[256 * 100];


int ALTERNATE_AMMO_METRE = 0;
BYTE AMMO_LIGHT = 0xC4;
BYTE AMMO_DARK = 0x4B;

int iface_region = 0;


LONG* imonitor_currentLine = nullptr;
LONG* imonitor_currentLine2 = nullptr;
LONG* imonitor_numVisibleLines = nullptr;
LONG* imonitor_time = nullptr;
LONG* timer1 = nullptr;
BYTE** lp_imonitorBgBuff = nullptr;
BYTE* imonitorBgBuff = nullptr;
DWORD* is_imonitor = nullptr;
BYTE* someFlag = nullptr;

LONG* imonitor_upButtRef = nullptr;
LONG* imonitor_downButtRef = nullptr;

int imonitorTxtHeight = 0;
int imonitorAreaX = 0;
int imonitorAreaY = 0;
int imonitorAreaWidth = 0;
int imonitorAreaHeight = 0;

LONG* buttRef_ifaceInv = nullptr;
LONG* buttRef_ifaceOptions = nullptr;
LONG* buttRef_ifaceSkill = nullptr;
LONG* buttRef_ifaceMap = nullptr;
LONG* buttRef_ifacePip = nullptr;
LONG* buttRef_ifaceChar = nullptr;
LONG* buttRef_ifaceAttack = nullptr;
LONG* buttRef_ifaceSwitchHand = nullptr;
LONG* buttRef_ifaceTurn = nullptr;
LONG* buttRef_ifaceCmbt = nullptr;


F_LONG_FUNC_VOID F_RedrawAmmoBar = nullptr;
void* F_DRAW_IFACE_HP_NUM = nullptr;
void* F_DRAW_IFACE_AC_NUM = nullptr;
void* F_DRAW_IFACE_AP_BAR = nullptr;
void* F_DRAW_AMMO_BAR = nullptr;

BYTE** winBuff_Iface = nullptr;


//______________________
void imonitorPrintText() {

	if (!*is_imonitor)
		return;
	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	if (!win)
		return;
	if (!win->buff)
		return;

	int oldFont = GetFont();
	SetFont(*IMONITOR_FONT);

	BYTE* winBuff = win->buff;

	winBuff += imonitorAreaY * win->width;
	winBuff += imonitorAreaX;
	int imonitorBgWidth = imonitorAreaWidth;
	MemBlt8(imonitorBgBuff, imonitorBgWidth, imonitorAreaHeight, imonitorBgWidth, winBuff, win->width);

	//console text colour 0xD7;
	//orange 0x92

	int xOffset = 0;
	char* txtBuff = 0;
	for (int i = 0; i < *imonitor_numVisibleLines; i++) {
		txtBuff = imonitorTxtBuff;
		txtBuff += ((*imonitor_currentLine2 + i + imonitor_numLines - *imonitor_numVisibleLines) % imonitor_numLines) * imonitor_numCharsWide;
		PrintText(winBuff + i * (imonitorTxtHeight * win->width) + xOffset, txtBuff, imonitorBgWidth, win->width, **IMONITOR_COLOUR);
		if (iface_region != 4)
			xOffset++;
	}

	DrawWindowArea(*pWinRef_Iface, imonitorRect);
	SetFont(oldFont);
}


//__________________________________________
void __declspec(naked) imonitor_print_text() {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		call imonitorPrintText
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
void imonitorInsertText(char* msg) {

	if (!*is_imonitor)
		return;

	char* txtBuff = imonitorTxtBuff;

	char lineStart_Char = '•';//0x95;
	int oldFont = GetFont();
	SetFont(*IMONITOR_FONT);

	int minTextWidth = GetCharWidth(lineStart_Char);//"ò\0");
	int msgWidth = 0;

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	if (!win)
		return;
	int lineWidth = imonitorAreaWidth;
	if (iface_region != 4)
		lineWidth -= *imonitor_numVisibleLines;
	lineWidth -= minTextWidth;

	if (!(*someFlag & 1)) {
		int ticks = 0;
		int time1 = *timer1;
		if (*imonitor_time < time1)
			ticks = time1 - *imonitor_time;
		else
			ticks = 0x7FFFFFFF;
		if (ticks >= 500) {
			PlayAcm("monitor");
			*imonitor_time = time1;
		}
	}

	int lineLength = 1;

	while (*msg != '\0') {
		txtBuff = imonitorTxtBuff + *imonitor_currentLine * imonitor_numCharsWide;//point txtBuff at next line
		if (lineStart_Char) {
			*txtBuff = lineStart_Char;
			txtBuff++;
		}
		else {
			if (iface_region == 4) {
				*txtBuff = ' ';
				txtBuff++;
			}
		}

		lineLength = pStr_FindLastWord(msg, lineWidth);

		if (*(msg + lineLength) == ' ')//add next char to end of line if equals space
			lineLength++;


		strncpy(txtBuff, msg, lineLength);
		txtBuff[lineLength] = '\0';

		if (lineLength == 0)
			*msg = '\0';
		else
			msg += lineLength;

		lineStart_Char = 0;
		*imonitor_currentLine = (*imonitor_currentLine + 1) % imonitor_numLines;
	}

	*imonitor_currentLine2 = *imonitor_currentLine;
	SetFont(oldFont);

	imonitorPrintText();
}


//___________________________________________
void __declspec(naked) imonitor_insert_text() {

	__asm {
		push eax
		call imonitorInsertText
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


//______________________________________________
void __declspec(naked) imonitor_insert_text_ch() {

	__asm {
		push ebp
		push eax
		call imonitorInsertText
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


//______________________
void imonitorCleatText() {
	if (!*is_imonitor)
		return;
	char* buff = imonitorTxtBuff;
	for (int i = 0; i < imonitor_numLines; i++) {
		*buff = '\0';
		buff += imonitor_numCharsWide;
	}
	*imonitor_currentLine = 0;
	*imonitor_currentLine2 = 0;
	imonitorPrintText();
}


//___________________________________________
void __declspec(naked) imonitor_clear_text() {

	__asm {
		push ebx
		push ecx
		push edx
		call imonitorCleatText
		xor eax, eax
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//___________________________________________
void __declspec(naked) imonitor_clear_text2() {

	__asm {
		call imonitorCleatText
		xor eax, eax
		add esp, 0x4
		pop ebp
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//_________________________
void iMonitorMouseDefault() {

	SetMouseImage(1);
}


//____________________
void iMonitorMouseUp() {

	SetMouseImage(2);
}


//______________________
void iMonitorMouseDown() {

	SetMouseImage(3);
}


//_____________________
void iMonitorScrollUp() {

	int newLineNum = (imonitor_numLines + *imonitor_currentLine2 - 1) % imonitor_numLines;
	if (newLineNum != *imonitor_currentLine)
		*imonitor_currentLine2 = newLineNum;
	imonitorPrintText();
}


//_______________________
void iMonitorScrollDown() {

	if (*imonitor_currentLine2 == *imonitor_currentLine)
		return;
	*imonitor_currentLine2 = (*imonitor_currentLine2 + 1) % imonitor_numLines;
	imonitorPrintText();
}


//________________________________________________________
bool CheckMouseInImonitorRect(int zDelta, bool scrollPage) {

	if (!*is_imonitor)
		return false;
	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	if (!win)
		return false;
	if (win->flags & F_WIN_HIDDEN)
		return false;

	RECT rcImon{
		imonitorRect->left + win->rect.left,
		imonitorRect->top + win->rect.top,
		imonitorRect->right + win->rect.left,
		imonitorRect->bottom + win->rect.top };

	int numLines = 1;
	if (scrollPage)
		numLines = *imonitor_numVisibleLines;

	if (IsMouseInRect(&rcImon)) {
		while (numLines > 0) {
			if (zDelta > 0)
				iMonitorScrollUp();
			else if (zDelta < 0)
				iMonitorScrollDown();
			numLines--;
		}
		return true;
	}
	return false;
}


//_______________________________
int iMonitorSetup(int isResizing) {

	int oldFont = GetFont();
	SetFont(*IMONITOR_FONT);
	imonitorTxtHeight = GetTextHeight();
	SetFont(oldFont);

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	if (!win)
		return -1;

	imonitorAreaX = 23;
	imonitorAreaWidth = 167 + (win->width - 640);
	int buttHeight;
	if (iface_region == 4) {//chi
		imonitorAreaY = 12;
		imonitorAreaHeight = 72;
		imonitorTxtHeight += 2;
		buttHeight = 36;
	}
	else if (pGvx) {
		imonitorAreaY = 24;
		imonitorAreaHeight = 60;
		imonitorTxtHeight += 2;
		buttHeight = 30;
	}
	else {
		imonitorAreaY = 24;
		imonitorAreaHeight = 60;
		buttHeight = 30;
	}

	*imonitor_numVisibleLines = imonitorAreaHeight / imonitorTxtHeight;
	*imonitor_currentLine = 0;
	*imonitor_currentLine2 = 0;

	imonitorBgBuff = FReallocateMemory(imonitorBgBuff, imonitorAreaWidth * imonitorAreaHeight);

	*lp_imonitorBgBuff = imonitorBgBuff;
	if (imonitorBgBuff == nullptr)
		return -1;

	BYTE* fromBuff = win->buff + imonitorAreaY * win->width + imonitorAreaX;
	MemBlt8(fromBuff, imonitorAreaWidth, imonitorAreaHeight, win->width, imonitorBgBuff, imonitorAreaWidth);

	if (!*is_imonitor) {
		*imonitor_upButtRef = CreateButton(*pWinRef_Iface, imonitorAreaX, imonitorAreaY, imonitorAreaWidth, buttHeight, -1, -1, -1, -1, 0, 0, 0, 0);
		SetButtonFunctions(*imonitor_upButtRef, &iMonitorMouseUp, &iMonitorMouseDefault, &iMonitorScrollUp, 0);

		*imonitor_downButtRef = CreateButton(*pWinRef_Iface, imonitorAreaX, imonitorAreaY + buttHeight, imonitorAreaWidth, buttHeight, -1, -1, -1, -1, 0, 0, 0, 0);
		SetButtonFunctions(*imonitor_downButtRef, &iMonitorMouseDown, &iMonitorMouseDefault, &iMonitorScrollDown, 0);
	}
	else {
		ButtonStruct* button = GetButton(*imonitor_upButtRef, nullptr);
		if (button) {
			button->rect.left = imonitorAreaX;
			button->rect.right = button->rect.left + imonitorAreaWidth;
			button->rect.top = imonitorAreaY;
			button->rect.bottom = button->rect.top + buttHeight;
		}
		button = GetButton(*imonitor_downButtRef, nullptr);
		if (button) {
			button->rect.left = imonitorAreaX;
			button->rect.right = button->rect.left + imonitorAreaWidth;
			button->rect.top = imonitorAreaY + buttHeight;
			button->rect.bottom = button->rect.top + buttHeight;
		}
	}
	imonitorCleatText();
	*is_imonitor = 1;
	imonitorPrintText();
	return 0;
}


//_____________________________________
void __declspec(naked) imonitor_setup() {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push ebp
		push 0
		call iMonitorSetup
		add esp, 0x4
		pop ebp
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//____________________________________________
BYTE* FixCounterPos(int* x, int y, BYTE* buff) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	if (win == nullptr)return buff;
	*x += (win->width - 640);
	buff += y * win->width;
	return buff;
}


//______________________________________
void __declspec(naked) fix_counter_pos() {

	__asm {
		push ebx
		push ecx
		push esi

		push edx
		push eax
		lea eax, [esp + 0x34]
		push eax
		call FixCounterPos
		add esp, 0xC
		mov edx, eax

		pop esi
		pop ecx
		pop ebx
		ret
	}
}


//_______________________________
void SetMonBgMem(DWORD sizeBytes) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	if (win == nullptr)return;

	int height;
	if (iface_region == 4)//chi
		height = 72;
	else
		height = 60;

	sizeBytes = (167 + (win->width - 640)) * 60;
	FAllocateMemory(sizeBytes);
}


//_____________________________________
void __declspec(naked) set_mon_bg_mem() {

	__asm {
		push eax
		call SetMonBgMem
		add esp, 0x4
		ret
	}
}


//___________________________________________________________________________________________________________________
void FixIfaceMonBGCopy(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	subWidth = 167 + (win->width - 640);
	toWidth = 167 + (win->width - 640);
	MemBlt8(fromBuff, subWidth, subHeight, fromWidth, toBuff, toWidth);
}


//____________________________________________________________
void SetAmmoBarColour(int barLevel, BYTE* lightC, BYTE* darkC) {

	switch (ALTERNATE_AMMO_METRE) {
	case 1:
		*lightC = AMMO_LIGHT, *darkC = AMMO_DARK;
		break;
	case 2:
	case 3:
		if (barLevel <= 14)
			*lightC = 0x88, * darkC = 0xB5; //red
		else if (barLevel <= 28)
			*lightC = 0x84, * darkC = 0x8C; //red brighter
		else if (barLevel <= 42)
			*lightC = 0x91, * darkC = 0x9A; //orange
		else if (barLevel <= 56)
			*lightC = 0x3A, * darkC = 0x42; //yellow
		else
			*lightC = 0xD7, * darkC = 0x4B; //green
		//*lightC = 0xC4, *darkC = 0x4B; //green
		break;

	default:
		*lightC = 0xC4, * darkC = 0x4B; //green
		break;

	}
}


//____________________________________________________
void DrawAlternateAmmoBar(int barX, int barFillHeight) {
	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	if (win == nullptr)return;

	//barX-=2;//211;//9;//
	//fix barX if iface longer
	barX += win->width - 640;

	int barY = 26;
	int barFillHeightMax = 70;

	int barWidth = 3;
	BYTE barColour;
	BYTE barColourDark;
	BYTE barEmptyColour = 0x0D;//0x0E
	BYTE barEmptyColourDark = 0x0F;//0x0E

	SetAmmoBarColour(barFillHeight, &barColour, &barColourDark);

	if (barFillHeight & 1)
		barFillHeight--;
	BYTE* buff = win->buff;
	buff += ((barY - 1) * win->width) + barX - 1;
	memset(buff, barEmptyColourDark, barWidth + 2);
	buff += win->width;

	while (barFillHeightMax > barFillHeight) {
		buff[0] = 0x0B;
		memset(buff + 1, barEmptyColour, barWidth);
		buff[barWidth + 1] = barEmptyColourDark;
		buff += win->width;
		buff[0] = 0x0B;
		memset(buff + 1, barEmptyColourDark, barWidth);
		buff[barWidth + 1] = barEmptyColourDark;
		buff += win->width;
		barFillHeightMax -= 2;
	}

	while (barFillHeight > 0) {
		if (ALTERNATE_AMMO_METRE == 3)
			SetAmmoBarColour(barFillHeight, &barColour, &barColourDark);
		buff[0] = 0x0B;
		memset(buff + 1, barColour, barWidth);
		buff[barWidth + 1] = barColourDark;
		buff += win->width;
		buff[0] = 0x0B;
		memset(buff + 1, barColourDark, barWidth);
		buff[barWidth + 1] = barColourDark;
		buff += win->width;
		barFillHeight -= 2;
	}
	memset(buff, 0x0A, barWidth + 2);

	RECT rect = { barX,26,barX + barWidth,96 };
	DrawWindowArea(*pWinRef_Iface, &rect);
}


//___________________________________________
void DrawAmmoBar(int barX, int barFillHeight) {
	if (ALTERNATE_AMMO_METRE) {
		DrawAlternateAmmoBar(barX, barFillHeight);
		return;
	}
	barX -= 4;//move bar position away from rivits

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	if (win == nullptr)return;

	//fix barX if iface longer
	barX += win->width - 640;

	int barY = 26;
	int barFillHeightMax = 70;

	if (barFillHeight & 1)
		barFillHeight--;
	BYTE* buff = win->buff;
	buff += (barY * win->width) + barX;

	while (barFillHeightMax > barFillHeight) {
		buff[0] = 0x0E;
		buff += win->width;
		barFillHeightMax--;
	}

	while (barFillHeight > 0) {
		buff[0] = 0xC4;
		buff += win->width;
		buff[0] = 0x0E;
		buff += win->width;
		barFillHeight -= 2;
	}

	RECT rect = { barX,26,barX + 1,96 };
	DrawWindowArea(*pWinRef_Iface, &rect);
}


//_______________________________________
void __declspec(naked) draw_ammo_bar() {

	__asm {
		push ebx
		push ecx
		push esi

		push edx
		push eax
		call DrawAmmoBar
		add esp, 0x8

		pop esi
		pop ecx
		pop ebx
		ret
	}

}


//____________________________________________________________________________________________________________________
void FixIfaceMainBGDraw(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	char ifaceName[256];

	if (iface_region == 4)
		sprintf(ifaceName, "HR_IFACE_CHI_%i.frm\0", win->width);
	else
		sprintf(ifaceName, "HR_IFACE_%i.frm\0", win->width);

	UNLSTDfrm* iface = LoadUnlistedFrm(ifaceName, ART_INTRFACE);
	if (iface && iface->frames[0].width == win->width) {
		if (iface->frames[0].height < subHeight)
			subHeight = iface->frames[0].height;
		MemBlt8(iface->frames[0].buff, iface->frames[0].width, subHeight, iface->frames[0].width, toBuff, win->width);
		delete iface;
	}
	else {
		MemBlt8(fromBuff, subWidth, subHeight, fromWidth, toBuff + (win->width - toWidth), win->width);

		if (win->width > 640) {
			MemBlt8(fromBuff, 32, subHeight, fromWidth, toBuff, win->width);
			int monWidth = 152 + win->width - toWidth;
			BYTE* monBuff = new BYTE[monWidth * subHeight];
			MemBlt8Stretch(fromBuff + 32, 152, subHeight, fromWidth, monBuff, monWidth, subHeight, false, false);
			MemBlt8(monBuff, monWidth, subHeight, monWidth, toBuff + 32, win->width);
			delete[] monBuff;
		}
	}
}


//____________________________________________________________________________________________________________________________
void FixIfaceActionPointsBGCopy(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	fromBuff -= (14 * 640 + 316);
	fromBuff += (14 * win->width + 316 + win->width - 640);
	MemBlt8(fromBuff, subWidth, subHeight, win->width, toBuff, toWidth);
}


//____________________________________________________________________________________________________________________________
void FixIfaceActionPointsBGDraw(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	toBuff -= (14 * 640 + 316);
	toBuff += (14 * win->width + 316 + win->width - 640);
	MemBlt8(fromBuff, subWidth, subHeight, fromWidth, toBuff, win->width);
}


//______________________________________________________________________________________________________________________
void FixIfaceCombatBGDraw(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	toBuff -= (38 * 640 + 580);
	toBuff += (38 * win->width + 580 + win->width - 640);
	MemBlt8(fromBuff, subWidth, subHeight, fromWidth, toBuff, win->width);
}
//_______________________________________________________________________________________________________________________
void FixIfaceCombatBGDraw2(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	toBuff -= (38 * 640 + 580);
	toBuff += (38 * win->width + 580 + win->width - 640);
	MemBltMasked8(fromBuff, subWidth, subHeight, fromWidth, toBuff, win->width);
}


//_____________________________________________________________________________________________________________________
void FixIfaceCounterDraw(BYTE* fromBuff, DWORD subWidth, DWORD subHeight, DWORD fromWidth, BYTE* toBuff, DWORD toWidth) {

	WinStruct* win = GetWinStruct(*pWinRef_Iface);
	MemBlt8(fromBuff, subWidth, subHeight, fromWidth, toBuff, win->width);
}


//_____________________________________________________________________________________________________________________________________________________________________________________________________
int _stdcall FixIfaceButton(int winRef, int x, int y, int width, int height, int keyHoverOn, int keyHoverOff, int keyPush, int keyLift, BYTE* upPicBuff, BYTE* downPicBuff, BYTE* unknown, DWORD flags) {

	WinStruct* win = GetWinStruct(winRef);
	x += win->width - 640;
	return CreateButton(winRef, x, y, width, height, keyHoverOn, keyHoverOff, keyPush, keyLift, upPicBuff, downPicBuff, unknown, flags);
}


//_______________________________________
void __declspec(naked) fix_iface_button() {

	__asm {
		push ebx
		push edx
		push eax
		mov ebx, dword ptr ds : [esp + 12]//copy ret address from the stack
		mov dword ptr ds : [esp + 12] , ecx//replace ret address with ecx(button width) on the stack
		call FixIfaceButton//set_button
		push ebx//re-insert ret address
		ret
	}
}


//________________________
void load_iface_frms(void) {

	char barArtName[32];
	sprintf(barArtName, "HR_IFACELFT%d.frm", IFACE_BAR_SIDE_ART);
	UNLSTDfrm* leftFrm = LoadUnlistedFrm(barArtName, ART_INTRFACE);
	if (leftFrm == nullptr)
		leftFrm = CreateUnlistedFrm(640, 480, 1, 1);

	sprintf(barArtName, "HR_IFACERHT%d.frm", IFACE_BAR_SIDE_ART);
	UNLSTDfrm* rightFrm = LoadUnlistedFrm(barArtName, ART_INTRFACE);
	if (rightFrm == nullptr)
		rightFrm = CreateUnlistedFrm(640, 480, 1, 1);

	RedrawWin(wRef_ifaceLeft);
	WinStruct* iLeftWin = GetWinStruct(wRef_ifaceLeft);
	if (!iLeftWin) return;
	RedrawWin(wRef_ifaceRight);
	WinStruct* iRightWin = GetWinStruct(wRef_ifaceRight);
	if (!iRightWin) return;

	int lWidth = leftFrm->frames[0].width;
	int lHeight = leftFrm->frames[0].height;
	int rWidth = rightFrm->frames[0].width;
	int rHeight = rightFrm->frames[0].height;

	int lSubWidth = iLeftWin->width;
	if (lSubWidth > lWidth)
		lSubWidth = lWidth;

	int rSubWidth = iRightWin->width;
	if (rSubWidth > rWidth)
		rSubWidth = rWidth;

	if (IFACE_BAR_SIDES_ORI == 1) {
		MemBlt8Stretch(leftFrm->frames[0].buff, lSubWidth, lHeight, lWidth, iLeftWin->buff, iLeftWin->width, iLeftWin->height, false, false);
		MemBlt8Stretch(rightFrm->frames[0].buff + (rWidth - rSubWidth), rSubWidth, rHeight, rWidth, iRightWin->buff, iRightWin->width, iRightWin->height, false, false);
	}
	else if (IFACE_BAR_SIDES_ORI == 0) {
		MemBlt8Stretch(rightFrm->frames[0].buff, rSubWidth, rHeight, rWidth, iRightWin->buff, iRightWin->width, iRightWin->height, false, false);
		MemBlt8Stretch(leftFrm->frames[0].buff + (lWidth - lSubWidth), lSubWidth, lHeight, lWidth, iLeftWin->buff, iLeftWin->width, iLeftWin->height, false, false);
	}

	delete leftFrm;
	leftFrm = nullptr;
	delete rightFrm;
	rightFrm = nullptr;
}


//____________________________________
void IfaceSidesReload(int ifaceArtNum) {

	IFACE_BAR_SIDE_ART = ifaceArtNum;
	if (IFACE_BAR_MODE == 0 && SCR_WIDTH > IFACE_BAR_WIDTH)
		load_iface_frms();
}


//_____________________
void HideIfaceWindows() {

	if (IFACE_BAR_MODE == 0) {
		HideWin(wRef_ifaceLeft);
		HideWin(wRef_ifaceRight);
	}

	HideWin(*pWinRef_Iface);
}


//_____________________
void ShowIfaceWindows() {

	if (IFACE_BAR_MODE == 0) {
		ShowWin(wRef_ifaceLeft);
		ShowWin(wRef_ifaceRight);
	}

	ShowWin(*pWinRef_Iface);
}


//_____________________________________________________
DWORD _stdcall IfaceSetup(DWORD colour, DWORD winFlags) {

	LONG winRef = -1;
	if (SCR_WIDTH < IFACE_BAR_WIDTH)
		IFACE_BAR_WIDTH = SCR_WIDTH;

	///winRef = Win_Create(SCR_WIDTH/2-IFACE_BAR_WIDTH/2, gameWin->rect.bottom, IFACE_BAR_WIDTH, 100, colour, winFlags);
	winRef = Win_Create(SCR_WIDTH / 2 - IFACE_BAR_WIDTH / 2, SCR_HEIGHT - 100, IFACE_BAR_WIDTH, 100, colour, winFlags);

	WinStruct* ifaceWin = GetWinStruct(winRef);

	if (IFACE_BAR_MODE == 0 && SCR_WIDTH > IFACE_BAR_WIDTH) {
		wRef_ifaceLeft = Win_Create(0, ifaceWin->rect.top, ifaceWin->rect.left, ifaceWin->height, colour, winFlags);
		wRef_ifaceRight = Win_Create(ifaceWin->rect.right + 1, ifaceWin->rect.top, SCR_WIDTH - (ifaceWin->rect.right + 1), ifaceWin->height, colour, winFlags);

		load_iface_frms();
	}

	icombatRect->left = 580 + ifaceWin->width - 640;
	icombatRect->right = 637 + ifaceWin->width - 640;

	imonitorRect->right = 189 + ifaceWin->width - 640;

	iactionRect->left = 316 + ifaceWin->width - 640;
	iactionRect->right = 406 + ifaceWin->width - 640;

	iitemSlotRect->left = 267 + ifaceWin->width - 640;
	iitemSlotRect->right = 455 + ifaceWin->width - 640;

	return winRef;
}


//________________________
void DestroyIfaceWindows() {

	if (IFACE_BAR_MODE == 0) {
		DestroyWin(wRef_ifaceLeft);
		wRef_ifaceLeft = -1;
		DestroyWin(wRef_ifaceRight);
		wRef_ifaceRight = -1;
	}
	DestroyWin(*pWinRef_Iface);
}


//___________________________________________
void __declspec(naked) h_iface_bar_kill(void) {

	__asm {
		call DestroyIfaceWindows
		mov ecx, -1
		ret
	}
}


//___________________________________________
void __declspec(naked) h_iface_bar_hide(void) {

	__asm {
		push ecx
		push edi
		call HideIfaceWindows
		pop edi
		pop ecx
		ret
	}
}


//___________________________________________
void __declspec(naked) h_iface_bar_show(void) {

	__asm {
		call ShowIfaceWindows
		xor ecx, ecx
		ret
	}
}


//______________________________________________________________________________________
int IfaceSubWin(LONG x, LONG y, DWORD width, DWORD height, DWORD colour, DWORD winFlags) {

	WinStruct* ifaceWin = GetWinStruct(*pWinRef_Iface);
	return Win_Create(x + ifaceWin->rect.left, ifaceWin->rect.bottom - (478 - y), width, height, colour, winFlags);
}


//__________________________________________
void __declspec(naked) h_iface_sub_win(void) {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]

		push ecx
		push ebx
		push edx
		push eax
		call IfaceSubWin
		add esp, 0x18
		ret 0x8
	}

}


//_________________________________________________________________________________________
LONG IfaceSkillWin(LONG x, LONG y, DWORD width, DWORD height, DWORD colour, DWORD winFlags) {

	WinStruct* ifaceWin = GetWinStruct(*pWinRef_Iface);
	return Win_Create(x + ifaceWin->rect.left + (ifaceWin->width - 640), ifaceWin->rect.bottom - (478 - y), width, height, colour, winFlags);
}


//____________________________________________
void __declspec(naked) h_iface_skill_win(void) {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]

		push ecx
		push ebx
		push edx
		push eax
		call IfaceSkillWin
		add esp, 0x18
		ret 0x8
	}

}


//___________________________________________
void __declspec(naked) h_iface_skill_kill(void) {

	__asm {
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push eax
		call DestroyWin
		add esp, 0x4
		mov eax, pWinRef_Skills
		mov dword ptr ds : [eax] , -1

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		ret
	}

}


//___________________________________
void F_DrawIfaceHPNumbers(DWORD flag) {

	__asm {
		mov eax, flag
		call F_DRAW_IFACE_HP_NUM
	}
}


//___________________________________
void F_DrawIfaceACNumbers(DWORD flag) {

	__asm {
		mov eax, flag
		call F_DRAW_IFACE_AC_NUM
	}
}


//_____________________________________
void F_DrawIfaceApBar(int actionPoints) {

	__asm {
		mov eax, actionPoints
		call F_DRAW_IFACE_AP_BAR
	}
}


//__________________
void F_DrawAmmoBar() {

	__asm {
		call F_DRAW_AMMO_BAR
	}
}


//________________
bool ReSizeIface() {

	if (*pWinRef_Iface == -1)
		return false;
	WinStruct* ifaceWin = GetWinStruct(*pWinRef_Iface);
	if (ifaceWin == nullptr)
		return false;

	IFACE_BAR_WIDTH = IFACE_BAR_GRAPHIC_WIDTH;
	if (IFACE_BAR_WIDTH > SCR_WIDTH)
		IFACE_BAR_WIDTH = SCR_WIDTH;

	ifaceWin->rect.left = SCR_WIDTH / 2 - IFACE_BAR_WIDTH / 2;
	ifaceWin->rect.top = SCR_HEIGHT - ifaceWin->height;//gameWin->rect.bottom;
	ifaceWin->rect.right = ifaceWin->rect.left + IFACE_BAR_WIDTH - 1;
	ifaceWin->rect.bottom = ifaceWin->rect.top + ifaceWin->height - 1;//ifaceWin->rect.top+ifaceWin->height-1;

	WinStruct* notifyWin = GetWinStruct(*pWinRef_NotifyBar);
	if (notifyWin != nullptr) {
		notifyWin->rect.left = ifaceWin->rect.left;
		notifyWin->rect.top = ifaceWin->rect.top - notifyWin->height;
		notifyWin->rect.right = notifyWin->rect.left + notifyWin->width - 1;
		notifyWin->rect.bottom = notifyWin->rect.top + notifyWin->height - 1;
	}
	WinStruct* skillsWin = GetWinStruct(*pWinRef_Skills);
	if (skillsWin != nullptr) {
		skillsWin->rect.left = ifaceWin->rect.right - skillsWin->width - 3;
		skillsWin->rect.top = ifaceWin->rect.top - skillsWin->height - 6;
		skillsWin->rect.right = skillsWin->rect.left + skillsWin->width - 1;
		skillsWin->rect.bottom = skillsWin->rect.top + skillsWin->height - 1;
	}

	if (ifaceWin->width != (int)IFACE_BAR_WIDTH) {

		ifaceWin->width = IFACE_BAR_WIDTH;


		ifaceWin->buff = FReallocateMemory(ifaceWin->buff, ifaceWin->width * ifaceWin->height);
		*winBuff_Iface = ifaceWin->buff;

		memset(ifaceWin->buff, ifaceWin->colour, ifaceWin->width * ifaceWin->height);
		UNLSTDfrm* iface = LoadUnlistedFrm("IFACE.frm", ART_INTRFACE);
		int subWidth = iface->frames[0].width;
		int subHeight = ifaceWin->height;
		if (iface->frames[0].height < subHeight)
			subHeight = iface->frames[0].height;
		FixIfaceMainBGDraw(iface->frames[0].buff, subWidth, subHeight, subWidth, ifaceWin->buff, subWidth);
		delete iface;

		icombatRect->left = 580 + ifaceWin->width - 640;
		icombatRect->right = 637 + ifaceWin->width - 640;

		imonitorRect->right = 189 + ifaceWin->width - 640;

		iactionRect->left = 316 + ifaceWin->width - 640;
		iactionRect->right = 406 + ifaceWin->width - 640;

		iitemSlotRect->left = 267 + ifaceWin->width - 640;
		iitemSlotRect->right = 455 + ifaceWin->width - 640;


		ButtonStruct* button = nullptr;
		if (*buttRef_ifaceInv != -1) {
			button = GetButton(*buttRef_ifaceInv, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 211);
				button->rect.right = button->rect.left + 32 - 1;
			}
		}
		if (*buttRef_ifaceOptions != -1) {
			button = GetButton(*buttRef_ifaceOptions, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 210);
				button->rect.right = button->rect.left + 34 - 1;
			}
		}
		if (*buttRef_ifaceSkill != -1) {
			button = GetButton(*buttRef_ifaceSkill, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 523);
				button->rect.right = button->rect.left + 22 - 1;
			}
		}
		if (*buttRef_ifaceMap != -1) {
			button = GetButton(*buttRef_ifaceMap, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 526);
				button->rect.right = button->rect.left + 41 - 1;
			}
		}
		if (*buttRef_ifacePip != -1) {
			button = GetButton(*buttRef_ifacePip, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 526);
				button->rect.right = button->rect.left + 41 - 1;
			}
		}
		if (*buttRef_ifaceChar != -1) {
			button = GetButton(*buttRef_ifaceChar, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 526);
				button->rect.right = button->rect.left + 41 - 1;
			}
		}
		if (*buttRef_ifaceAttack != -1) {
			button = GetButton(*buttRef_ifaceAttack, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 267);
				button->rect.right = button->rect.left + 188 - 1;
			}
		}
		if (*buttRef_ifaceSwitchHand != -1) {
			button = GetButton(*buttRef_ifaceSwitchHand, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 218);
				button->rect.right = button->rect.left + 22 - 1;
			}
		}
		if (*buttRef_ifaceTurn != -1) {
			button = GetButton(*buttRef_ifaceTurn, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 590);
				button->rect.right = button->rect.left + 38 - 1;
			}
		}
		if (*buttRef_ifaceCmbt != -1) {
			button = GetButton(*buttRef_ifaceCmbt, nullptr);
			if (button) {
				button->rect.left = ifaceWin->width - (640 - 590);
				button->rect.right = button->rect.left + 38 - 1;
			}
		}

		iMonitorSetup(true);

		if (!(ifaceWin->flags & F_WIN_HIDDEN)) {
			F_RedrawAmmoBar();
			F_DrawIfaceHPNumbers(0);
			F_DrawIfaceACNumbers(0);
		}

		if (*buttRef_ifaceCmbt != -1) {
			UNLSTDfrm* frm = LoadUnlistedFrm("endanim.frm", ART_INTRFACE);
			int width = frm->frames[frm->numFrames - 1].width;
			int height = frm->frames[frm->numFrames - 1].height;
			BYTE* toBuff = ifaceWin->buff + ifaceWin->width * 39 - 60;
			MemBlt8(frm->frames[frm->numFrames - 1].buff, width, height, width, toBuff, ifaceWin->width);
			delete frm;
			frm = LoadUnlistedFrm("endltgrn.frm", ART_INTRFACE);
			width = frm->frames[0].width;
			height = frm->frames[0].height;
			MemBltMasked8(frm->frames[0].buff, width, height, width, toBuff, ifaceWin->width);
			delete frm;

			if (*lpObj_PC == *lpObj_ActiveCritter) {
				OBJStruct* pObj_PC = *lpObj_PC;
				F_DrawIfaceApBar(pObj_PC->pud.critter.combat_data.currentAP);
			}
		}
	}

	if (IFACE_BAR_MODE == 0 && SCR_WIDTH > IFACE_BAR_WIDTH) {
		if (wRef_ifaceLeft == -1 || wRef_ifaceRight == -1) {
			if (wRef_ifaceLeft == -1)
				wRef_ifaceLeft = Win_Create(0, ifaceWin->rect.top, ifaceWin->rect.left, ifaceWin->height, ifaceWin->colour, ifaceWin->flags);
			if (wRef_ifaceRight == -1)
				wRef_ifaceRight = Win_Create(ifaceWin->rect.right + 1, ifaceWin->rect.top, SCR_WIDTH - (ifaceWin->rect.right + 1), ifaceWin->height, ifaceWin->colour, ifaceWin->flags);
		}
		else {
			WinStruct* win = GetWinStruct(wRef_ifaceLeft);
			if (win) {
				win->rect.left = 0;
				win->rect.right = ifaceWin->rect.left - 1;
				win->rect.top = ifaceWin->rect.top;
				win->rect.bottom = ifaceWin->rect.bottom;
				win->width = ifaceWin->rect.left;
				win->buff = FReallocateMemory(win->buff, win->width * win->height);
			}
			win = GetWinStruct(wRef_ifaceRight);
			if (win) {
				win->rect.left = ifaceWin->rect.right + 1;
				win->rect.right = SCR_WIDTH - 1;
				win->rect.top = ifaceWin->rect.top;
				win->rect.bottom = ifaceWin->rect.bottom;
				win->width = SCR_WIDTH - (ifaceWin->rect.right + 1);
				win->buff = FReallocateMemory(win->buff, win->width * win->height);
			}
		}
		load_iface_frms();
	}
	else if (wRef_ifaceLeft != -1 || wRef_ifaceRight != -1) {
		if (wRef_ifaceLeft != -1) {
			DestroyWin(wRef_ifaceLeft);
			wRef_ifaceLeft = -1;
		}
		if (wRef_ifaceRight != -1) {
			DestroyWin(wRef_ifaceRight);
			wRef_ifaceRight = -1;
		}
	}
	return true;
}



//______________________
void IfaceFixes_CH(void) {

	buttRef_ifaceInv = (LONG*)0x528D08;
	buttRef_ifaceOptions = (LONG*)0x528D14;
	buttRef_ifaceSkill = (LONG*)0x528D20;
	buttRef_ifaceMap = (LONG*)0x528D30;
	buttRef_ifacePip = (LONG*)0x528D40;
	buttRef_ifaceChar = (LONG*)0x528D4C;
	buttRef_ifaceAttack = (LONG*)0x528D58;
	buttRef_ifaceSwitchHand = (LONG*)0x528D7C;
	buttRef_ifaceTurn = (LONG*)0x528DA0;
	buttRef_ifaceCmbt = (LONG*)0x528DAC;

	//NOTIFY-BAR SETUP
	FuncWrite32(0x460D00, 0x07B3B9, (DWORD)&h_iface_sub_win);

	//SKILLBAR WINDOW SETUP
	FuncWrite32(0x4AAF90, 0x031129, (DWORD)&h_iface_skill_win);

	FuncWrite32(0x4AB21F, 0x03119D, (DWORD)&h_iface_skill_kill);

	//IFACE SETUP
	FuncWrite32(0x45CFBD, 0x07F0FC, (DWORD)&IfaceSetup);

	FuncWrite32(0x45DF47, 0x07E475, (DWORD)&h_iface_bar_kill);

	FuncWrite32(0x45DAF6, 0x07F604, (DWORD)&h_iface_bar_hide);

	FuncWrite32(0x45DFFC, 0x07F0FE, (DWORD)&h_iface_bar_hide);

	FuncWrite32(0x45E0FE, 0x07EFFC, (DWORD)&h_iface_bar_hide);

	FuncWrite32(0x45E149, 0x07EE81, (DWORD)&h_iface_bar_show);


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	icombatRect = (RECT*)0x528D90;
	imonitorRect = (RECT*)0x528300;
	//iface action bar rect
	iactionRect = (RECT*)0x528DC4;
	//iface item slot rect
	iitemSlotRect = (RECT*)0x528D6C;

	imonitor_currentLine = (LONG*)0x57FB54;

	imonitor_currentLine2 = (LONG*)0x57FB48;

	someFlag = (BYTE*)0x520734;

	timer1 = (LONG*)0x6BCCF8;

	imonitor_time = (LONG*)0x57FB58;

	is_imonitor = (DWORD*)0x5282FC;

	imonitor_numVisibleLines = (LONG*)0x57FB40;

	lp_imonitorBgBuff = (BYTE**)0x57FB3C;

	imonitor_upButtRef = (LONG*)0x528314;
	imonitor_downButtRef = (LONG*)0x528310;

	IMONITOR_COLOUR = (BYTE**)0x43171A;//0xD7;

	IMONITOR_FONT = (LONG*)0x4316F7;//101;

	if (IFACE_BAR_WIDTH > 640) {
		//draw iface background
		FuncWrite32(0x45D051, 0x07BE67, (DWORD)&FixIfaceMainBGDraw);

		//create inv button
		FuncWrite32(0x45D10F, 0x081B54, (DWORD)&fix_iface_button);
		//create game options button
		FuncWrite32(0x45D1EE, 0x081A75, (DWORD)&fix_iface_button);
		//create skilldex button
		FuncWrite32(0x45D30E, 0x081955, (DWORD)&fix_iface_button);
		//create map button
		FuncWrite32(0x45D445, 0x08181E, (DWORD)&fix_iface_button);
		//create pip button
		FuncWrite32(0x45D534, 0x08172F, (DWORD)&fix_iface_button);
		//create char button
		FuncWrite32(0x45D623, 0x081640, (DWORD)&fix_iface_button);
		//create attack button
		FuncWrite32(0x45D7B6, 0x0814AD, (DWORD)&fix_iface_button);
		//create switch slot button
		FuncWrite32(0x45D8F0, 0x081373, (DWORD)&fix_iface_button);


		//action points-----
		  //action points background copy and paste
		FuncWrite32(0x45DA5D, 0x07B45B, (DWORD)&FixIfaceActionPointsBGCopy);
		FuncWrite32(0x45E544, 0x07A974, (DWORD)&FixIfaceActionPointsBGDraw);
		FuncWrite32(0x45E5E0, 0x07A8D8, (DWORD)&FixIfaceActionPointsBGDraw);
		FuncWrite32(0x45E62E, 0x07A88A, (DWORD)&FixIfaceActionPointsBGDraw);


		//combat-----
		  //create turn button
		FuncWrite32(0x45FF84, 0x07ECDF, (DWORD)&fix_iface_button);
		//create cmbt button
		FuncWrite32(0x4600E4, 0x07EB7F, (DWORD)&fix_iface_button);

		FuncWrite32(0x45F12B, 0x079D8D, (DWORD)&FixIfaceCombatBGDraw);
		FuncWrite32(0x45F17C, 0x079D3C, (DWORD)&FixIfaceCombatBGDraw);

		FuncWrite32(0x45F288, 0x079C30, (DWORD)&FixIfaceCombatBGDraw);
		FuncWrite32(0x45F2D2, 0x079BE6, (DWORD)&FixIfaceCombatBGDraw);

		FuncWrite32(0x45F373, 0x079B80, (DWORD)&FixIfaceCombatBGDraw2);
		FuncWrite32(0x45F407, 0x079AEC, (DWORD)&FixIfaceCombatBGDraw2);


		//counters-----
		MemWrite8(0x4602FA, 0x01, 0x89);
		MemWrite8(0x460302, 0xC1, 0xE8);
		FuncWrite32(0x460303, 0xC20107E0, (DWORD)&fix_counter_pos);

		FuncWrite32(0x4603C5, 0x078AF3, (DWORD)&FixIfaceCounterDraw);

		FuncWrite32(0x4603F9, 0x078ABF, (DWORD)&FixIfaceCounterDraw);

		FuncWrite32(0x46042D, 0x078A8B, (DWORD)&FixIfaceCounterDraw);

		FuncWrite32(0x460476, 0x078A42, (DWORD)&FixIfaceCounterDraw);

		FuncWrite32(0x4605A2, 0x078916, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x4605F2, 0x0788C6, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x460648, 0x078870, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x4606A1, 0x078817, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x4606DE, 0x0787DA, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x460711, 0x0787A7, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x460761, 0x078757, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x460786, 0x078732, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x4607AC, 0x07870C, (DWORD)&FixIfaceCounterDraw);
		FuncWrite32(0x4607ED, 0x0786CB, (DWORD)&FixIfaceCounterDraw);


		//monitor-----
		MemWrite8(0x431250, 0x53, 0xE9);
		FuncWrite32(0x431251, 0x55565251, (DWORD)&imonitor_setup);

		MemWrite8(0x4314A5, 0x83, 0xE9);
		FuncWrite32(0x4314A6, 0x158B18EC, (DWORD)&imonitor_insert_text_ch);

		MemWrite8(0x431694, 0x53, 0xE9);
		FuncWrite32(0x431695, 0x57565251, (DWORD)&imonitor_print_text);

		MemWrite8(0x431434, 0x53, 0xE9);
		FuncWrite32(0x431435, 0x3D835251, (DWORD)&imonitor_clear_text);

		MemWrite8(0x431648, 0x53, 0xE9);
		FuncWrite32(0x431649, 0x3D835251, (DWORD)&imonitor_clear_text);

		MemWrite8(0x4313F5, 0x85, 0xE9);
		FuncWrite32(0x4313F6, 0x8D1D7EDB, (DWORD)&imonitor_clear_text2);
	}

	//ammo bar-----
	MemWrite16(0x4601A0, 0x5153, 0xE990);
	FuncWrite32(0x4601A2, 0x10EC8356, (DWORD)&draw_ammo_bar);


	F_RedrawAmmoBar = (LONG(__cdecl*)(void))0x45EF38;
	F_DRAW_AMMO_BAR = (void*)0x45EF38;

	F_DRAW_IFACE_HP_NUM = (void*)0x45E2D8;

	F_DRAW_IFACE_AC_NUM = (void*)0x45E4A8;

	F_DRAW_IFACE_AP_BAR = (void*)0x45E50C;

	winBuff_Iface = (BYTE**)0x5AD974;
}


//_______________________________
void IfaceFixes_MULTI(int region) {

	buttRef_ifaceInv = (LONG*)FixAddress(0x518F18);
	buttRef_ifaceOptions = (LONG*)FixAddress(0x518F24);
	buttRef_ifaceSkill = (LONG*)FixAddress(0x518F30);
	buttRef_ifaceMap = (LONG*)FixAddress(0x518F40);
	buttRef_ifacePip = (LONG*)FixAddress(0x518F50);
	buttRef_ifaceChar = (LONG*)FixAddress(0x518F5C);
	buttRef_ifaceAttack = (LONG*)FixAddress(0x518F68);
	buttRef_ifaceSwitchHand = (LONG*)FixAddress(0x518F8C);
	buttRef_ifaceTurn = (LONG*)FixAddress(0x518FB0);
	buttRef_ifaceCmbt = (LONG*)FixAddress(0x518FBC);

	//NOTIFY-BAR SETUP
	FuncReplace32(0x461600, 0x074C34, (DWORD)&h_iface_sub_win);

	//SKILLBAR WINDOW SETUP
	FuncReplace32(0x4AC261, 0x029FD3, (DWORD)&h_iface_skill_win);

	FuncReplace32(0x4AC683, 0x029DE1, (DWORD)&h_iface_skill_kill);

	//IFACE SETUP
	FuncReplace32(0x45D8BD, 0x078977, (DWORD)&IfaceSetup);

	FuncReplace32(0x45E847, 0x077C1D, (DWORD)&h_iface_bar_kill);

	FuncReplace32(0x45E3F6, 0x078A6A, (DWORD)&h_iface_bar_hide);

	FuncReplace32(0x45E8FC, 0x078564, (DWORD)&h_iface_bar_hide);

	FuncReplace32(0x45E9FE, 0x078462, (DWORD)&h_iface_bar_hide);

	FuncReplace32(0x45EA49, 0x07835F, (DWORD)&h_iface_bar_show);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	icombatRect = (RECT*)FixAddress(0x518FA0);
	imonitorRect = (RECT*)FixAddress(0x518510);
	//iface action bar rect
	iactionRect = (RECT*)FixAddress(0x518FD4);
	//iface item slot rect
	iitemSlotRect = (RECT*)FixAddress(0x518F7C);
	imonitor_currentLine = (LONG*)FixAddress(0x56FB54);
	imonitor_currentLine2 = (LONG*)FixAddress(0x56FB48);

	someFlag = (BYTE*)FixAddress(0x510944);
	timer1 = (LONG*)FixAddress(0x6AC788);
	imonitor_time = (LONG*)FixAddress(0x56FB58);
	is_imonitor = (DWORD*)FixAddress(0x51850C);

	imonitor_numVisibleLines = (LONG*)FixAddress(0x56FB40);

	lp_imonitorBgBuff = (BYTE**)FixAddress(0x56FB3C);

	imonitor_upButtRef = (LONG*)FixAddress(0x518524);
	imonitor_downButtRef = (LONG*)FixAddress(0x518520);

	IMONITOR_COLOUR = (BYTE**)FixAddress(0x431AF2);//0xD7;

	IMONITOR_FONT = (LONG*)FixAddress(0x431ADB);//101;


	if (IFACE_BAR_WIDTH > 640) {
		//draw iface background
		FuncReplace32(0x45D951, 0x075D7F, (DWORD)&FixIfaceMainBGDraw);

		//create inv button
		FuncReplace32(0x45DA0F, 0x07A84D, (DWORD)&fix_iface_button);
		//create game options button
		FuncReplace32(0x45DAEE, 0x07A76E, (DWORD)&fix_iface_button);
		//create skilldex button
		FuncReplace32(0x45DC0E, 0x07A64E, (DWORD)&fix_iface_button);
		//create map button
		FuncReplace32(0x45DD45, 0x07A517, (DWORD)&fix_iface_button);
		//create pip button
		FuncReplace32(0x45DE34, 0x07A428, (DWORD)&fix_iface_button);
		//create char button
		FuncReplace32(0x45DF23, 0x07A339, (DWORD)&fix_iface_button);
		//create attack button
		FuncReplace32(0x45E0B6, 0x07A1A6, (DWORD)&fix_iface_button);
		//create switch slot button
		FuncReplace32(0x45E1F0, 0x07A06C, (DWORD)&fix_iface_button);

		//action points-----
		//action points background copy and paste
		FuncReplace32(0x45E35D, 0x075373, (DWORD)&FixIfaceActionPointsBGCopy);
		FuncReplace32(0x45EE44, 0x07488C, (DWORD)&FixIfaceActionPointsBGDraw);
		FuncReplace32(0x45EEE0, 0x0747F0, (DWORD)&FixIfaceActionPointsBGDraw);
		FuncReplace32(0x45EF2E, 0x0747A2, (DWORD)&FixIfaceActionPointsBGDraw);


		//combat-----
		//create turn button
		FuncReplace32(0x460884, 0x0779D8, (DWORD)&fix_iface_button);
		//create cmbt button
		FuncReplace32(0x4609E4, 0x077878, (DWORD)&fix_iface_button);

		FuncReplace32(0x45FA2B, 0x073CA5, (DWORD)&FixIfaceCombatBGDraw);
		FuncReplace32(0x45FA7C, 0x073C54, (DWORD)&FixIfaceCombatBGDraw);

		FuncReplace32(0x45FB88, 0x073B48, (DWORD)&FixIfaceCombatBGDraw);
		FuncReplace32(0x45FBD2, 0x073AFE, (DWORD)&FixIfaceCombatBGDraw);

		FuncReplace32(0x45FC73, 0x073A8D, (DWORD)&FixIfaceCombatBGDraw2);
		FuncReplace32(0x45FD07, 0x0739F9, (DWORD)&FixIfaceCombatBGDraw2);


		//counters-----
		MemWrite8(0x460BFA, 0x01, 0x89);
		MemWrite8(0x460C02, 0xC1, 0xE8);
		FuncWrite32(0x460C03, 0xC20107E0, (DWORD)&fix_counter_pos);

		FuncReplace32(0x460CC5, 0x072A0B, (DWORD)&FixIfaceCounterDraw);

		FuncReplace32(0x460CF9, 0x0729D7, (DWORD)&FixIfaceCounterDraw);

		FuncReplace32(0x460D2D, 0x0729A3, (DWORD)&FixIfaceCounterDraw);

		FuncReplace32(0x460D76, 0x07295A, (DWORD)&FixIfaceCounterDraw);

		FuncReplace32(0x460EA2, 0x07282E, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x460EF2, 0x0727DE, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x460F48, 0x072788, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x460FA1, 0x07272F, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x460FDE, 0x0726F2, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x461011, 0x0726BF, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x461061, 0x07266F, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x461086, 0x07264A, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x4610AC, 0x072624, (DWORD)&FixIfaceCounterDraw);
		FuncReplace32(0x4610ED, 0x0725E3, (DWORD)&FixIfaceCounterDraw);


		//monitor-----
		MemWrite8(0x431610, 0x53, 0xE9);
		FuncWrite32(0x431611, 0x55565251, (DWORD)&imonitor_setup);

		MemWrite8(0x431872, 0x83, 0xE9);
		FuncWrite32(0x431873, 0xC18908EC, (DWORD)&imonitor_insert_text);

		MemWrite8(0x431A78, 0x53, 0xE9);
		FuncWrite32(0x431A79, 0x57565251, (DWORD)&imonitor_print_text);

		MemWrite8(0x431800, 0x53, 0xE9);
		FuncWrite32(0x431801, 0x3D835251, (DWORD)&imonitor_clear_text);

		MemWrite8(0x431A2C, 0x53, 0xE9);
		FuncWrite32(0x431A2D, 0x3D835251, (DWORD)&imonitor_clear_text);

		MemWrite8(0x4317BF, 0x85, 0xE9);
		FuncWrite32(0x4317C0, 0x8D1D7EDB, (DWORD)&imonitor_clear_text2);
	}

	//ammo bar-----
	MemWrite16(0x460AA0, 0x5153, 0xE990);
	FuncWrite32(0x460AA2, 0x10EC8356, (DWORD)&draw_ammo_bar);

	F_RedrawAmmoBar = (LONG(__cdecl*)(void))FixAddress(0x45F838);
	F_DRAW_AMMO_BAR = (void*)FixAddress(0x45F838);

	F_DRAW_IFACE_HP_NUM = (void*)FixAddress(0x45EBD8);
	F_DRAW_IFACE_AC_NUM = (void*)FixAddress(0x45EDA8);
	F_DRAW_IFACE_AP_BAR = (void*)FixAddress(0x45EE0C);

	winBuff_Iface = (BYTE**)FixAddress(0x59D3F4);
}


//___________________________
void IfaceFixes(DWORD region) {

	IFACE_BAR_MODE = ConfigReadInt("IFACE", "IFACE_BAR_MODE", 0);
	if (IFACE_BAR_MODE > 1)IFACE_BAR_MODE = 0;
	IFACE_BAR_SIDE_ART = ConfigReadInt("IFACE", "IFACE_BAR_SIDE_ART", 0);
	//if(IFACE_BAR_SIDE_ART>99)IFACE_BAR_SIDE_ART=0;
	IFACE_BAR_SIDES_ORI = ConfigReadInt("IFACE", "IFACE_BAR_SIDES_ORI", 0);
	if (IFACE_BAR_SIDES_ORI > 1)IFACE_BAR_SIDES_ORI = 0;

	IFACE_BAR_WIDTH = ConfigReadInt("IFACE", "IFACE_BAR_WIDTH", 640);
	if (IFACE_BAR_WIDTH < 640)IFACE_BAR_WIDTH = 640;
	IFACE_BAR_GRAPHIC_WIDTH = IFACE_BAR_WIDTH;

	ALTERNATE_AMMO_METRE = ConfigReadInt("IFACE", "ALTERNATE_AMMO_METRE", 0);

	AMMO_LIGHT = (BYTE)ConfigReadInt("IFACE", "ALTERNATE_AMMO_LIGHT", 0xC4);
	AMMO_DARK = (BYTE)ConfigReadInt("IFACE", "ALTERNATE_AMMO_DARK", 0x4B);

	if (region == 4)
		IfaceFixes_CH(), iface_region = 4;
	else
		IfaceFixes_MULTI(region);
}
