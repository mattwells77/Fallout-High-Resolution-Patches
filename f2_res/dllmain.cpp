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

// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "memwrite.h"
#include "fixes.h"
#include "F_Windows.h"
#include "F_File.h"
#include "F_Art.h"
#include "F_Text.h"
#include "F_Msg.h"
#include "F_Objects.h"
#include "F_Mapper.h"
#include "F_Scripts.h"
#include "WinFall.h"

HINSTANCE phinstDLL;


//_______________
void Initialize() {

	int region = region_check();
	FArtSetup(region);
	FFileSetup(region);
	FTextSetup(region);
	FMsgSetup(region);
	FWindowsSetup(region);
	F_ObjectsSetup(region);
	FScriptsSetup(region);
	FMapperSetup(region);

	WinFallFixes(region);
	DirectDraw7_Fixes(region);
#if !defined (VERSION_LEGACY)
	DirectX9_Fixes(region);
#endif

	WinGeneralFixes(region);

	GameScrnFixes(region);
	MapFixes(region);

	SplashScrnFixes(region);
	CharScrnFixes(region);
	CreditsFixes(region);
	DialogInventoryFixes(region);
	OptionsFixes(region);
	EndSlidesFixes(region);
	WorldMapFixes(region);
	DeathScrnFixes(region);
	MovieFixes(region);
	LoadSaveFixes(region);
	MainMenuFixes(region);
	IfaceFixes(region);
	PipBoyFixes(region);

	HelpScrnFixes(region);
	PauseScrnFixes(region);
	HiResSettings(region);

	print_mem_errors();
}


//___________________________________
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved) {

	phinstDLL = hModule;

	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		Initialize();
		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
