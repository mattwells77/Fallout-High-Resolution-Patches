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
#include "get_reslist.h"


//________________________________________________________
bool compare_ascending(const RESdata* a, const RESdata* b) {

	if (a->width == b->width)
		return a->height < b->height;
	else
		return a->width < b->width;
}


//_________________________________________________________
bool compare_descending(const RESdata* a, const RESdata* b) {

	if (a->width == b->width)
		return a->height > b->height;
	else
		return a->width > b->width;
}


//____________________________________________
DWORD RESlist::find(DWORD width, DWORD height) {

	for (DWORD item = 0; item < numRes; item++) {
		if (list[item]->width == width &&
			list[item]->height == height)// &&
			return item;


	}
	return 0;
}


//___________________
bool RESlist::Empty() {

	for (DWORD i = 0; i < numRes; i++) {
		free(list[i]);
		list[i] = nullptr;
	}
	if (list)
		free(list);

	list = nullptr;
	numRes = 0;
	return true;
}

//_______________________________________
void RESlist::push_back(RESdata* resData) {

	RESdata** tmp = list;
	tmp = (RESdata**)realloc(tmp, (numRes + 1) * sizeof(RESdata*));
	if (tmp) {
		list = tmp;
		list[numRes] = resData;
		numRes++;
	}
}


//_____________________________________________________________
bool RESlist::CheckParameters(DWORD colours, DWORD frequency, DWORD min_width, DWORD min_height) {

	if (colours == coloursCurrent && frequency == frequencyCurrent && minWidth == min_width && minHeight == min_height)
		return true;

	return false;
}

//____________________________________________________________________________________________________
bool RESlist::Fill(DWORD colours, DWORD frequency, DWORD min_width, DWORD min_height, bool isInverted) {

	minWidth = min_width;
	minHeight = min_height;
	coloursCurrent = colours;
	frequencyCurrent = frequency;

	if (list)
		Empty();

	RESdata* resData = nullptr;

	DEVMODE info;
	memset(&info, 0, sizeof(DEVMODE));
	info.dmSize = sizeof(DEVMODE);

	bool sameResFlag = false;

	for (int m = 0; EnumDisplaySettings(nullptr, m, &info); m++) {
		if (info.dmPelsWidth < minWidth || info.dmPelsHeight < minHeight)//exclude resolutions below 640x480
			continue;

		sameResFlag = false;

		for (DWORD item = 0; item < numRes; item++) {
			if (info.dmPelsWidth == list[item]->width && info.dmPelsHeight == list[item]->height)//exclude same resolution
				sameResFlag = true;
		}

		if (!sameResFlag) {//exclude same resolution at different colour bits
			info.dmBitsPerPel = colours;
			info.dmDisplayFrequency = frequency;
			EnumDisplaySettings(nullptr, m, &info);// test if resolution works then print
			if (ChangeDisplaySettings(&info, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
				resData = (RESdata*)malloc(sizeof(RESdata));
				resData->width = info.dmPelsWidth;
				resData->height = info.dmPelsHeight;
				push_back(resData);
				resData = nullptr;
			}
		}
	}

	if (!numRes)
		return false;

	Sort(isInverted);
	return true;
}


//_________________________________________________________________________
bool compare_ResData(const RESdata* a, const RESdata* b, bool isDescending) {

	if (isDescending) {
		if (a->width == b->width)
			return a->height > b->height;
		else
			return a->width > b->width;
	}
	else {
		if (b->width == a->width)
			return b->height > a->height;
		else
			return b->width > a->width;
	}
}


//___________________________________
bool RESlist::Sort(bool isDescending) {

	if (!list)
		return false;

	RESdata* resTemp = nullptr;

	for (DWORD item = 0; item < numRes; item++) {

		for (DWORD itemJ = 0; itemJ < numRes - 1; itemJ++) {

			if (!compare_ResData(list[itemJ], list[itemJ + 1], isDescending)) {
				resTemp = list[itemJ + 1];
				list[itemJ + 1] = list[itemJ];
				list[itemJ] = resTemp;
			}
		}
	}

	return true;
}


//____________________
bool FREQlist::Empty() {

	if (list)
		free(list);
	list = nullptr;
	num = 0;
	return true;
}


//__________________________________
void FREQlist::push_back(DWORD freq) {

	DWORD* tmp = list;
	tmp = (DWORD*)realloc(tmp, (num + 1) * sizeof(DWORD));
	if (tmp) {
		list = tmp;
		list[num] = freq;
		num++;
	}
}

//___________________________________________________________
bool FREQlist::Fill(DWORD width, DWORD height, DWORD colours) {

	if (list)
		Empty();
	push_back(0);//first freq should be 0 for driver default

	DWORD newFrequency = 0;

	DEVMODE info;
	memset(&info, 0, sizeof(DEVMODE));
	info.dmSize = sizeof(DEVMODE);

	bool sameFreq = false;

	for (int m = 0; EnumDisplaySettings(nullptr, m, &info); m++) {
		if (info.dmPelsWidth != width || info.dmPelsHeight != height)//exclude other resolutions
			continue;

		sameFreq = false;

		for (DWORD item = 0; item < num; item++) {
			if (info.dmDisplayFrequency == list[item])//exclude same freq
				sameFreq = true;
		}

		if (!sameFreq) {//exclude same refresh rate at different colour bits
			info.dmBitsPerPel = colours;
			EnumDisplaySettings(nullptr, m, &info);// test if resolution works then print
			if (ChangeDisplaySettings(&info, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
				newFrequency = info.dmDisplayFrequency;
				push_back(newFrequency);
				newFrequency = 0;
			}
		}
	}

	if (!num)
		return false;

	Sort(false);
	return true;
}


//_______________________________________________________________
bool compareUint(const DWORD a, const DWORD b, bool isDescending) {

	if (isDescending)
		return a < b;
	else
		return b < a;
}


//____________________________________
bool FREQlist::Sort(bool isDescending) {

	if (!list)
		return false;

	DWORD freqTemp = 0;

	for (DWORD item = 0; item < num; item++) {

		for (DWORD itemJ = 0; itemJ < num - 1; itemJ++) {
			//if(list[itemJ] > list[itemJ+1]) {
			if (compareUint(list[itemJ], list[itemJ + 1], isDescending)) {
				freqTemp = list[itemJ + 1];
				list[itemJ + 1] = list[itemJ];
				list[itemJ] = freqTemp;
			}
		}
	}

	return true;
}
