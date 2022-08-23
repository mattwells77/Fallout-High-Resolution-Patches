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

#include "F_Controls.h"
#include "F_Windows.h"
#include "F_Art.h"
#include "F_Text.h"
#include "memwrite.h"
#include "convert.h"

DWORD buttMessage = 0;


//__________________________________
void SetCheckBoxButton(LONG buttRef) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;
	bool* pIsChecked = (bool*)butt->buffDnDis;

	if (butt->flags & FLG_ButtTglDn)
		butt->flags = butt->flags & 0xFFFDFFFF;//remove initial button toggled down flag.

	if (*pIsChecked == true) {
		*pIsChecked = false;
		//butt->buffDefault=butt->buffDn;
		butt->buffCurrent = butt->buffUp;
	}
	else if (*pIsChecked == false) {
		*pIsChecked = true;
		butt->buffCurrent = butt->buffDn;
	}
}


//_________________________________________
void SetCheckBoxVal(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;
	bool* pIsChecked = (bool*)butt->buffDnDis;

	if (butt->buffCurrent == butt->buffUp) {
		*pIsChecked = false;
	}
	else if (butt->buffCurrent == butt->buffDn) {
		*pIsChecked = true;
	}
}


//____________________________________________
void __declspec(naked) set_check_box_val(void) {

	__asm {
		pushad
		push edx
		push eax
		call SetCheckBoxVal
		add esp, 0x8
		popad
		ret
	}
}


//_________________________________________
void UpdateCheckBox(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;
	bool* pIsChecked = (bool*)butt->buffDnDis;

	if (*pIsChecked == true)
		butt->buffCurrent = butt->buffDn;
	else
		butt->buffCurrent = butt->buffUp;
}


//___________________________________________
void __declspec(naked) update_check_box(void) {

	__asm {
		pushad
		push edx
		push eax
		call UpdateCheckBox
		add esp, 0x8
		popad
		ret
	}
}


//________________________________________________________________________________
LONG CreateCheckBoxButton(LONG winRef, LONG x, LONG y, LONG key, bool* pIsChecked) {

	UNLSTDfrm* frmA = hrFrmList->LoadFrm("PRFXOUT.frm", ART_INTRFACE, 0);
	UNLSTDfrm* frmB = hrFrmList->LoadFrm("PRFXIN.frm", ART_INTRFACE, 0);
	LONG buttRef = CreateButton(winRef, x, y, frmA->frames[0].width, frmA->frames[0].height, -1, -1, key, key, frmA->frames[0].buff, frmB->frames[0].buff, 0, FLG_ButtReturnMsg | FLG_ButtTrans | FLG_ButtToggle);

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (*pIsChecked)
		butt->flags = butt->flags | FLG_ButtTglDn;

	butt->buffDnDis = (BYTE*)pIsChecked;
	SetButtonFunctions(buttRef, (void*)update_check_box, (void*)update_check_box, (void*)set_check_box_val, (void*)set_check_box_val);
	return buttRef;
}


//_____________________________________________________________
CONTROLbase* GetNamedControl(LONG winRef, const char* buttName) {

	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return nullptr;
	if (!win->ButtonList)
		return nullptr;

	ButtonStruct* butt = win->ButtonList;

	//find start of list - this is done in fallouts draw button fuction.
	while (butt->prevButton != nullptr)
		butt = butt->prevButton;

	DWORD hashName = hash(buttName);

	while (butt != nullptr) {
		if ((DWORD)butt->buffHvDis == hashName)
			return (CONTROLbase*)butt->buffDnDis;
		butt = butt->nextButton;
	}

	return nullptr;
}


//_______________________________________________________
CONTROLbase* GetNamedControl(LONG winRef, DWORD hashName) {
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return nullptr;
	if (!win->ButtonList)
		return nullptr;

	ButtonStruct* butt = win->ButtonList;

	//find start of list - this is done in fallouts draw button fuction.
	while (butt->prevButton != nullptr)
		butt = butt->prevButton;

	while (butt != nullptr) {
		if ((DWORD)butt->buffHvDis == hashName)
			return (CONTROLbase*)butt->buffDnDis;
		butt = butt->nextButton;
	}

	return nullptr;
}


//_______________________________________
LONG CONTROLbase::SetPosCurrent(LONG pos) {

	posCurrent = pos;

	if (flags & CTRL_MIN && pos < posMin)
		posCurrent = posMin;
	else if (flags & CTRL_MAX && pos > posMax)
		posCurrent = posMax;

	Draw();
	return posCurrent;
}


//_________________________
LONG CONTROLbase::PrevPos() {

	posCurrent -= posInc;
	if (flags & CTRL_MIN && posCurrent < posMin)
		posCurrent = posMin;
	Draw();
	return posCurrent;
}


//_________________________
LONG CONTROLbase::NextPos() {

	posCurrent += posInc;
	if (flags & CTRL_MAX && posCurrent > posMax)
		posCurrent = posMax;
	Draw();
	return posCurrent;
}


//_________________________
void CONTROLscrollv::Draw() {

	if (!buttRef)
		return;
	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef[0], &win);
	if (!butt)
		return;

	BYTE* drawPos = win->buff + (butt->rect.top * win->width) + butt->rect.left;

	LONG barWidth = butt->rect.right - butt->rect.left + 1;
	MemBlt8(buffBg, barWidth, posMax, barWidth, drawPos, win->width);

	UNLSTDfrm* frm = nullptr;
	if (isScrolling)
		frm = hrFrmList->LoadFrm("PRFSLDON.frm", ART_INTRFACE, 0);
	else
		frm = hrFrmList->LoadFrm("PRFSLDOF.frm", ART_INTRFACE, 0);

	drawPos += (posCurrent * win->width);
	MemBltMasked8(frm->frames[0].buff, frm->frames[0].width, frm->frames[0].height, frm->frames[0].width, drawPos, win->width);

	FDrawWinRectRelative(win, &butt->rect);
}


//_______________________________________________________________________________________
void CONTROLscrollv::Update(LONG inPosMin, LONG inPosMax, LONG inPosCurrent, DWORD flags) {

	if (controlMaster) {
		if (flags & CTRL_MIN)
			masterMin = controlMaster->GetPosMin();
		if (flags & CTRL_MAX)
			masterMax = controlMaster->GetPosMax();
		if (flags & CTRL_CURRENT)
			masterCurrent = controlMaster->GetPosCurrent();
	}
	else {
		if (flags & CTRL_MIN)
			masterMax = inPosMin;
		if (flags & CTRL_MAX)
			masterMin = inPosMax;
		if (flags & CTRL_CURRENT)
			masterCurrent = inPosCurrent;
	}

	stepLength = masterMax / (double)barLength;

	posCurrent = (LONG)((double)masterCurrent / stepLength);
	if (posCurrent < 0)
		posCurrent = 0;
	else if (posCurrent > posMax)
		posCurrent = posMax;

	Draw();
}


//______________________________________________
void CONTROLscrollv::SetScrollPos(LONG inBarPos) {

	posCurrent = inBarPos;
	if (posCurrent < 0)
		posCurrent = 0;
	else if (posCurrent > posMax)
		posCurrent = posMax;

	LONG masterPos = 0;
	masterPos = (LONG)(((double)posCurrent) * stepLength);
	if (controlMaster)
		masterCurrent = controlMaster->SetPosCurrent(masterPos);
	else
		masterCurrent = masterPos;

	Draw();
}


//_________________________________________
void ScrollListButt(LONG buttRef, LONG key) {

	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef, &win);
	if (!butt)
		return;
	CONTROLscrollv* scrollList = (CONTROLscrollv*)butt->buffDnDis;

	LONG mouseX = 0, mouseY = 0;
	scrollList->SetIsScrolling(true);
	while (GetMouseFlags() & (F_MSE_LPRESS | F_MSE_LHOLD)) {
		CheckButtons();
		F_GetMousePos(&mouseX, &mouseY);
		mouseY -= win->rect.top;
		mouseY -= butt->rect.top;
		scrollList->SetScrollPos(mouseY);
	}
	scrollList->SetIsScrolling(false);
	scrollList->Draw();
	buttMessage = scrollList->GetPosCurrent();
}


//___________________________________________
void __declspec(naked) scroll_list_butt(void) {

	__asm {
		pushad
		push edx
		push eax
		call ScrollListButt
		add esp, 0x8
		popad
		ret
	}
}


//_____________________________
void CONTROLscrollv::ScrollUp() {
	if (controlMaster) {
		masterCurrent = controlMaster->PrevPos();
	}
	else if (masterCurrent > masterMin) {
		masterCurrent--;
	}
}


//_______________________________________
void ScrollButtUp(LONG buttRef, LONG key) {
	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;
	CONTROLscrollv* scrollList = (CONTROLscrollv*)butt->buffDnDis;
	scrollList->ScrollUp();
	buttMessage = scrollList->GetPosCurrent();
}


//_________________________________________
void __declspec(naked) scroll_butt_up(void) {

	__asm {
		pushad
		push edx
		push eax
		call ScrollButtUp
		add esp, 0x8
		popad
		ret
	}
}



//_____________________________
void CONTROLscrollv::ScrollDn() {

	if (controlMaster) {
		masterCurrent = controlMaster->NextPos();
	}
	else if (masterCurrent < masterMax) {
		masterCurrent++;
	}
}


//_______________________________________
void ScrollButtDn(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;
	CONTROLscrollv* scrollList = (CONTROLscrollv*)butt->buffDnDis;
	scrollList->ScrollDn();
	buttMessage = scrollList->GetPosCurrent();
}


//_________________________________________
void __declspec(naked) scroll_butt_dn(void) {

	__asm {
		pushad
		push edx
		push eax
		call ScrollButtDn
		add esp, 0x8
		popad
		ret
	}
}

//PRFSLDOF.FRM
//PRFSLDON.FRM
//PREFSLDR.FRM
//SAVEBOX.FRM

//medium
//INVDNDS.FRM
//INVDNIN.FRM
//INVDNOUT.FRM
//INVUPDS.FRM
//INVUPIN.FRM
//INVUPOUT.FRM

//__________________________________________________________________________________________________________________________________________________________________________________________________________________
CONTROLscrollv::CONTROLscrollv(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG inWidth, LONG inHeight, LONG keyCode, LONG inPosMin, LONG inPosMax, LONG inPosCurrent, CONTROLbase* inControlMaster) {

	winRef = inWinRef;
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;

	controlID = hash(controlName);
	typeID = CID_SCROLL_V;
	flags = CTRL_MIN | CTRL_MAX | CTRL_CURRENT;
	isScrolling = false;
	controlMaster = inControlMaster;
	x = inX;
	y = inY;
	height = inHeight;
	LONG barX = x + 1;
	LONG barY = y + 1;

	posMin = 0;
	posMax = height - 2;

	masterMax = inPosMax;
	masterMin = inPosMin;
	masterCurrent = inPosCurrent;
	if (controlMaster) {
		masterMax = controlMaster->GetPosMax();
		masterMin = controlMaster->GetPosMin();
		masterCurrent = controlMaster->GetPosCurrent();
	}

	buttRef = new LONG[3];

	UNLSTDfrm* frmA = hrFrmList->LoadFrm("INVUPOUT.frm", ART_INTRFACE, 0);
	UNLSTDfrm* frmB = hrFrmList->LoadFrm("INVUPIN.frm", ART_INTRFACE, 0);
	width = frmA->frames[0].width + 2;
	posMax -= frmA->frames[0].height;
	buttRef[1] = CreateButton(winRef, x + 1, y + 1, frmA->frames[0].width, frmA->frames[0].height, -1, -1, keyCode, -1, frmA->frames[0].buff, frmB->frames[0].buff, nullptr, 0x40);
	ButtonStruct* butt = GetButton(buttRef[1], nullptr);
	butt->buffDnDis = (BYTE*)this;
	SetButtonFunctions(buttRef[1], nullptr, nullptr, (void*)scroll_butt_up, nullptr);

	barY += frmA->frames[0].height;

	frmA = hrFrmList->LoadFrm("INVDNOUT.frm", ART_INTRFACE, 0);
	frmB = hrFrmList->LoadFrm("INVDNIN.frm", ART_INTRFACE, 0);
	posMax -= frmA->frames[0].height;
	buttRef[2] = CreateButton(winRef, x + 1, barY + posMax, frmA->frames[0].width, frmA->frames[0].height, -1, -1, keyCode, -1, frmA->frames[0].buff, frmB->frames[0].buff, nullptr, 0x40);
	butt = GetButton(buttRef[2], nullptr);
	butt->buffDnDis = (BYTE*)this;
	SetButtonFunctions(buttRef[2], nullptr, nullptr, (void*)scroll_butt_dn, nullptr);

	frmA = hrFrmList->LoadFrm("PRFSLDOF.frm", ART_INTRFACE, 0);
	frmB = hrFrmList->LoadFrm("PRFSLDON.frm", ART_INTRFACE, 0);

	barLength = posMax - frmA->frames[0].height;
	stepLength = masterMax / (double)barLength;
	LONG barWidth = frmA->frames[0].width;
	barX += ((width - barWidth) / 2);


	char bgBuffID[32];
	sprintf(bgBuffID, "SCROLL_V_BACK_%i\0", controlID);
	UNLSTDfrm* frmC = hrFrmList->NewFrm(barWidth, posMax, 1, 1, bgBuffID, ART_INTRFACE, 0);
	if (!frmC)
		MessageBox(nullptr, bgBuffID, "frm creation failed", MB_ICONEXCLAMATION | MB_OK);
	BYTE* buttOff = win->buff + barY * win->width + barX;
	MemBlt8(buttOff, barWidth, posMax, win->width, frmC->frames[0].buff, barWidth);
	buffBg = frmC->frames[0].buff;

	posCurrent = y - barY;
	buttRef[0] = CreateButton(winRef, barX, barY, barWidth, posMax, -1, -1, keyCode, -1, nullptr, nullptr, nullptr, 0x40);
	butt = GetButton(buttRef[0], nullptr);
	//butt->buffUpDis = (BYTE*)item;
	butt->buffDnDis = (BYTE*)this;

	SetButtonFunctions(buttRef[0], nullptr, nullptr, (void*)scroll_list_butt, nullptr);
}


//_____________________________________
LONG CONTROLlist::SetItem(LONG itemNum) {

	if (itemNum < itemsTotal && itemNum >= 0)
		itemSelected = itemNum;
	if (itemSelected < posCurrent)
		posCurrent = itemSelected;
	else if (itemSelected > posCurrent + itemsVisible - 1)
		posCurrent = itemSelected - itemsVisible + 1;

	Draw();
	return itemSelected;
}


//__________________________
LONG CONTROLlist::NextItem() {

	if (itemSelected < itemsTotal - 1)
		itemSelected++;
	if (itemSelected < posCurrent)
		posCurrent = itemSelected;
	else if (itemSelected > posCurrent + itemsVisible - 1)
		posCurrent = itemSelected - itemsVisible + 1;

	Draw();
	return itemSelected;
}


//__________________________
LONG CONTROLlist::PrevItem() {

	if (itemSelected > 0)
		itemSelected--;
	if (itemSelected < posCurrent)
		posCurrent = itemSelected;
	else if (itemSelected > posCurrent + itemsVisible - 1)
		posCurrent = itemSelected - itemsVisible + 1;
	Draw();
	return itemSelected;
}


//_________________________________________
void ListButtSelect(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, 0);
	CONTROLlist* buttList = (CONTROLlist*)butt->buffDnDis;
	buttList->SelectRelativeItem((LONG)butt->buffUpDis);
	buttMessage = buttList->GetSelectedItem();
}


//___________________________________________
void __declspec(naked) list_butt_select(void) {

	__asm {
		pushad
		push edx
		push eax
		call ListButtSelect
		add esp, 0x8
		popad
		ret
	}
}


//______________________________________________________________________________
void CONTROLlist::SetListTxt(char** inListTxt, DWORD inListSize, DWORD numChars) {

	if (listTxt) {
		for (int i = 0; i < itemsTotal; i++)
			delete[] listTxt[i];
		listTxt = nullptr;
		itemsTotal = 0;
	}

	if (inListSize <= 0)
		inListSize = 0;

	itemSelected = 0;
	itemsTotal = inListSize;
	listTxt = new char* [itemsTotal];

	posMin = 0;
	posMax = itemsTotal - itemsVisible;
	if (posMax < 0)
		posMax = 0;
	posCurrent = 0;
	flags = flags | CTRL_RANGED;
	posInc = 1;

	for (int i = 0; i < itemsTotal; i++) {
		listTxt[i] = new char[numChars];
		memset(listTxt[i], '\0', numChars);
		if (inListTxt)
			memcpy(listTxt[i], inListTxt[i], numChars - 1);
	}

	scroll->Update(0, 0, 0, CTRL_RANGED | CTRL_CURRENT);
	Draw();
}






//__________________________________________________________________________________________________________________________________________
CONTROLlist::CONTROLlist(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG inWidth, LONG inHeight, LONG keyCode, LONG inFont) {

	winRef = inWinRef;
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;
	controlID = hash(controlName);
	typeID = CID_LISTBOX;
	flags = CTRL_MIN | CTRL_MAX | CTRL_CURRENT;

	x = inX;
	y = inY;
	width = inWidth;
	height = inHeight;

	itemSelected = 0;
	itemsTotal = 0;
	listTxt = nullptr;

	font = inFont;
	LONG oldFont = GetFont();
	SetFont(font);
	LONG txtH = GetTextHeight();
	itemHeight = txtH;
	itemsVisible = height / itemHeight;

	posMin = 0;
	posMax = itemsTotal - itemsVisible;
	if (posMax < 0)
		posMax = 0;

	posCurrent = 0;

	buttRef = new LONG[itemsVisible];
	ButtonStruct* butt = nullptr;
	int item = 0;
	for (item = 0; item < itemsVisible; item++) {
		buttRef[item] = CreateButton(winRef, x, y + item * itemHeight, width, itemHeight, -1, -1, -1, keyCode, nullptr, nullptr, nullptr, 0x40);
		butt = GetButton(buttRef[item], nullptr);
		butt->buffUpDis = (BYTE*)item;
		butt->buffDnDis = (BYTE*)this;
		butt->buffHvDis = (BYTE*)controlID;
		SetButtonFunctions(buttRef[item], nullptr, nullptr, nullptr, (void*)list_butt_select);
	}

	char bgBuffID[32];
	sprintf(bgBuffID, "LIST_BACK_%i\0", controlID);
	UNLSTDfrm* frm = hrFrmList->NewFrm(width, height, 1, 1, bgBuffID, ART_INTRFACE, 0);
	if (!frm)
		MessageBox(nullptr, bgBuffID, "frm creation failed", MB_ICONEXCLAMATION | MB_OK);
	BYTE* buttOff = win->buff + y * win->width + x;
	MemBlt8(buttOff, width, height, win->width, frm->frames[0].buff, width);
	buffBg = frm->frames[0].buff;

	SetFont(oldFont);
	sprintf(bgBuffID, "LIST_SCROLL_%i\0", controlID);
	scroll = new CONTROLscrollv(bgBuffID, winRef, x + width, y, width, height, -1, 0, 0, 0, this);//controlID);
}


//______________________
void CONTROLlist::Draw() {

	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef[0], &win);

	BYTE* drawPos = win->buff + butt->rect.top * win->width + butt->rect.left;
	LONG buttoff = itemHeight * win->width;

	LONG oldFont = GetFont();
	SetFont(font);
	MemBlt8(buffBg, width, height, width, drawPos, win->width);

	RECT drawRect = { x, y, x + width - 1, y + height - 1 };

	if (!itemsTotal) {
		PrintText(drawPos, "List Empty", width, win->width, 0x39);
		FDrawWinRectRelative(win, &drawRect);
		return;
	}
	if (itemSelected < 0)
		itemSelected = 0;
	else if (itemSelected > itemsTotal - 1)
		itemSelected = itemsTotal - 1;
	if (itemSelected < 0)
		itemSelected = 0;

	LONG numLines = itemsVisible;
	if (numLines > itemsTotal - posCurrent)
		numLines = itemsTotal - posCurrent;

	for (LONG item = posCurrent; item < posCurrent + numLines; item++) {
		butt = GetButton(buttRef[item - posCurrent], 0);
		if (item == itemSelected)
			PrintText(drawPos, listTxt[item], width, win->width, 0x39);
		else
			PrintText(drawPos, listTxt[item], width, win->width, 0xD7);
		drawPos += buttoff;
	}

	FDrawWinRectRelative(win, &drawRect);
	SetFont(oldFont);

	scroll->Update(0, 0, 0, CTRL_CURRENT);
}


//_________________________
void CONTROLdialBig::Draw() {

	if (posCurrent < 0)
		posCurrent = 0;
	else if (posCurrent > posMax)
		posCurrent = posMax;

	PrintPosText();

	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef[0], &win);

	BYTE* drawPos = win->buff + butt->rect.top * win->width + butt->rect.left;
	MemBlt8(buffBg, width, height, width, drawPos, win->width);
	MemBltMasked8(buff[posCurrent], width, height, width, drawPos, win->width);
	FDrawWinRectRelative(win, &butt->rect);
}


//________________________________________
void BigDialSelect(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, 0);
	CONTROLdialBig* bigDial = (CONTROLdialBig*)butt->buffDnDis;
	if (!bigDial)
		return;
	bigDial->SetPosCurrent((LONG)butt->buffUpDis);

	buttMessage = bigDial->GetPosCurrent();
	PlayAcm("butout3");
}


//__________________________________________
void __declspec(naked) big_dial_select(void) {

	__asm {
		pushad
		push edx
		push eax
		call BigDialSelect
		add esp, 0x8
		popad
		ret
	}
}


//____________________________
LONG CONTROLdialBig::TurnPos() {

	if (posCurrent == posMin)
		isCW = true;
	else if (posCurrent == posMax)
		isCW = false;

	if (isCW)
		return NextPos();
	else
		return PrevPos();
}


//______________________________________
void BigDialTurn(LONG buttRef, LONG key) {

	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef, &win);

	CONTROLdialBig* bigDial = (CONTROLdialBig*)butt->buffDnDis;
	if (!bigDial)
		return;
	bigDial->TurnPos();

	buttMessage = bigDial->GetPosCurrent();
	PlayAcm("butout3");
}


//________________________________________
void __declspec(naked) big_dial_turn(void) {

	__asm {
		pushad
		push edx
		push eax
		call BigDialTurn
		add esp, 0x8
		popad
		ret
	}
}


//_________________________________
void CONTROLdialBig::PrintPosText() {

	if (!posTxt)
		return;
	LONG oldFont = GetFont();
	SetFont(4);

	LONG txtH = GetTextHeight();
	LONG txtW = 0;
	LONG txtX = 0, txtY = 0;

	WinStruct* win = GetWinStruct(winRef);
	ButtonStruct* butt = nullptr;

	for (int pos = 0; pos <= posMax; pos++) {
		butt = GetButton(buttRef[pos + 1], nullptr);
		txtW = GetTextWidth(posTxt[pos]);
		txtX = butt->rect.left;
		txtY = butt->rect.top;

		if (txtX > 0 && txtX + txtW < win->width && txtY > 0 && txtY + txtH < win->height)
			PrintText2(win->buff, posTxt[pos], txtW, txtX, txtY, win->width, 0x3D);
	}

	SetFont(oldFont);
}


//_______________________________________________________________________________________________________________________________________________
CONTROLdialBig::CONTROLdialBig(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG inNumPos, LONG InitPos, LONG keyCode, char** txt) {

	winRef = inWinRef;
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;
	if (!hrFrmList)
		return;

	controlID = hash(controlName);
	typeID = CID_BIG_DIAL;
	flags = CTRL_MIN | CTRL_MAX | CTRL_CURRENT;

	isCW = true;
	x = inX;
	y = inY;
	posMax = inNumPos - 1;
	buff = new BYTE * [inNumPos];
	LONG numItemsMax = 4;
	UNLSTDfrm* frm = hrFrmList->LoadFrm("PRFBKNBS.frm", ART_INTRFACE, 0);
	width = frm->frames[0].width;
	height = frm->frames[0].height / numItemsMax;
	DWORD offset = 0;
	for (int i = 0; i < numItemsMax; i++) {
		buff[i] = frm->frames[0].buff + offset;
		offset += width * height;//2162;
	}

	size_t numChars = 0;
	if (txt) {
		posTxt = new char* [inNumPos];
		for (int i = 0; i < inNumPos; i++) {
			if (txt[i]) {
				numChars = strlen(txt[i]) + 1;
				posTxt[i] = new char[numChars];
				memset(posTxt[i], '\0', numChars);
				memcpy(posTxt[i], txt[i], numChars - 1);
			}
		}
	}
	else
		posTxt = nullptr;

	LONG oldFont = GetFont();
	SetFont(4);

	LONG txtH = GetTextHeight();
	LONG txtW = 0;
	LONG txtX = 0, txtY = 0;
	buttRef = new LONG[inNumPos + 1];
	ButtonStruct* butt = nullptr;

	for (int pos = 0; pos < inNumPos; pos++) {
		txtW = GetTextWidth(txt[pos]);
		switch (pos) {
		case 0:
			txtX = x - txtW, txtY = y + 10 - txtH / 2;
			break;
		case 1:
			txtX = x + width / 2 - txtW / 2, txtY = y - txtH;
			break;
		case 2:
			txtX = x + width, txtY = y + 10 - txtH / 2;
			break;
		case 3:
			txtX = x + width, txtY = y + height - 10 - txtH / 2;
			break;
		default:
			break;
		}

		if (txtX > 0 && txtX + txtW < win->width && txtY > 0 && txtY + txtH < win->height) {
			buttRef[pos + 1] = CreateButton(winRef, txtX, txtY, txtW, txtH, -1, -1, -1, keyCode, nullptr, nullptr, nullptr, 0x40);
			butt = GetButton(buttRef[pos + 1], nullptr);
			butt->buffUpDis = (BYTE*)pos;
			butt->buffDnDis = (BYTE*)this;
			SetButtonFunctions(buttRef[pos + 1], nullptr, nullptr, nullptr, (void*)big_dial_select);
		}
	}
	PrintPosText();

	char bgBuffID[128];
	sprintf(bgBuffID, "DIAL_BACK_%i\0", controlID);
	frm = hrFrmList->NewFrm(width, height, 1, 1, bgBuffID, ART_INTRFACE, 0);
	if (!frm)
		MessageBox(nullptr, bgBuffID, "frm creation failed", MB_ICONEXCLAMATION | MB_OK);
	BYTE* buttOff = win->buff + y * win->width + x;
	MemBlt8(buttOff, width, height, win->width, frm->frames[0].buff, width);
	buffBg = frm->frames[0].buff;

	buttRef[0] = CreateButton(winRef, x, y, width, height, -1, -1, -1, keyCode, nullptr, nullptr, 0, 0x40);
	ButtonStruct* dial = GetButton(buttRef[0], nullptr);
	dial->buffDnDis = (BYTE*)this;
	dial->buffHvDis = (BYTE*)controlID;
	SetButtonFunctions(buttRef[0], nullptr, nullptr, nullptr, (void*)big_dial_turn);

	SetFont(oldFont);
	posCurrent = InitPos;
	Draw();
}


//_________________________
void CONTROLEditBox::Draw() {

	if (!txt)
		return;

	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef[0], &win);

	BYTE* drawPos = win->buff + butt->rect.top * win->width + butt->rect.left;
	MemBlt8(buffBg, width, height, width, drawPos, win->width);
	LONG oldFont = GetFont();
	SetFont(font);
	PrintText(drawPos, txt, width, win->width, colour & 0xFF);
	SetFont(oldFont);
	FDrawWinRectRelative(win, &butt->rect);
}


//_______________________________________
void CONTROLEditBox::SetText(char* inTxt) {

	if (!txt || !inTxt)
		return;
	memset(txt, '\0', numChars + 2);
	memcpy(txt, inTxt, numChars);
	Draw();
}


//__________________________________
int isCondition(int ch, DWORD flags) {

	if (flags & CTRL_NUMERICAL)
		return isdigit(ch);
	else
		return isprint(ch);
}


//_____________________________
void CONTROLEditBox::EditText() {

	if (!editable)
		return;
	if (!txt)
		return;

	ButtonStruct* butt = GetButton(buttRef[0], nullptr);

	LONG newTick = 0;
	LONG oldTick = 0;
	LONG cursorSpeed = 140;
	char* txtTmp = new char[numChars + 2];
	memcpy(txtTmp, txt, numChars);
	int val = 0;
	BYTE* valBuff = nullptr;
	int key = '0';
	txt[0] = '|';
	txt[1] = '\0';
	LONG length = 0;

	while (isCondition(key, flags) || key <= 0 || key == VK_BACK) {
		key = CheckButtons();
		if (isCondition(key, flags) && length < numChars) {
			txt[length] = key;
			txt[length + 1] = '|';
			txt[length + 2] = '\0';
			length++;
		}
		else if (key == VK_BACK && length > 0) {
			length--;
			txt[length] = '|';
			txt[length + 1] = '\0';
		}
		else if (key == -2) {
			if (!IsMouseInRect(&butt->rect))
				key = VK_ESCAPE;
		}

		newTick = GetTickCount();//timer for redraw
		if (oldTick > newTick)
			oldTick = newTick;

		if (newTick - oldTick > cursorSpeed) {//time to rotate critter
			oldTick = newTick;
			if (txt[length] == '|')
				txt[length] = '\0';
			else {
				txt[length] = '|';
				txt[length + 1] = '\0';
			}
			Draw();
		}
	}
	if (key != VK_RETURN) {
		memset(txt, '\0', numChars + 2);
		memcpy(txt, txtTmp, numChars);
	}
	else if (txt[length] == '|')
		txt[length] = '\0';
	delete[] txtTmp;

	if (!preSet && flags & CTRL_NUMERICAL) {
		posCurrent = atoi(txt);
		if ((flags & CTRL_MIN) && posCurrent < posMin)
			posCurrent = posMin;
		else if ((flags & CTRL_MAX) && posCurrent > posMax)
			posCurrent = posMax;
		sprintf(txt, "%d\0", posCurrent);
	}

	Draw();
}


//______________________________________
void EditBoxEdit(LONG buttRef, LONG key) {

	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef, &win);

	CONTROLEditBox* editBox = (CONTROLEditBox*)butt->buffDnDis;
	if (!editBox)
		return;
	editBox->EditText();
	buttMessage = (DWORD)editBox->GetText();
}


//________________________________________
void __declspec(naked) edit_box_edit(void) {

	__asm {
		pushad
		push edx
		push eax
		call EditBoxEdit
		add esp, 0x8
		popad
		ret
	}
}


//___________________________________________________________________
void CONTROLEditBox::SetPreSets(char** inPreSets, LONG inNumPreSets) {

	if (!inPreSets)
		return;
	if (inNumPreSets <= 0)
		return;

	if (preSet) {
		for (int i = 0; i < numPreSets; i++)
			delete[] preSet[i];
		preSet = nullptr;
		numPreSets = 0;
	}

	numPreSets = inNumPreSets;
	preSet = new char* [numPreSets];

	posMin = 0;
	posMax = numPreSets - 1;
	posCurrent = -1;
	flags = flags | CTRL_RANGED;
	posInc = 1;

	for (int i = 0; i < numPreSets; i++) {
		preSet[i] = new char[numChars + 2];
		memset(preSet[i], '\0', numChars + 2);
		memcpy(preSet[i], inPreSets[i], numChars);

		if (atoi(txt) == atoi(preSet[i]))
			posCurrent = i;
	}

	if (posCurrent < posMin)
		posCurrent = posMin;

	if (preSet && posCurrent >= posMin && posCurrent <= posMax)
		memcpy(txt, preSet[posCurrent], numChars);

	Draw();
}


//__________________________________________
LONG CONTROLEditBox::SetPosCurrent(LONG pos) {

	CONTROLbase::SetPosCurrent(pos);

	if (preSet)// && posCurrent >= posMin && posCurrent <= posMax)
		memcpy(txt, preSet[posCurrent], numChars);
	else if (flags & CTRL_NUMERICAL)
		sprintf(txt, "%d", posCurrent);

	Draw();
	msg = (DWORD)txt;
	return posCurrent;
}

//____________________________
LONG CONTROLEditBox::PrevPos() {

	CONTROLbase::PrevPos();

	if (preSet)// && posCurrent >= posMin && posCurrent <= posMax)
		memcpy(txt, preSet[posCurrent], numChars);
	else if (flags & CTRL_NUMERICAL)
		sprintf(txt, "%d", posCurrent);

	Draw();
	msg = (DWORD)txt;
	return posCurrent;
}

//_____________________________
LONG CONTROLEditBox::NextPos() {

	CONTROLbase::NextPos();

	if (preSet)// && posCurrent >= posMin && posCurrent <= posMax)
		memcpy(txt, preSet[posCurrent], numChars);
	else if (flags & CTRL_NUMERICAL)
		sprintf(txt, "%d", posCurrent);

	Draw();
	msg = (DWORD)txt;
	return posCurrent;
}


//______________________________________________________________________________________________________________________________________________________________________________________
CONTROLEditBox::CONTROLEditBox(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG keyCode, LONG inFont, const char* inTxt, LONG inNumChars, DWORD inColour, DWORD inFlags) {

	winRef = inWinRef;
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;
	if (!hrFrmList)
		return;
	if (!inNumChars)
		return;

	controlID = hash(controlName);
	typeID = CID_EDIT_BOX;
	flags = inFlags;
	editable = true;
	numPreSets = 0;
	preSet = nullptr;

	x = inX;
	y = inY;

	font = inFont;
	numChars = inNumChars;
	txt = new char[numChars + 2];
	if (inTxt) {
		memset(txt, '\0', numChars + 2);
		memcpy(txt, inTxt, numChars);
		posCurrent = atoi(txt);
	}

	colour = inColour;

	LONG oldFont = GetFont();
	SetFont(font);
	width = GetMaxCharWidth() * numChars;
	height = GetTextHeight();
	buffBg = new BYTE[width * height];

	char bgBuffID[128];
	sprintf(bgBuffID, "EDIT_BACK_%i\0", controlID);
	UNLSTDfrm* frm = hrFrmList->NewFrm(width, height, 1, 1, bgBuffID, ART_INTRFACE, 0);
	if (!frm)
		MessageBox(nullptr, bgBuffID, "frm creation failed", MB_ICONEXCLAMATION | MB_OK);
	BYTE* buttOff = win->buff + y * win->width + x;
	MemBlt8(buttOff, width, height, win->width, frm->frames[0].buff, width);
	buffBg = frm->frames[0].buff;

	buttRef = new LONG[1];
	buttRef[0] = CreateButton(winRef, x, y, width, height, -1, -1, -1, keyCode, nullptr, nullptr, 0, 0x40);
	ButtonStruct* butt = GetButton(buttRef[0], nullptr);
	butt->buffDnDis = (BYTE*)this;
	butt->buffHvDis = (BYTE*)controlID;
	SetButtonFunctions(buttRef[0], nullptr, nullptr, nullptr, (void*)edit_box_edit);

	SetFont(oldFont);
	Draw();
}


//___________________________
LONG CONTROLUpDown::PrevPos() {

	if (buddy)
		posCurrent = buddy->PrevPos();
	else
		posCurrent = CONTROLbase::PrevPos();

	return posCurrent;
}


//________________________________________
void SpinnerButtUp(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;

	CONTROLUpDown* spinner = (CONTROLUpDown*)butt->buffDnDis;
	buttMessage = spinner->PrevPos();
	CONTROLbase* buddy = spinner->GetBuddy();
	if (buddy)
		buttMessage = buddy->GetMsg();
}


//__________________________________________
void __declspec(naked) spinner_butt_up(void) {

	__asm {
		pushad
		push edx
		push eax
		call SpinnerButtUp
		add esp, 0x8
		popad
		ret
	}
}


//____________________________
LONG CONTROLUpDown::NextPos() {
	if (buddy)
		posCurrent = buddy->NextPos();
	else
		posCurrent = CONTROLbase::NextPos();

	return posCurrent;
}


//________________________________________
void SpinnerButtDn(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;

	CONTROLUpDown* spinner = (CONTROLUpDown*)butt->buffDnDis;
	buttMessage = spinner->NextPos();
	CONTROLbase* buddy = spinner->GetBuddy();
	if (buddy)
		buttMessage = buddy->GetMsg();
}


//__________________________________________
void __declspec(naked) spinner_butt_dn(void) {

	__asm {
		pushad
		push edx
		push eax
		call SpinnerButtDn
		add esp, 0x8
		popad
		ret
	}
}


//____________________________________________________________________________________________________________________________________________________________________________________________________________
CONTROLUpDown::CONTROLUpDown(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG keyCode, LONG inPosMin, LONG inPosMax, LONG inPosCurrent, LONG inIncrement, CONTROLbase* inBuddy, DWORD inFlags) {

	winRef = inWinRef;
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;
	if (!hrFrmList)
		return;

	controlID = hash(controlName);
	typeID = CID_UP_DOWN;
	flags = inFlags;

	buddy = inBuddy;
	if (buddy) {
		buddy->SetPosIncrement(inIncrement);
		x = buddy->GetX();
		y = buddy->GetY();
		y += (buddy->GetHeight() / 2);
		DWORD buddyFlags = buddy->GetFlags();

		posCurrent = buddy->GetPosCurrent();
		if (CTRL_MIN)
			buddy->SetPosMin(inPosMin);
		else if (buddyFlags & CTRL_MIN)
			posMin = buddy->GetPosMin();
		if (CTRL_MAX)
			buddy->SetPosMax(inPosMax);
		else if (buddyFlags & CTRL_MAX)
			posMax = buddy->GetPosMax();
	}
	else {
		posInc = inIncrement;
		x = inX;
		y = inY;
		if (flags & CTRL_MIN)
			posMin = inPosMin;
		if (flags & CTRL_MAX)
			posMax = inPosMax;
		posCurrent = inPosCurrent;
	}

	//small
	//DNARWOFF.FRM
	//DNARWON.FRM
	//UPARWOFF.FRM
	//UPARWON.FRM
	//BARARRWS.FRM
	buttRef = new LONG[2];

	UNLSTDfrm* frmA = nullptr;
	UNLSTDfrm* frmB = nullptr;
	ButtonStruct* butt = nullptr;

	frmA = hrFrmList->LoadFrm("UPARWOFF.frm", ART_INTRFACE, 0);
	frmB = hrFrmList->LoadFrm("UPARWON.frm", ART_INTRFACE, 0);
	width = frmA->frames[0].width;
	height = frmA->frames[0].height;
	if (buddy) {
		x -= width;
		x -= 10;
		y -= height;
	}
	buttRef[0] = CreateButton(winRef, x, y, width, frmA->frames[0].height, -1, -1, keyCode, -1, frmA->frames[0].buff, frmB->frames[0].buff, nullptr, 0x40);
	butt = GetButton(buttRef[0], nullptr);
	butt->buffDnDis = (BYTE*)this;
	SetButtonFunctions(buttRef[0], nullptr, nullptr, (void*)spinner_butt_up, nullptr);

	LONG dnY = y + height;
	frmA = hrFrmList->LoadFrm("DNARWOFF.frm", ART_INTRFACE, 0);
	frmB = hrFrmList->LoadFrm("DNARWON.frm", ART_INTRFACE, 0);
	height += frmA->frames[0].height;
	buttRef[1] = CreateButton(winRef, x, dnY, width, frmA->frames[0].height, -1, -1, keyCode, -1, frmA->frames[0].buff, frmB->frames[0].buff, nullptr, 0x40);
	butt = GetButton(buttRef[1], nullptr);
	butt->buffDnDis = (BYTE*)this;
	SetButtonFunctions(buttRef[1], nullptr, nullptr, (void*)spinner_butt_dn, nullptr);
}


//________________________
void CONTROLStatic::Draw() {

	if (!txt)
		return;

	WinStruct* win = GetWinStruct(winRef);

	BYTE* drawPos = win->buff + y * win->width + x;
	MemBlt8(buffBg, width, height, width, drawPos, win->width);
	LONG oldFont = GetFont();
	SetFont(font);
	PrintText(drawPos, txt, width, win->width, colour & 0xFF);
	SetFont(oldFont);
	RECT rect = { x, y, x + width - 1, y + height - 1 };
	FDrawWinRectRelative(win, &rect);
}


//______________________________________
void CONTROLStatic::SetText(char* inTxt) {

	if (!inTxt)
		return;
	if (!txt)
		txt = new char[numChars + 2];
	memset(txt, '\0', numChars + 2);
	memcpy(txt, inTxt, numChars);
	Draw();
}


//______________________________________________________________________________________________________________________________________________________________________
CONTROLStatic::CONTROLStatic(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG inFont, const char* inTxt, LONG inNumChars, DWORD inColour, DWORD inFlags) {

	winRef = inWinRef;
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;
	if (!hrFrmList)
		return;

	controlID = hash(controlName);
	typeID = CID_STATIC;
	flags = inFlags;

	x = inX;
	y = inY;

	font = inFont;
	numChars = inNumChars;
	txt = new char[numChars + 2];
	if (inTxt) {
		memset(txt, '\0', numChars + 2);
		memcpy(txt, inTxt, numChars);
	}

	colour = inColour;

	LONG oldFont = GetFont();
	SetFont(font);
	width = GetMaxCharWidth() * numChars;
	height = GetTextHeight();
	buffBg = new BYTE[width * height];

	char bgBuffID[128];
	sprintf(bgBuffID, "STATIC_BACK_%i\0", controlID);
	UNLSTDfrm* frm = hrFrmList->NewFrm(width, height, 1, 1, bgBuffID, ART_INTRFACE, 0);
	if (!frm)
		MessageBox(nullptr, bgBuffID, "frm creation failed", MB_ICONEXCLAMATION | MB_OK);
	BYTE* buttOff = win->buff + y * win->width + x;
	MemBlt8(buttOff, width, height, win->width, frm->frames[0].buff, width);
	buffBg = frm->frames[0].buff;

	SetFont(oldFont);
	Draw();
}


//________________________
void CONTROLslider::Draw() {

	if (!buttRef)
		return;
	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef[0], &win);
	if (!butt)
		return;

	BYTE* drawPos = win->buff + (butt->rect.top * win->width) + butt->rect.left;
	MemBlt8(buffBg, width, height, width, drawPos, win->width);

	UNLSTDfrm* frm = nullptr;
	if (isScrolling)
		frm = hrFrmList->LoadFrm("PRFSLDON.frm", ART_INTRFACE, 0);
	else
		frm = hrFrmList->LoadFrm("PRFSLDOF.frm", ART_INTRFACE, 0);

	if (flags & CTRL_IS_VERTICAL)
		drawPos += (posCurrent * win->width);
	else
		drawPos += (posCurrent);

	MemBltMasked8(frm->frames[0].buff, frm->frames[0].width, frm->frames[0].height, frm->frames[0].width, drawPos, win->width);

	FDrawWinRectRelative(win, &butt->rect);

	isScrolling = false;
}


//________________________________________
LONG CONTROLslider::SetMasterPos(LONG pos) {

	LONG masterPos = pos - masterMin;
	posCurrent = (LONG)(((double)masterPos) * stepLength);

	if (posCurrent < 0)
		posCurrent = 0;
	else if (posCurrent > posMax)
		posCurrent = posMax;

	masterPos = (LONG)(((double)posCurrent) / stepLength);
	masterCurrent = masterMin + masterPos;

	isScrolling = false;
	Draw();

	return masterCurrent;
}


//______________________________
LONG CONTROLslider::SlideMinus() {

	LONG masterPos = masterCurrent - masterMin - 1;
	posCurrent = (LONG)(((double)masterPos) * stepLength);

	if (posCurrent < 0)
		posCurrent = 0;
	else if (posCurrent > posMax)
		posCurrent = posMax;

	masterPos = (LONG)(((double)posCurrent) / stepLength);
	masterCurrent = masterMin + masterPos;

	isScrolling = false;
	Draw();

	return masterCurrent;
}


//_____________________________
LONG CONTROLslider::SlidePlus() {

	LONG masterPos = masterCurrent - masterMin + 1;
	posCurrent = (LONG)(((double)masterPos) * stepLength);

	if (posCurrent < 0)
		posCurrent = 0;
	else if (posCurrent > posMax)
		posCurrent = posMax;

	masterPos = (LONG)(((double)posCurrent) / stepLength);
	masterCurrent = masterMin + masterPos;

	isScrolling = false;
	Draw();

	return masterCurrent;
}


//_____________________________________________
void CONTROLslider::SetSliderPos(LONG inBarPos) {

	posCurrent = inBarPos;
	if (posCurrent < 0)
		posCurrent = 0;
	else if (posCurrent > posMax)
		posCurrent = posMax;

	LONG masterPos = (LONG)(((double)posCurrent) / stepLength);
	masterCurrent = masterMin + masterPos;
	posCurrent = (LONG)(((double)masterPos) * stepLength);
	isScrolling = true;
	Draw();
}


//____________________________________
void SlideButt(LONG buttRef, LONG key) {

	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef, &win);
	if (!butt)
		return;
	CONTROLslider* slider = (CONTROLslider*)butt->buffDnDis;

	LONG mouseX = 0, mouseY = 0, pos;
	while (GetMouseFlags() & (F_MSE_LPRESS | F_MSE_LHOLD)) {
		CheckButtons();
		F_GetMousePos(&mouseX, &mouseY);
		if (slider->GetFlags() & CTRL_IS_VERTICAL)
			pos = mouseY - win->rect.top - butt->rect.top;
		else
			pos = mouseX - win->rect.left - butt->rect.left;
		slider->SetSliderPos(pos);

	}
	slider->Draw();

	buttMessage = slider->GetMasterPos();
}


//_____________________________________
void __declspec(naked) slide_butt(void) {

	__asm {
		pushad
		push edx
		push eax
		call SlideButt
		add esp, 0x8
		popad
		ret
	}
}


//____________________________________________________________________________________________________________________________________________________________________________________
CONTROLslider::CONTROLslider(const char* controlName, LONG inWinRef, LONG inX, LONG inY, DWORD inLength, LONG keyCode, LONG inPosMin, LONG inPosMax, LONG inPosCurrent, DWORD inFlags) {

	winRef = inWinRef;
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;

	controlID = hash(controlName);
	typeID = CID_SLIDE_H;
	flags = CTRL_MIN | CTRL_MAX | CTRL_CURRENT | inFlags;
	isScrolling = false;

	x = inX;
	y = inY;

	masterCurrent = inPosCurrent;
	masterMin = inPosMin;
	masterMax = inPosMax;
	DWORD masterSize = masterMax - masterMin;
	LONG masterpos = masterCurrent - masterMin;

	posMin = 0;
	posMax = inLength;

	buttRef = new LONG[1];

	UNLSTDfrm* frmA = hrFrmList->LoadFrm("PRFSLDOF.frm", ART_INTRFACE, 0);
	UNLSTDfrm* frmB = hrFrmList->LoadFrm("PRFSLDON.frm", ART_INTRFACE, 0);
	if (flags & CTRL_IS_VERTICAL) {
		height = inLength;
		width = frmA->frames[0].width;
		posMax -= frmA->frames[0].height;
	}
	else {
		width = inLength;
		height = frmA->frames[0].height;
		posMax -= frmA->frames[0].width;
	}
	stepLength = posMax / (double)masterSize;
	posCurrent = (LONG)(((double)masterpos) * stepLength);

	char bgBuffID[32];
	sprintf(bgBuffID, "SLIDER_H_BACK_%i\0", controlID);
	UNLSTDfrm* frmC = hrFrmList->NewFrm(width, height, 1, 1, bgBuffID, ART_INTRFACE, 0);
	if (!frmC)
		MessageBox(nullptr, bgBuffID, "frm creation failed", MB_ICONEXCLAMATION | MB_OK);
	BYTE* buttOff = win->buff + y * win->width + x;
	MemBlt8(buttOff, width, height, win->width, frmC->frames[0].buff, width);
	buffBg = frmC->frames[0].buff;


	buttRef[0] = CreateButton(winRef, x, y, width, height, -1, -1, keyCode, -1, nullptr, nullptr, nullptr, 0x40);
	ButtonStruct* butt = GetButton(buttRef[0], nullptr);
	butt->buffDnDis = (BYTE*)this;

	SetButtonFunctions(buttRef[0], nullptr, nullptr, (void*)slide_butt, nullptr);
	Draw();
}


//_________________________
void CONTROLTextBox::Draw() {

	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;
	if (!win->buff)
		return;
	BYTE* lineBuff = win->buff + y * win->width + x;
	MemBlt8(buffBg, width, height, width, lineBuff, win->width);

	LONG oldFont = GetFont();
	SetFont(font);
	DWORD lineOff = win->width * GetTextHeight();

	LONG lineLength = 1;
	LONG numLines = height / GetTextHeight();
	LONG numCharsWide = 256;
	LONG currentLine = 0;
	char* txtBuff = new char[numCharsWide];

	while (txt && *txt != '\0' && currentLine < numLines) {
		///lineLength=Str_FindLastWordMulti_NL(txt, width);
		lineLength = pStr_FindLastWord_NL(txt, width);

		if (*(txt + lineLength) == ' ')//add next char to end of line if equals space
			lineLength++;

		strncpy(txtBuff, txt, lineLength);
		txtBuff[lineLength] = '\0';

		if (lineLength == 0)
			*txt = '\0';
		else
			txt += lineLength;

		PrintText(lineBuff, txtBuff, width, win->width, colour & 0xFF);
		lineBuff += lineOff;
		currentLine++;
	}

	delete[] txtBuff;
	SetFont(oldFont);
	RECT rect = { x, y, x + width - 1, y + height - 1 };
	FDrawWinRectRelative(win, &rect);
}


//_______________________________________
void CONTROLTextBox::SetText(char* inTxt) {

	txt = inTxt;
	Draw();

}


//___________________________________________________________________________________________________________________________________________________________________
CONTROLTextBox::CONTROLTextBox(const char* controlName, LONG inWinRef, LONG inX, LONG inY, DWORD inWidth, DWORD inHeight, LONG inFont, DWORD inColour, DWORD inFlags) {

	winRef = inWinRef;
	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;
	if (!hrFrmList)
		return;

	controlID = hash(controlName);
	typeID = CID_TEXT_BOX;
	flags = inFlags;

	x = inX;
	y = inY;
	width = inWidth;
	height = inHeight;
	font = inFont;

	txt = nullptr;

	colour = inColour;

	buffBg = new BYTE[width * height];

	char bgBuffID[128];
	sprintf(bgBuffID, "TEXT_BOX_%i\0", controlID);
	UNLSTDfrm* frm = hrFrmList->NewFrm(width, height, 1, 1, bgBuffID, ART_INTRFACE, 0);
	if (!frm)
		MessageBox(nullptr, bgBuffID, "frm creation failed", MB_ICONEXCLAMATION | MB_OK);
	BYTE* buttOff = win->buff + y * win->width + x;
	MemBlt8(buttOff, width, height, win->width, frm->frames[0].buff, width);
	buffBg = frm->frames[0].buff;

	Draw();
}
