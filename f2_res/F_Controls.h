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

#define CID_SCROLL_V   0x100
#define CID_LISTBOX    0x101
#define CID_BIG_DIAL   0x102
#define CID_EDIT_BOX   0x103
#define CID_UP_DOWN    0x104
#define CID_STATIC     0x105
#define CID_SLIDE_H    0x106
#define CID_TEXT_BOX   0x107

#define CTRL_MIN         0x00000001
#define CTRL_MAX         0x00000002
#define CTRL_CURRENT     0x00000004
#define CTRL_RANGED      0x00000003

#define CTRL_NUMERICAL   0x01000000
#define CTRL_IS_VERTICAL 0x02000000

extern DWORD buttMessage;

void SetCheckBoxButton(LONG buttRef);
LONG CreateCheckBoxButton(LONG winRef, LONG x, LONG y, LONG key, bool* pIsChecked);


//_______________
class CONTROLbase {
protected:
	DWORD controlID;
	DWORD typeID;
	DWORD flags;
	CONTROLbase* buddy;
	DWORD msg;
	LONG winRef;
	LONG x;
	LONG y;
	LONG width;
	LONG height;
	BYTE* buffBg;
	LONG* buttRef;

	LONG posMin;
	LONG posMax;
	LONG posCurrent;
	LONG posInc;
public:
	CONTROLbase() {
		controlID = 0;
		typeID = 0;
		flags = 0;
		buddy = nullptr;
		msg = 0;
		winRef = 0;
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		buffBg = nullptr;
		buttRef = nullptr;

		posMin = 0;
		posMax = 0;
		posCurrent = 0;
		posInc = 1;
	}
	virtual ~CONTROLbase() {
		if (buffBg)
			buffBg = nullptr;
		if (buttRef)
			delete[] buttRef;
	}
	virtual void Draw() {};
	virtual void SetPosMin(LONG pos) { posMin = pos; flags = flags | CTRL_MIN; };
	virtual void SetPosMax(LONG pos) { posMax = pos; flags = flags | CTRL_MAX; };
	virtual void SetPosIncrement(LONG inc) { posInc = inc; };
	virtual LONG SetPosCurrent(LONG pos);

	virtual LONG PrevPos();
	virtual LONG NextPos();

	virtual LONG GetPosMin() { return posMin; };
	virtual LONG GetPosMax() { return posMax; };
	virtual LONG GetPosCurrent() { return posCurrent; };

	virtual LONG GetX() { return x; };
	virtual LONG GetY() { return y; };
	virtual LONG GetWidth() { return width; };
	virtual LONG GetHeight() { return height; };

	virtual DWORD GetFlags() { return flags; };
	virtual CONTROLbase* GetBuddy() { return buddy; };
	virtual DWORD GetMsg() { return msg; };
};


//______________________________________
class CONTROLscrollv :public CONTROLbase {
	LONG masterCurrent;
	LONG masterMin;
	LONG masterMax;
	LONG barLength;
	bool isScrolling;
	double stepLength;
	CONTROLbase* controlMaster;
public:
	CONTROLscrollv(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG inWidth, LONG inHeight, LONG keyCode, LONG inPosMin, LONG inPosMax, LONG inPosCurrent, CONTROLbase* inControlMaster);//DWORD inMasterID);
	~CONTROLscrollv() { controlMaster = nullptr; };
	void Draw();
	void ScrollUp();
	void ScrollDn();
	void Update(LONG inPosMin, LONG inPosMax, LONG inPosCurrent, DWORD flags);
	void SetScrollPos(LONG scrnY);
	bool SetIsScrolling(bool inIsScrolling) { isScrolling = inIsScrolling; return isScrolling; };
};


//___________________________________
class CONTROLlist :public CONTROLbase {
	LONG itemsTotal;
	LONG itemsVisible;
	LONG itemSelected;
	LONG itemHeight;
	DWORD listTxtSize;
	char** listTxt;
	LONG font;
	CONTROLscrollv* scroll;
public:
	CONTROLlist(const char* buttName, LONG inWinRef, LONG inX, LONG inY, LONG inWidth, LONG inHeight, LONG keyCode, LONG inFont);
	~CONTROLlist() {
		if (listTxt) {
			for (int i = 0; i < itemsTotal; i++)
				delete[] listTxt[i];
			delete[] listTxt;
		}
		if (scroll)
			delete scroll;
	}
	void Draw();
	LONG GetSelectedItem() { return itemSelected; };
	LONG GetTotalItems() { return itemsTotal; };
	LONG GetVisibleItems() { return itemsVisible; };

	LONG SetItem(LONG itemNum);
	void SetTopItem(LONG itemNum) { if (itemNum < itemsTotal - itemsVisible + 1 && itemNum >= 0)posCurrent = itemNum; Draw(); };
	void SelectRelativeItem(LONG itemNum) { itemSelected = itemNum + posCurrent; Draw(); };
	LONG NextItem();
	LONG PrevItem();

	void SetListTxt(char** inListTxt, DWORD inListSize, DWORD numChars);
};


//______________________________________
class CONTROLdialBig :public CONTROLbase {
	bool isCW;
	BYTE** buff;
	char** posTxt;
	LONG font;

public:
	CONTROLdialBig(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG inNumPos, LONG InitPos, LONG keyCode, char** txt);
	~CONTROLdialBig() {
		if (buff) {
			for (int i = 0; i < posMax; i++)
				buff[i] = nullptr;
			delete[] buff;
		}
		if (posTxt) {
			for (int i = 0; i < posMax; i++)
				delete[] posTxt[i];
			delete[] posTxt;
		}
	}
	void Draw();
	void PrintPosText();
	LONG TurnPos();
};


//______________________________________
class CONTROLEditBox :public CONTROLbase {
	LONG font;
	DWORD colour;
	LONG numChars;
	char* txt;
	LONG numPreSets;
	char** preSet;
	bool editable;
public:
	CONTROLEditBox(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG keyCode, LONG inFont, const char* inTxt, LONG inNumChars, DWORD inColour, DWORD inFlags);
	~CONTROLEditBox() {
		if (txt)
			delete[] txt;
		if (preSet) {
			for (int i = 0; i < numPreSets; i++)
				delete[] preSet[i];
			preSet = nullptr;
			numPreSets = 0;
		}
	}
	void Draw();
	void EditText();
	char* GetText() { return txt; };
	void SetText(char* inTxt);
	void SetPreSets(char** inPreSets, LONG inNumPreSets);

	LONG SetPosCurrent(LONG pos);
	LONG PrevPos();
	LONG NextPos();
	bool IsEditable() { return editable; };
	bool SetEditable(bool editFlag) { editable = editFlag; return editable; };
};


//____________________________________
class CONTROLUpDown :public CONTROLbase {
public:
	CONTROLUpDown(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG keyCode, LONG inPosMin, LONG inPosMax, LONG inPosCurrent, LONG inIncrement, CONTROLbase* inBuddy, DWORD inFlags);

	LONG PrevPos();
	LONG NextPos();
};



//____________________________________
class CONTROLStatic :public CONTROLbase {
	LONG font;
	DWORD colour;
	LONG numChars;
	char* txt;
public:
	CONTROLStatic(const char* controlName, LONG inWinRef, LONG inX, LONG inY, LONG inFont, const char* inTxt, LONG inNumChars, DWORD inColour, DWORD inFlags);
	~CONTROLStatic() {
		if (txt)
			delete[] txt;
	}
	void Draw();
	void SetText(char* inTxt);
};


//_____________________________________
class CONTROLslider :public CONTROLbase {
	LONG masterCurrent;
	LONG masterMin;
	LONG masterMax;
	bool isScrolling;
	double stepLength;
public:
	CONTROLslider(const char* controlName, LONG inWinRef, LONG inX, LONG inY, DWORD inLength, LONG keyCode, LONG inPosMin, LONG inPosMax, LONG inPosCurrent, DWORD flags);
	void Draw();
	void SetSliderPos(LONG inBarPos);
	LONG SlideMinus();
	LONG SlidePlus();
	LONG GetMasterPos() { return masterCurrent; };
	LONG SetMasterPos(LONG pos);
};


//______________________________________
class CONTROLTextBox :public CONTROLbase {
	LONG font;
	DWORD colour;
	char* txt;
public:
	CONTROLTextBox(const char* controlName, LONG inWinRef, LONG inX, LONG inY, DWORD inWidth, DWORD inHeight, LONG inFont, DWORD inColour, DWORD inFlags);
	~CONTROLTextBox() {
		if (txt)
			txt = nullptr;
	}
	void Draw();
	void SetText(char* inTxt);
};
