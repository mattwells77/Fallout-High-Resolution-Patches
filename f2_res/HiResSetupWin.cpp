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

#if !defined(VERSION_LEGACY)
#include "Dx9.h"
#endif

#include "fixes.h"
#include "get_reslist.h"
#include "F_Windows.h"
#include "F_File.h"
#include "F_Art.h"
#include "F_Msg.h"
#include "F_Text.h"
#include "configTools.h"
#include "memwrite.h"
#include "convert.h"
#include "F_Controls.h"
#include "WinFall.h"

#include "version.h"


char* hiResAboutString = nullptr;
char* graphicsModeString = nullptr;
char* graphicsModeDetailsString = nullptr;
char* basicModeString = nullptr;

bool isSfall = false;

int hiResWinRef = -1;

struct HIRESsettings {
	DWORD width;
	DWORD height;
	DWORD colours;
	DWORD frequency;
	DWORD scaler;
	bool isWindowed;
	bool invertResList;
	int ifaceArtNum;
	LONG panelSize[6];
	bool isIgnoreDistPC;
	bool isFogOfWar;
	LONG fogLightLevel;
	LONG numPathNodes;
};


//_____________________________________________
void LoadHiResSettings(HIRESsettings* settings) {

	settings->width = ConfigReadInt("MAIN", "SCR_WIDTH", 640);
	settings->height = ConfigReadInt("MAIN", "SCR_HEIGHT", 480);
	settings->colours = ConfigReadInt("MAIN", "COLOUR_BITS", 8);
	settings->frequency = ConfigReadInt("MAIN", "REFRESH_RATE", 0);
	settings->scaler = ConfigReadInt("MAIN", "SCALE_2X", 0);
	if (scaler > 0)
		scaler = 1;

	if (ConfigReadInt("MAIN", "WINDOWED", 0))
		settings->isWindowed = true;
	else
		settings->isWindowed = false;

	if (ConfigReadInt("HI_RES_PANEL", "DISPLAY_LIST_DESCENDING", 0))
		settings->invertResList = true;
	else
		settings->invertResList = false;

	settings->ifaceArtNum = ConfigReadInt("IFACE", "IFACE_BAR_SIDE_ART", 0);

	settings->panelSize[0] = ConfigReadInt("MOVIES", "MOVIE_SIZE", 0);
	settings->panelSize[1] = ConfigReadInt("STATIC_SCREENS", "SPLASH_SCRN_SIZE", 0);
	settings->panelSize[2] = ConfigReadInt("STATIC_SCREENS", "HELP_SCRN_SIZE", 0);
	settings->panelSize[3] = ConfigReadInt("STATIC_SCREENS", "DEATH_SCRN_SIZE", 0);
	settings->panelSize[4] = ConfigReadInt("STATIC_SCREENS", "END_SLIDE_SIZE", 0);
	settings->panelSize[5] = ConfigReadInt("MAINMENU", "MAIN_MENU_SIZE", 0);

	if (ConfigReadInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 0))
		settings->isIgnoreDistPC = true;
	else
		settings->isIgnoreDistPC = false;

	if (ConfigReadInt("MAPS", "FOG_OF_WAR", 0))
		settings->isFogOfWar = true;
	else
		settings->isFogOfWar = false;

	settings->fogLightLevel = ConfigReadInt("MAPS", "FOG_LIGHT_LEVEL", 0);

	settings->numPathNodes = ConfigReadInt("MAPS", "NumPathNodes", 0);
}


//___________________________________________
void HiResToolTipHvOn(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;

	char* txt = (char*)butt->buffHvDis;
	CONTROLTextBox* textBox = (CONTROLTextBox*)butt->buffDnDis;
	textBox->SetText(txt);
}


//______________________________________________
void __declspec(naked) hires_tooltip_hv_on(void) {

	__asm {
		pushad
		push edx
		push eax
		call HiResToolTipHvOn
		add esp, 0x8
		popad
		ret
	}
}


//____________________________________________
void HiResToolTipHvOff(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;

	CONTROLTextBox* textBox = (CONTROLTextBox*)butt->buffDnDis;
	textBox->SetText(nullptr);
}


//_______________________________________________
void __declspec(naked) hires_tooltip_hv_off(void) {

	__asm {
		pushad
		push edx
		push eax
		call HiResToolTipHvOff
		add esp, 0x8
		popad
		ret
	}
}


//_______________________________________
void ButtDnSound3(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;

	if ((LONG)butt->buffHvDis == 0) {
		PlayAcm("butin3");
		butt->buffHvDis = (BYTE*)-1;
	}
}


//_______________________________________
void ButtUpSound3(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;

	PlayAcm("butout3");

	if ((LONG)butt->buffHvDis == -1)
		butt->buffHvDis = (BYTE*)0;
}


//__________________________________________
void ButtHvOffSound3(LONG buttRef, LONG key) {

	ButtonStruct* butt = GetButton(buttRef, nullptr);
	if (!butt)
		return;

	if ((LONG)butt->buffHvDis == -1)
		butt->buffHvDis = (BYTE*)0;
}


//_________________________________________
void __declspec(naked) butt_dn_sound3(void) {

	__asm {
		pushad
		push edx
		push eax
		call ButtDnSound3
		add esp, 0x8
		popad
		ret
	}
}


//__________________________________________
void __declspec(naked) butt_up_sound3(void) {

	__asm {
		pushad
		push edx
		push eax
		call ButtUpSound3
		add esp, 0x8
		popad
		ret
	}
}


//____________________________________________
void __declspec(naked) butt_hvoff_sound3(void) {

	__asm {
		pushad
		push edx
		push eax
		call ButtHvOffSound3
		add esp, 0x8
		popad
		ret
	}
}


//___________
void butin3() {

	PlayAcm("butin3");
}


//____________
void butout3() {

	PlayAcm("butout3");
}


//______________________________________
int SetIfaceArt(int ifaceNum, bool init) {

	if (!init)
		ifaceNum++;

	int subWidth = 129;
	int subHeight = 94;
	char barArtName[32]{ 0 };

	sprintf(barArtName, "HR_IFACERHT%d.frm", ifaceNum);
	UNLSTDfrm* rightFrm = LoadUnlistedFrm(barArtName, ART_INTRFACE);
	UNLSTDfrm* frm = hrFrmList->GetFrm("IFACE_SELECT", ART_INTRFACE);
	if (rightFrm == nullptr) {
		ifaceNum = 0;
		memset(frm->frames[0].buff, 0xE4, frm->frames[0].width * frm->frames[0].height);
	}
	else {
		if (subWidth > rightFrm->frames[0].width)
			subWidth = rightFrm->frames[0].width;
		if (subHeight > rightFrm->frames[0].height)
			subHeight = rightFrm->frames[0].height;
		MemBlt8Stretch(rightFrm->frames[0].buff, subWidth, subHeight, rightFrm->frames[0].width, frm->frames[0].buff, frm->frames[0].width, frm->frames[0].height, false, false);
		delete rightFrm;
	}

	return ifaceNum;
}


//____________________________________________
void ButtIfaceSelectUp(LONG buttRef, LONG key) {

	WinStruct* win = nullptr;
	ButtonStruct* butt = GetButton(buttRef, &win);
	if (!butt)
		return;
	LONG* ifaceNum = (LONG*)butt->buffHvDis;
	*ifaceNum = SetIfaceArt(*ifaceNum, 0);
	FDrawWinRectRelative(win, &butt->rect);
	PlayAcm("butout3");
}


//_______________________________________________
void __declspec(naked) butt_iface_select_up(void) {

	__asm {
		pushad
		push edx
		push eax
		call ButtIfaceSelectUp
		add esp, 0x8
		popad
		ret
	}
}


//______________________________________
LONG HiResWinSetup(HIRESsettings* hrSet) {

	LONG winRef = Win_Create(SCR_WIDTH / 2 - 320, SCR_HEIGHT / 2 - 240, 640, 480, 0, 0x8);
	if (winRef == -1)return -1;

	WinStruct* win = GetWinStruct(winRef);
	if (!win)return -1;

	int oldFont = GetFont();

	UNLSTDfrm* frmBG = hrFrmList->LoadFrm("HR_SCRN_BG.FRM", ART_INTRFACE, 0);
	MemBlt8(frmBG->frames[0].buff, frmBG->frames[0].width, frmBG->frames[0].height, frmBG->frames[0].width, win->buff, win->width);
	hrFrmList->UnLoadFrm("HR_SCRN_BG.FRM", ART_INTRFACE);

	int buttRef = 0;
	//Done & Cancel buttons
	UNLSTDfrm* bgFrm = hrFrmList->LoadFrm("LILREDUP.frm", ART_INTRFACE, 0);
	UNLSTDfrm* frm = hrFrmList->LoadFrm("LILREDDN.frm", ART_INTRFACE, 0);
	buttRef = CreateButton(winRef, 521, 431, bgFrm->frames[0].width, bgFrm->frames[0].height, -1, -1, -1, VK_RETURN, bgFrm->frames[0].buff, frm->frames[0].buff, 0, FLG_ButtReturnMsg | FLG_ButtTrans);
	SetButtonFunctions(buttRef, nullptr, (void*)butt_hvoff_sound3, (void*)butt_dn_sound3, (void*)butt_up_sound3);
	buttRef = CreateButton(winRef, 391, 431, bgFrm->frames[0].width, bgFrm->frames[0].height, -1, -1, -1, VK_ESCAPE, bgFrm->frames[0].buff, frm->frames[0].buff, 0, FLG_ButtReturnMsg | FLG_ButtTrans);
	SetButtonFunctions(buttRef, nullptr, (void*)butt_hvoff_sound3, (void*)butt_dn_sound3, (void*)butt_up_sound3);

	frm = hrFrmList->NewFrm(128, 94, 2, 1, "IFACE_SELECT", ART_INTRFACE, 0);
	//iface sides select button
	UNLSTDfrm* hrIfaceSelect = LoadUnlistedFrm("HR_IFACE_SELECT.frm", ART_INTRFACE);
	MemBltMasked8(hrIfaceSelect->frames[0].buff, hrIfaceSelect->frames[0].width, hrIfaceSelect->frames[0].height, hrIfaceSelect->frames[0].width, win->buff + 406 + (290 + 20) * win->width, win->width);
	delete hrIfaceSelect;
	buttRef = CreateButton(winRef, 453, 297 + 20, 128, 94, -1, -1, -1, -1, frm->frames[0].buff, nullptr, nullptr, FLG_ButtReturnMsg | FLG_ButtTrans);
	ButtonStruct* butt = GetButton(buttRef, nullptr);
	//butt->buffHvDis = (BYTE*)0;
	hrSet->ifaceArtNum = SetIfaceArt(hrSet->ifaceArtNum, true);
	butt->buffHvDis = (BYTE*)&hrSet->ifaceArtNum;
	SetButtonFunctions(buttRef, nullptr, nullptr, nullptr, (void*)butt_iface_select_up);

	win = nullptr;
	SetFont(oldFont);

	return winRef;
}


//______________________________________________________________________________________________________________________________
void CreateToolTipButton(LONG winRef, LONG txtX, LONG txtY, LONG txtW, LONG txtH, char* toolTxt, CONTROLTextBox* toolTipTextBox) {

	LONG buttRef = 0;
	ButtonStruct* butt = nullptr;

	WinStruct* win = GetWinStruct(winRef);

	if (txtX < 0)
		txtX = 0;
	else if (txtX > win->width)
		txtX = win->width - 1;

	if (txtW < 0)
		txtW = 1;
	if (txtX + txtW > win->width)
		txtW = win->width - txtX;

	buttRef = CreateButton(winRef, txtX, txtY, txtW, txtH, -1, -1, -1, -1, nullptr, nullptr, nullptr, 0x0);
	butt = GetButton(buttRef, nullptr);
	if (butt) {
		butt->buffHvDis = (BYTE*)toolTxt;
		butt->buffDnDis = (BYTE*)toolTipTextBox;
		SetButtonFunctions(buttRef, (void*)hires_tooltip_hv_on, (void*)hires_tooltip_hv_off, nullptr, nullptr);
	}
}


//___________________________________________________________________________________
void SetupGeneralToolTips(LONG winRef, CONTROLTextBox* toolTipTextBox, MSGList* pMsg) {

	WinStruct* win = GetWinStruct(winRef);

	char* txt = nullptr, * toolTxt = nullptr;


	LONG txtW = 0, txtH = 0;
	LONG txtX = 0, txtY = 0;

	LONG oldFont = GetFont();

	///screen settings title text
	SetFont(104);
	txtH = GetTextHeight();

	txt = GetMsg(pMsg, 50, 2);
	txtW = GetTextWidth(txt);
	txtX = 180 - txtW / 2;
	txtY = 22 - txtH / 2;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = hiResAboutString;///GetMsg(pMsg, 500, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);


	SetFont(103);
	txtH = GetTextHeight();

	///Graphics Mode
	txt = graphicsModeString;//GetMsg(pMsg, 140, 2);
	txtW = GetTextWidth(txt);
	txtX = 40;
	txtY = 60;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = graphicsModeDetailsString;
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///fullscreen title text
	txt = GetMsg(pMsg, 204, 2);
	txtW = GetTextWidth(txt);
	txtX = 62;
	txtY = 136;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 501, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///colours text
	txt = GetMsg(pMsg, 102, 2);
	txtW = GetTextWidth(txt);
	txtX = 286 - txtW / 2;
	txtY = 189 - txtH / 2;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	if (basicModeString)
		toolTxt = basicModeString;
	else
		toolTxt = GetMsg(pMsg, 502, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Refresh Rate text
	txt = GetMsg(pMsg, 111, 2);
	txtW = GetTextWidth(txt);
	txtX = 286 - txtW / 2;
	txtY = 288 - txtH / 2;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	if (basicModeString)
		toolTxt = basicModeString;
	else
		toolTxt = GetMsg(pMsg, 503, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	txt = new char[8];
	sprintf(txt, "Hz");
	txtW = GetTextWidth(txt);
	PrintText2Win(winRef, txt, txtW, 313, 313, 0x3D);
	delete txt;


	///custom width
	txt = GetMsg(pMsg, 52, 2);
	txtW = GetTextWidth(txt);
	txtX = 137 - txtW / 2;
	txtY = 388;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 504, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///custom height text
	txt = GetMsg(pMsg, 53, 2);
	txtW = GetTextWidth(txt);
	txtX = 253 - txtW / 2;
	txtY = 388;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 505, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///window mode text
	txt = GetMsg(pMsg, 108, 2);
	txtW = GetTextWidth(txt);
	txtX = 234;//230;
	txtY = 130;//60;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	if (basicModeString)
		toolTxt = basicModeString;
	else
		toolTxt = GetMsg(pMsg, 506, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	SetFont(103);
	txtH = GetTextHeight();

	///Pixel size text
	txt = GetMsg(pMsg, 110, 2);
	txtW = GetTextWidth(txt);
	txtX = 66;
	txtY = 90;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	if (basicModeString)
		toolTxt = basicModeString;
	else
		toolTxt = GetMsg(pMsg, 510, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	SetFont(101);
	txtH = GetTextHeight();

	///Invert list text
	txt = GetMsg(pMsg, 107, 2);
	txtW = GetTextWidth(txt);
	PrintText2Win(winRef, txt, txtW, 96, 360 - txtH / 2, 0x3D);

	SetFont(103);
	txtH = GetTextHeight();

	///FOG OF WAR
	txt = GetMsg(pMsg, 256, 2);//fog enabled
	txtW = GetTextWidth(txt);
	txtX = 400;
	txtY = 62;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 511, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Fog Visibility
	txt = GetMsg(pMsg, 257, 2);
	txtW = GetTextWidth(txt);
	txtX = 400;
	txtY = 84;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 512, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	SetFont(101);
	txtH = GetTextHeight();

	///Fog Vis Off
	txt = GetMsg(pMsg, 203, 2);//off
	txtW = GetTextWidth(txt);
	txtX = 400;
	txtY = 105;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 513, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Fog Vis dark
	txt = GetMsg(pMsg, 258, 2);//dark
	txtW = GetTextWidth(txt);
	txtX = 400 + 30;
	txtY = 105;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 514, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Fog Vis brighter
	txt = GetMsg(pMsg, 259, 2);//brighter
	txtW = GetTextWidth(txt);
	txtX = 400 + 200 - txtW;
	txtY = 105;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 514, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);


	SetFont(103);
	txtH = GetTextHeight();

	///Ignore PC Scroll Limit
	txt = GetMsg(pMsg, 250, 2);//Ignore PC Scroll Limit
	txtW = GetTextWidth(txt);
	txtX = 400;
	txtY = 145;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 515, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Path Finding Range
	txt = GetMsg(pMsg, 253, 2);//Path Finding Range
	txtW = GetTextWidth(txt);
	txtX = 400;
	txtY = 174;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 516, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	SetFont(101);
	txtH = GetTextHeight();

	///Path Finding Normal
	txt = GetMsg(pMsg, 254, 2);//Normal
	txtW = GetTextWidth(txt);
	txtX = 400;
	txtY = 195;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 517, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Path Finding Max
	txt = GetMsg(pMsg, 255, 2);//Max
	txtW = GetTextWidth(txt);
	txtX = 400 + 200 - txtW;
	txtY = 195;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 518, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	SetFont(103);
	txtH = GetTextHeight();

	///Panel Scaling
	txt = GetMsg(pMsg, 150, 2);//Panel Scaling
	txtW = GetTextWidth(txt);
	txtX = 400;
	txtY = 232;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 519, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	SetFont(101);
	txtH = GetTextHeight();

	///Panel Scaling None
	txt = GetMsg(pMsg, 151, 2);//None
	txtW = GetTextWidth(txt);
	txtX = 500;
	txtY = 252;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 520, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Panel Scaling Fit
	txt = GetMsg(pMsg, 152, 2);//Fit
	txtW = GetTextWidth(txt);
	txtX = 500 + 50 - txtW / 2;
	txtY = 252;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 521, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Panel Scaling Fill
	txt = GetMsg(pMsg, 153, 2);//Fill
	txtW = GetTextWidth(txt);
	txtX = 500 + 100 - txtW;
	txtY = 252;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 522, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	SetFont(103);
	txtH = GetTextHeight();

	///IFace side bar text
	txt = GetMsg(pMsg, 106, 2);
	txtW = GetTextWidth(txt);
	txtX = 501 - txtW / 2;
	txtY = 272 + 18;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 523, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);


	///Done button text
	txt = GetMsg(pMsg, 54, 2);
	txtW = GetTextWidth(txt);
	txtX = 528 + 14;
	txtY = 440 - txtH / 2;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 524, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	///Cancel button text
	txt = GetMsg(pMsg, 55, 2);
	txtW = GetTextWidth(txt);
	txtX = 398 + 14;
	txtY = 440 - txtH / 2;
	PrintText2Win(winRef, txt, txtW, txtX, txtY, 0x3D);
	toolTxt = GetMsg(pMsg, 525, 2);
	CreateToolTipButton(winRef, txtX, txtY, txtW, txtH, toolTxt, toolTipTextBox);

	SetFont(oldFont);
}


//_____________________
void DestroyHiResFrms() {

	if (hrFrmList)
		delete hrFrmList;
	hrFrmList = nullptr;
}


//______________________________________
bool CheckSettings(HIRESsettings* hrSet) {

	bool settingsChanged = false;

	HIRESsettings hrSetOld;
	LoadHiResSettings(&hrSetOld);

	if (hrSetOld.width != hrSet->width) {
		ConfigWriteInt("MAIN", "SCR_WIDTH", hrSet->width);
		settingsChanged = true;
	}
	if (hrSetOld.height != hrSet->height) {
		ConfigWriteInt("MAIN", "SCR_HEIGHT", hrSet->height);
		settingsChanged = true;
	}
	if (hrSetOld.colours != hrSet->colours) {
		ConfigWriteInt("MAIN", "COLOUR_BITS", hrSet->colours);
		settingsChanged = true;
	}
	if (hrSetOld.frequency != hrSet->frequency) {
		ConfigWriteInt("MAIN", "REFRESH_RATE", hrSet->frequency);
		settingsChanged = true;
	}
	if (hrSetOld.scaler != hrSet->scaler) {
		ConfigWriteInt("MAIN", "SCALE_2X", hrSet->scaler);
		settingsChanged = true;
	}
	if (hrSetOld.isWindowed != hrSet->isWindowed) {
		if (hrSet->isWindowed)
			ConfigWriteInt("MAIN", "WINDOWED", 1);
		else
			ConfigWriteInt("MAIN", "WINDOWED", 0);
		settingsChanged = true;
	}
	if (hrSetOld.ifaceArtNum != hrSet->ifaceArtNum) {
		ConfigWriteInt("IFACE", "IFACE_BAR_SIDE_ART", hrSet->ifaceArtNum);
		IfaceSidesReload(hrSet->ifaceArtNum);
	}
	if (hrSetOld.invertResList != hrSet->invertResList) {
		ConfigWriteInt("HI_RES_PANEL", "DISPLAY_LIST_DESCENDING", hrSet->invertResList);
	}
	if (hrSetOld.panelSize[0] != hrSet->panelSize[0]) {
		ConfigWriteInt("MOVIES", "MOVIE_SIZE", hrSet->panelSize[0]);
		UpdateMovieGlobals();
	}
	if (hrSetOld.panelSize[1] != hrSet->panelSize[1])
		ConfigWriteInt("STATIC_SCREENS", "SPLASH_SCRN_SIZE", hrSet->panelSize[1]);
	if (hrSetOld.panelSize[2] != hrSet->panelSize[2])
		ConfigWriteInt("STATIC_SCREENS", "HELP_SCRN_SIZE", hrSet->panelSize[2]);
	if (hrSetOld.panelSize[3] != hrSet->panelSize[3])
		ConfigWriteInt("STATIC_SCREENS", "DEATH_SCRN_SIZE", hrSet->panelSize[3]);
	if (hrSetOld.panelSize[4] != hrSet->panelSize[4])
		ConfigWriteInt("STATIC_SCREENS", "END_SLIDE_SIZE", hrSet->panelSize[4]);
	if (hrSetOld.panelSize[5] != hrSet->panelSize[5]) {
		ConfigWriteInt("MAINMENU", "MAIN_MENU_SIZE", hrSet->panelSize[5]);
		ReSizeMainMenu();
	}

	bool isMapGlobalChanged = false;

	if (hrSetOld.isIgnoreDistPC != hrSet->isIgnoreDistPC) {
		if (hrSet->isIgnoreDistPC)
			ConfigWriteInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 1);
		else
			ConfigWriteInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 0);
		isMapGlobalChanged = true;
	}

	if (hrSetOld.isFogOfWar != hrSet->isFogOfWar) {
		if (hrSet->isFogOfWar)
			ConfigWriteInt("MAPS", "FOG_OF_WAR", 1);
		else
			ConfigWriteInt("MAPS", "FOG_OF_WAR", 0);
		isMapGlobalChanged = true;
	}
	if (hrSetOld.fogLightLevel != hrSet->fogLightLevel) {
		ConfigWriteInt("MAPS", "FOG_LIGHT_LEVEL", hrSet->fogLightLevel);
		isMapGlobalChanged = true;
	}
	if (hrSetOld.numPathNodes != hrSet->numPathNodes) {
		ConfigWriteInt("MAPS", "NumPathNodes", hrSet->numPathNodes);
		isMapGlobalChanged = true;
	}

	if (isMapGlobalChanged)
		UpdateMapSettings(REGION);

	return settingsChanged;
}


RESlist* resList = new RESlist;

//______________________________________________________________________________________
void SetResolutionList(HIRESsettings* hrSet, RESlist* resList, CONTROLlist* resListView) {

	DWORD item = 0;
	DWORD minWidth = 640 << hrSet->scaler;
	DWORD minHeight = 480 << hrSet->scaler;
	if (resList->CheckParameters(hrSet->colours, hrSet->frequency, minWidth, minHeight))
		resList->Sort(hrSet->invertResList);
	else {
		resList->Fill(hrSet->colours, hrSet->frequency, minWidth, minHeight, hrSet->invertResList);
		//MessageBox(nullptr, "SetResolutionList", "Hi-Res Patch Error",MB_ICONEXCLAMATION | MB_OK);
	}
	char** resStrings = new char* [resList->size()];
	RESdata* resData = nullptr;
	for (item = 0; item < resList->size(); item++) {
		resData = resList->at(item);
		resStrings[item] = new char[16];
		sprintf(resStrings[item], "%dx%d", resData->width, resData->height);
	}
	resListView->SetListTxt(resStrings, resList->size(), 16);
	for (item = 0; item < resList->size(); item++) {
		resData = resList->at(item);
		if (hrSet->width == (LONG)resData->width && hrSet->height == (LONG)resData->height)
			resListView->SetItem(item);

		delete[] resStrings[item];
	}
	delete[] resStrings;
	resStrings = nullptr;
}


//___________________________________________________________________________________________
void SetFreqencyPresets(HIRESsettings* hrSet, FREQlist* freqList, CONTROLEditBox* customFreq) {

	DWORD i = 0;
	freqList->Fill(hrSet->width, hrSet->height, hrSet->colours);
	char** freqtxt = new char* [freqList->size()];
	for (i = 0; i < freqList->size(); i++) {
		freqtxt[i] = new char[3 + 1];
		sprintf(freqtxt[i], "%d", freqList->at(i));
	}

	customFreq->SetPreSets(freqtxt, (LONG)freqList->size());

	for (i = 0; i < freqList->size(); i++) {
		if (hrSet->frequency == freqList->at(i))
			customFreq->SetPosCurrent(i);

		delete freqtxt[i];
	}
	delete[] freqtxt;

	hrSet->frequency = atoi(customFreq->GetText());
}


//___________________________________________________
void SetColourVal(HIRESsettings* hrSet, LONG dialPos) {

	if (dialPos == 0)hrSet->colours = 8;
	else if (dialPos == 1)hrSet->colours = 16;
	else if (dialPos == 2)hrSet->colours = 32;
}


//____________________________________________________________________________________________________________________________________________
void SetCustomWidthHeight(HIRESsettings* hrSet, RESlist* resList, CONTROLEditBox* customWidth, CONTROLEditBox* customHeight, LONG resListItem) {

	if (resList->size() > 0) {
		hrSet->width = resList->at(resListItem)->width;
		hrSet->height = resList->at(resListItem)->height;
	}
	else {
		hrSet->width = 0;
		hrSet->height = 0;
	}
	char txtWH[5]{ 0 };
	sprintf(txtWH, "%d\0", hrSet->width);
	customWidth->SetText(txtWH);
	sprintf(txtWH, "%d\0", hrSet->height);
	customHeight->SetText(txtWH);
}


//________________
bool HiResWindow() {

	int oldFont = GetFont();
	bool settingsChanged = 0;
	hrFrmList = new FRMlist;

	HIRESsettings hrSetTmp;
	LoadHiResSettings(&hrSetTmp);

	hiResWinRef = HiResWinSetup(&hrSetTmp);
	if (hiResWinRef == -1)
		return 0;

	MSGList* pMsg = new MSGList;
	if (!LoadMsgList(pMsg, "game\\ScrnSet.msg"))//load game pref scrn text from ScrnSet.msg
		return false;


	hiResAboutString = new char[256];

	sprintf(hiResAboutString, "%s v%s.\nCreated by \"Mash\" Matt Wells.", VER_FILE_DESCRIPTION_STR, VER_FILE_VERSION_STR);

	int GRAPHICS_MODE = graphicsMode;//ConfigReadInt("MAIN", "GRAPHICS_MODE", 0);
	char* gmodeString0 = GetMsg(pMsg, 140, 2);
	char* gmodeString1 = GetMsg(pMsg, 140 + GRAPHICS_MODE * 3 + 1, 2);
	char* gmodeString2 = GetMsg(pMsg, 140 + GRAPHICS_MODE * 3 + 2, 2);
	char* gmodeString3 = GetMsg(pMsg, 140 + GRAPHICS_MODE * 3 + 3, 2);
	char* gmodebool1 = nullptr;
	char* gmodebool2 = nullptr;
	graphicsModeString = new char[256];
	graphicsModeDetailsString = new char[256];

	if (GRAPHICS_MODE == 0) {
		basicModeString = GetMsg(pMsg, 207, 2);
		int sfalMode = SfallReadInt("Graphics", "Mode", 0);
		if (sfalMode == 4 || sfalMode == 5) {
			if (sfalMode == 4)
				gmodebool1 = GetMsg(pMsg, 204, 2);
			else if (sfalMode == 5)
				gmodebool1 = GetMsg(pMsg, 205, 2);
		}
		else {
			gmodeString2 = GetMsg(pMsg, 60, 2);
			gmodebool1 = GetMsg(pMsg, 60, 2);
		}
		gmodebool2 = GetMsg(pMsg, 60, 2);
	}
	else if (GRAPHICS_MODE == 1) {
		gmodebool1 = GetMsg(pMsg, 60, 2);
		gmodebool2 = GetMsg(pMsg, 60, 2);
	}
#if !defined (VERSION_LEGACY)
	else if (GRAPHICS_MODE == 2) {
		if (isSoftwareVertex)
			gmodebool1 = GetMsg(pMsg, 59, 2);//"False";
		else
			gmodebool1 = GetMsg(pMsg, 58, 2);//"True";
		if (isShader2_0)
			gmodebool2 = GetMsg(pMsg, 58, 2);//"True";
		else
			gmodebool2 = GetMsg(pMsg, 59, 2);//"False";
	}
#endif

	sprintf(graphicsModeString, "%s %d", gmodeString0, GRAPHICS_MODE);
	sprintf(graphicsModeDetailsString, "%s %s %s\n %s %s\n %s %s", GetMsg(pMsg, 100, 2), gmodeString0, gmodeString1, gmodeString2, gmodebool1, gmodeString3, gmodebool2);

	//windowed check box
	LONG buttWindowed = CreateCheckBoxButton(hiResWinRef, 210, 130, -1, &hrSetTmp.isWindowed);
	//invert res list checkbox
	LONG buttInvert = CreateCheckBoxButton(hiResWinRef, 70, 351, 0x704, &hrSetTmp.invertResList);

	SetFont(101);

	WinStruct* hiResWin = GetWinStruct(hiResWinRef);

	int resListX = 70, resListY = 190;
	int resListWidth = 118, resListHeight = 144;

	FREQlist* freqList = new FREQlist;
	freqList->Fill(hrSetTmp.width, hrSetTmp.height, hrSetTmp.colours);
	RESlist* resList = new RESlist;
	resList->Fill(hrSetTmp.colours, hrSetTmp.frequency, 640 << hrSetTmp.scaler, 480 << hrSetTmp.scaler, hrSetTmp.invertResList);

	CONTROLlist* resListView = new CONTROLlist("ResList", hiResWinRef, resListX, resListY, resListWidth, resListHeight, 0x1FF, 0x4);
	LONG resListItem = 0;

	SetResolutionList(&hrSetTmp, resList, resListView);


	LONG dialNum = 0;
	if (hrSetTmp.colours == 8)
		dialNum = 0;
	else if (hrSetTmp.colours == 16)
		dialNum = 1;
	else if (hrSetTmp.colours == 32)
		dialNum = 2;

	CONTROLdialBig* colourDial = nullptr;

	char* txts[]{
		GetMsg(pMsg, 103, 2),
		GetMsg(pMsg, 104, 2),
		GetMsg(pMsg, 105, 2)
	};
	colourDial = new CONTROLdialBig("HR_ColourDial", hiResWinRef, 260, 220, 3, dialNum, 0x5FF, txts);
	for (int t = 0; t < 3; t++)
		txts[t] = nullptr;

	//custom width and height buttons
	char txtWH[5]{ 0 };
	sprintf(txtWH, "%d\0", hrSetTmp.width);
	CONTROLEditBox* customWidth = new CONTROLEditBox("customWidth", hiResWinRef, 110, 420, 0x700, 4, txtWH, 4, 0xD7, CTRL_MIN | CTRL_NUMERICAL);
	sprintf(txtWH, "%d\0", hrSetTmp.height);
	CONTROLEditBox* customHeight = new CONTROLEditBox("customHeight", hiResWinRef, 227, 420, 0x701, 4, txtWH, 4, 0xD7, CTRL_MIN | CTRL_NUMERICAL);

	//refresh rate spinner
	sprintf(txtWH, "%d\0", 0);
	CONTROLEditBox* customFreq = new CONTROLEditBox("customFreq", hiResWinRef, 278, 313, 0x702, 4, txtWH, 3, 0xD7, CTRL_NUMERICAL);
	CONTROLUpDown* customFreqUpDn = new CONTROLUpDown("customFreqUpDn", hiResWinRef, 0, 0, 0x702, 0, 999, 0, 1, customFreq, CTRL_RANGED);
	SetFreqencyPresets(&hrSetTmp, freqList, customFreq);

	//pixel res spinner
	bool isScale2X = false;
	if (hrSetTmp.scaler)
		isScale2X = true;
	LONG scalerButt = CreateCheckBoxButton(hiResWinRef, 40, 90, 0x703, &isScale2X);

	SetFont(101);
	LONG txtH = GetTextHeight();

	SetFont(101);
	txtH = GetTextHeight();
	sprintf(txtWH, "%d\0", 0);
	CONTROLEditBox* panelScale = new CONTROLEditBox("panelScale", hiResWinRef, 397, 267 - txtH / 2, 0x706, 101, txtWH, 10, 0xD7, CTRL_RANGED);
	panelScale->SetEditable(false);
	CONTROLUpDown* panelScaleUpDn = new CONTROLUpDown("panelScaleUpDn", hiResWinRef, 0, 0, 0x706, 0, 5, 0, 1, panelScale, CTRL_RANGED);

	char* panel[]{
		GetMsg(pMsg, 154, 2),
		GetMsg(pMsg, 155, 2),
		GetMsg(pMsg, 156, 2),
		GetMsg(pMsg, 157, 2),
		GetMsg(pMsg, 158, 2),
		GetMsg(pMsg, 159, 2)
	};
	panelScale->SetPreSets(panel, 6);
	for (int p = 0; p < 6; p++)
		panel[p] = nullptr;
	panelScale->SetPosCurrent(0);

	CONTROLslider* panelScaleSlide = new CONTROLslider("panelScaleSlide", hiResWinRef, 500, 267, 100, 0x707, 0, 2, hrSetTmp.panelSize[panelScale->GetPosCurrent()], 0);

	//Fog Of War checkbox
	LONG checkFogOfWar = CreateCheckBoxButton(hiResWinRef, 376, 62, -1, &hrSetTmp.isFogOfWar);
	//Fog Light Level slider
	CONTROLslider* fogLevel = new CONTROLslider("fogLevel", hiResWinRef, 400, 120, 200, 0x70A, 0, 10, hrSetTmp.fogLightLevel, 0);

	//ignore scroll dist from player checkbox
	LONG checkIgnoreDistPC = CreateCheckBoxButton(hiResWinRef, 376, 145, -1, &hrSetTmp.isIgnoreDistPC);

	//pathNodes slider
	CONTROLslider* pathNodes = new CONTROLslider("pathNodes", hiResWinRef, 400, 210, 200, 0x70B, 1, 20, hrSetTmp.numPathNodes, 0);

	//tool tip window
	CONTROLTextBox* toolTipTextBox = new CONTROLTextBox("toolTipTextBox", hiResWinRef, 380, 14, 238, 32, 101, 0xD7, 0);
	SetupGeneralToolTips(hiResWinRef, toolTipTextBox, pMsg);

	int activeWinRef = hiResWinRef;
	WinStruct* activeWin = hiResWin;

	bool isMouse = IsMouseHidden();
	F_ShowMouse();

	ShowWin(activeWinRef);

	int button = 0;
	while (button != VK_ESCAPE && !IsExitGame()) {
		button = CheckButtons();

		switch (button) {
		case VK_RETURN:
			settingsChanged = CheckSettings(&hrSetTmp);
			button = VK_ESCAPE;
			break;
		case 's':
		case 'S':
			SetCheckBoxButton(buttWindowed);
			break;
		case 'a':
		case 'A':
			SetCheckBoxButton(buttInvert);
		case 0x704:
			SetResolutionList(&hrSetTmp, resList, resListView);
			resListItem = resListView->GetSelectedItem();
			SetCustomWidthHeight(&hrSetTmp, resList, customWidth, customHeight, resListItem);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 'r':
		case 'R':
			ShowWin(hiResWinRef);
			activeWinRef = hiResWinRef;
			activeWin = hiResWin;
			RedrawWin(activeWinRef);
			break;
		case ARROW_UP:
			resListItem = resListView->PrevItem();
			SetCustomWidthHeight(&hrSetTmp, resList, customWidth, customHeight, resListItem);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case ARROW_DOWN:
			resListItem = resListView->NextItem();
			SetCustomWidthHeight(&hrSetTmp, resList, customWidth, customHeight, resListItem);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 0x1FF:
			resListItem = buttMessage;
			SetCustomWidthHeight(&hrSetTmp, resList, customWidth, customHeight, resListItem);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 'w':
		case 'W':
			customWidth->EditText();
			hrSetTmp.width = atoi(customWidth->GetText());
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 'h':
		case 'H':
			customHeight->EditText();
			hrSetTmp.height = atoi(customHeight->GetText());
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 0x700:
			hrSetTmp.width = atoi((char*)buttMessage);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 0x701:
			hrSetTmp.height = atoi((char*)buttMessage);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 0x702:
			hrSetTmp.frequency = atoi((char*)buttMessage);
			break;
		case 0x703:
			if (isScale2X)hrSetTmp.scaler = 1;
			else hrSetTmp.scaler = 0;
			SetResolutionList(&hrSetTmp, resList, resListView);
			resListItem = resListView->GetSelectedItem();
			SetCustomWidthHeight(&hrSetTmp, resList, customWidth, customHeight, resListItem);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 0x706:
			panelScaleSlide->SetMasterPos(hrSetTmp.panelSize[panelScale->GetPosCurrent()]);
			break;
		case 0x707:
			hrSetTmp.panelSize[panelScale->GetPosCurrent()] = panelScaleSlide->GetMasterPos();
			break;

		case 0x70A:
			hrSetTmp.fogLightLevel = (LONG)buttMessage;
			break;
		case 0x70B:
			hrSetTmp.numPathNodes = (LONG)buttMessage;
			break;
		case 'i':
		case 'I':
			hrSetTmp.ifaceArtNum = SetIfaceArt(hrSetTmp.ifaceArtNum, false);
			FDrawWinRect(&activeWin->rect);
			break;
		case 'm':
		case 'M':
			SetColourVal(&hrSetTmp, colourDial->NextPos());
			SetResolutionList(&hrSetTmp, resList, resListView);
			resListItem = resListView->GetSelectedItem();
			SetCustomWidthHeight(&hrSetTmp, resList, customWidth, customHeight, resListItem);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		case 0x5FF:
			SetColourVal(&hrSetTmp, buttMessage);
			SetResolutionList(&hrSetTmp, resList, resListView);
			resListItem = resListView->GetSelectedItem();
			SetCustomWidthHeight(&hrSetTmp, resList, customWidth, customHeight, resListItem);
			SetFreqencyPresets(&hrSetTmp, freqList, customFreq);
			break;
		default:
			break;
		}
	}

	delete colourDial;
	colourDial = nullptr;
	delete resListView;
	delete customWidth;
	delete customHeight;
	delete customFreqUpDn;
	delete customFreq;

	delete panelScaleUpDn;
	delete panelScale;

	delete fogLevel;
	delete pathNodes;

	delete toolTipTextBox;

	delete[] hiResAboutString;
	hiResAboutString = nullptr;
	delete[] graphicsModeString;
	graphicsModeString = nullptr;
	delete[] graphicsModeDetailsString;
	graphicsModeDetailsString = nullptr;
	basicModeString = nullptr;
	if (isMouse)
		F_HideMouse();

	DestroyWin(hiResWinRef);
	DestroyMsgList(pMsg);
	hiResWinRef = -1;
	DestroyHiResFrms();
	SetFont(oldFont);
	return settingsChanged;
}


//________________
int SettingsMenu() {

	MSGList* pMsg = new MSGList;
	if (!LoadMsgList(pMsg, "game\\ScrnSet.msg"))
		return 1;

	UNLSTDfrm* hrOptBgFrm = LoadUnlistedFrm("HR_OPTIONS_BG.FRM", ART_INTRFACE);
	if (!hrOptBgFrm)
		return 1;

	//OPTIONS_BUTTON
	ButtonStruct* optButt = GetButton(OPTIONS_BUTTON, nullptr);
	int winRef = Win_Create(optButt->rect.left, optButt->rect.top, hrOptBgFrm->frames[0].width, hrOptBgFrm->frames[0].height, 0, 0x14);
	if (winRef == -1)
		return 1;

	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return 1;

	MemBlt8(hrOptBgFrm->frames[0].buff, hrOptBgFrm->frames[0].width, hrOptBgFrm->frames[0].height, hrOptBgFrm->frames[0].width, win->buff, win->width);
	delete hrOptBgFrm;

	UNLSTDfrm* hrOptUpFrm1 = LoadUnlistedFrm("HR_OPTIONS_UP.FRM", ART_INTRFACE);
	UNLSTDfrm* hrOptDnFrm1 = LoadUnlistedFrm("HR_OPTIONS_DN.FRM", ART_INTRFACE);

	UNLSTDfrm* hrOptUpFrm2 = LoadUnlistedFrm("HR_OPTIONS_UP.FRM", ART_INTRFACE);
	UNLSTDfrm* hrOptDnFrm2 = LoadUnlistedFrm("HR_OPTIONS_DN.FRM", ART_INTRFACE);


	int oldFont = GetFont();
	SetFont(103);

	char* text = GetMsg(pMsg, 0, 2);

	int txtHeight = GetTextHeight();
	int txtWidth = GetTextWidth(text);
	if (txtWidth > hrOptUpFrm1->frames[0].width)
		txtWidth = hrOptUpFrm1->frames[0].width;

	PrintText2(hrOptUpFrm1->frames[0].buff, text, txtWidth, hrOptUpFrm1->frames[0].width / 2 - txtWidth / 2, hrOptUpFrm1->frames[0].height / 2 - txtHeight / 2 + 2, hrOptUpFrm1->frames[0].width, 0x3D);
	PrintText2(hrOptDnFrm1->frames[0].buff, text, txtWidth, hrOptDnFrm1->frames[0].width / 2 - txtWidth / 2, hrOptDnFrm1->frames[0].height / 2 - 2 - txtHeight / 2 + 4, hrOptDnFrm1->frames[0].width, 0x3E);


	int buttRef = CreateButton(winRef, 17, 20, hrOptUpFrm1->frames[0].width, hrOptUpFrm1->frames[0].height, -1, -1, -1, 'g', hrOptUpFrm1->frames[0].buff, hrOptDnFrm1->frames[0].buff, 0, 0x20);
	SetButtonSounds(buttRef, (void*)butin3, (void*)butout3);


	text = GetMsg(pMsg, 1, 2);
	txtWidth = GetTextWidth(text);
	if (txtWidth > hrOptUpFrm1->frames[0].width)
		txtWidth = hrOptUpFrm1->frames[0].width;

	PrintText2(hrOptUpFrm2->frames[0].buff, text, txtWidth, hrOptUpFrm2->frames[0].width / 2 - txtWidth / 2, hrOptUpFrm2->frames[0].height / 2 - txtHeight / 2 + 2, hrOptUpFrm2->frames[0].width, 0x3D);
	PrintText2(hrOptDnFrm2->frames[0].buff, text, txtWidth, hrOptDnFrm2->frames[0].width / 2 - txtWidth / 2, hrOptDnFrm2->frames[0].height / 2 - 2 - txtHeight / 2 + 4, hrOptDnFrm2->frames[0].width, 0x3E);


	buttRef = CreateButton(winRef, 17, win->height - 20 - hrOptUpFrm2->frames[0].height, hrOptUpFrm2->frames[0].width, hrOptUpFrm2->frames[0].height, -1, -1, -1, 's', hrOptUpFrm2->frames[0].buff, hrOptDnFrm2->frames[0].buff, 0, 0x20);
	SetButtonSounds(buttRef, (void*)butin3, (void*)butout3);


	bool startMouseWin = IsMouseInRect(&win->rect);


	bool isMouse = IsMouseHidden();
	F_ShowMouse();

	ShowWin(winRef);


	int retVal = 0;
	int button = 0;

	while (button != VK_ESCAPE && !IsExitGame()) {
		button = CheckButtons();

		switch (button) {
		case 'g':
		case 'G':
			retVal = 1;
			button = VK_ESCAPE;
			break;
		case 's':
		case 'S':
			retVal = 2;
			button = VK_ESCAPE;
			break;
		case -2:
			if (!IsMouseInRect(&win->rect))
				button = VK_ESCAPE;
		default:
			if (startMouseWin && !IsMouseInRect(&win->rect))
				button = VK_ESCAPE;
			break;
		}
		RedrawWin(winRef);
	}

	delete hrOptUpFrm1;
	delete hrOptDnFrm1;
	delete hrOptUpFrm2;
	delete hrOptDnFrm2;

	DestroyMsgList(pMsg);
	SetFont(oldFont);

	DestroyWin(winRef);

	if (isMouse)
		F_HideMouse();

	return retVal;
}


//__________________________________________________________________________
int SetHiResDataPath(char* path1, int isFolder1, char* path2, int isFolder2) {

	if (FSetDataPath(path1, isFolder1, path2, isFolder2))
		return -1;

	char datPath[MAX_PATH];
	char patchesPath[MAX_PATH];
	ConfigReadString("MAIN", "f2_res_dat", "f2_res.dat", datPath, MAX_PATH);
	ConfigReadString("MAIN", "f2_res_patches", "data\\", patchesPath, MAX_PATH);
	FSetDataPath(datPath, 0, patchesPath, 1);

	return 0;
}


//________________________________________
void __declspec(naked) set_data_path(void) {

	__asm {
		push esi
		push edi
		push ebp

		push ecx
		push ebx
		push edx
		push eax
		call SetHiResDataPath
		add esp, 0x10

		pop ebp
		pop edi
		pop esi
		ret
	}
}


//____________________________
void HiResSettings(int region) {

	if (region == 4)
		FuncWrite32(0x443AC5, 0x0810F0, (DWORD)&set_data_path);
	else
		FuncReplace32(0x444255, 0x081AD7, (DWORD)&set_data_path);
}
