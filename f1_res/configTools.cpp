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

#include "configTools.h"
#include "F_File.h"


#include <Shlobj.h>
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib") 

char* pConfigPath = nullptr;
char* pSFallPath = nullptr;


typedef HRESULT(__stdcall* PFNSHGETFOLDERPATHA)(HWND, int, HANDLE, DWORD, LPSTR);  // "SHGetFolderPathA"
PFNSHGETFOLDERPATHA GetFuncPtr_SHGetFolderPathA()
{
	static HMODULE hMod = nullptr;
	PFNSHGETFOLDERPATHA pSHGetFolderPath = nullptr;

	// Load SHFolder.dll only once
	if (!hMod)
		hMod = LoadLibrary("Shell32.dll");

	if (hMod)
		// Obtain a pointer to the SHGetFolderPathA function
		pSHGetFolderPath = (PFNSHGETFOLDERPATHA)GetProcAddress(hMod,
			"SHGetFolderPathA");

	return pSHGetFolderPath;
}


typedef BOOL(WINAPI* PFNGETVERSIONEX)(LPOSVERSIONINFO);
PFNGETVERSIONEX GetFuncPtr_GetVersionEx() {
	static HMODULE hMod = nullptr;
	PFNGETVERSIONEX pGetVersionEx = nullptr;

	// Load SHFolder.dll only once
	if (!hMod)
		hMod = LoadLibrary("Kernel32.dll");

	if (hMod)
		// Obtain a pointer to the SHGetFolderPathA function
		pGetVersionEx = (PFNGETVERSIONEX)GetProcAddress(hMod,
			"GetVersionExA");

	return pGetVersionEx;
}


//______________________
BOOL IsWinVistaOrLater() {

	OSVERSIONINFO osVerInfo;
	ZeroMemory(&osVerInfo, sizeof(OSVERSIONINFO));
	osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);


	PFNGETVERSIONEX pGetVersionEx = GetFuncPtr_GetVersionEx();
	if (pGetVersionEx)
		pGetVersionEx(&osVerInfo);
	else
		return false;

	if (osVerInfo.dwMajorVersion >= 6)
		return true;
	else
		return false;
}



// ______________________________________________________
DWORD ConfigPath_GetCurrent(char** ppReturnPath) {
	if (!ppReturnPath)
		return 0;
	DWORD buffSize = GetCurrentDirectory(0, nullptr);
	buffSize += 12;
	char* pReturnPath = new char[buffSize];
	ZeroMemory(pReturnPath, buffSize);

	if (!GetCurrentDirectory(buffSize, pReturnPath)) {
		delete pReturnPath;
		pReturnPath = nullptr;
		*ppReturnPath = nullptr;
		return 0;
	}
	strncat(pReturnPath, "\\f1_res.ini", 11);
	*ppReturnPath = pReturnPath;
	return buffSize;
}


//_________________________________
void ConfigCreate(char* configPath) {

	char* pathCurrent = nullptr;

	if (!configPath) {
		DWORD buffSize = GetCurrentDirectory(0, nullptr);
		pathCurrent = new char[buffSize];
		ZeroMemory(pathCurrent, buffSize);
		GetCurrentDirectory(buffSize, pathCurrent);
		ConfigWriteString("LOCATION", "path", pathCurrent);//insert current path for visual identification.
		delete[] pathCurrent;
		pathCurrent = nullptr;
		ConfigPath_GetCurrent(&pathCurrent);//get path of ini in current folder to read from.
		configPath = pathCurrent;
	}
	else if (GetFileAttributes(configPath) == INVALID_FILE_ATTRIBUTES) {
		MessageBox(nullptr, configPath, "Hi-Res patch Config - Invalid Path", MB_ICONERROR);
		return;
	}

	int intVal = 0;
	const DWORD maxStringLength = 256;
	char stringVal[maxStringLength];

	intVal = GetPrivateProfileInt("MAIN", "GRAPHICS_MODE", 2, configPath);
	ConfigWriteInt("MAIN", "GRAPHICS_MODE", intVal);

	intVal = GetPrivateProfileInt("MAIN", "SCALE_2X", 0, configPath);
	ConfigWriteInt("MAIN", "SCALE_2X", intVal);

	intVal = GetPrivateProfileInt("MAIN", "SCR_WIDTH", 1024, configPath);
	ConfigWriteInt("MAIN", "SCR_WIDTH", intVal);

	intVal = GetPrivateProfileInt("MAIN", "SCR_HEIGHT", 768, configPath);
	ConfigWriteInt("MAIN", "SCR_HEIGHT", intVal);

	intVal = GetPrivateProfileInt("MAIN", "COLOUR_BITS", 32, configPath);
	ConfigWriteInt("MAIN", "COLOUR_BITS", intVal);

	intVal = GetPrivateProfileInt("MAIN", "REFRESH_RATE", 0, configPath);
	ConfigWriteInt("MAIN", "REFRESH_RATE", intVal);

	intVal = GetPrivateProfileInt("MAIN", "WINDOWED", 1, configPath);
	ConfigWriteInt("MAIN", "WINDOWED", intVal);

	intVal = GetPrivateProfileInt("MAIN", "WINDOWED_FULLSCREEN", 0, configPath);
	ConfigWriteInt("MAIN", "WINDOWED_FULLSCREEN", intVal);

	WINDOWPLACEMENT winData;
	winData.length = sizeof(WINDOWPLACEMENT);
	if (GetPrivateProfileStruct("MAIN", "WIN_DATA", &winData, winData.length, configPath))
		ConfigWriteWinData("MAIN", "WIN_DATA", &winData);
	else
		ConfigWriteInt("MAIN", "WIN_DATA", 0);

	///GetPrivateProfileString("MAIN", "f2_res_dat", "f2_res.dat", stringVal, maxStringLength, configPath);
	///ConfigWriteString("MAIN", "f2_res_dat", stringVal);

	///GetPrivateProfileString("MAIN", "f2_res_patches", "data\\", stringVal, maxStringLength, configPath);
	///ConfigWriteString("MAIN", "f2_res_patches", stringVal);


	intVal = ConfigReadInt("MAIN", "ALT_MOUSE_INPUT", -1);//check if ALT_MOUSE_INPUT is in MAIN. Now located in INPUT.
	if (intVal != -1) {
		//WritePrivateProfileString ("MAIN", "ALT_MOUSE_INPUT", nullptr, configPath);
		ConfigWriteString("MAIN", "ALT_MOUSE_INPUT", nullptr);//remove ALT_MOUSE_INPUT from main if it exists
		ConfigWriteInt("INPUT", "ALT_MOUSE_INPUT", intVal);//copy to INPUT section
	}
	else {
		intVal = GetPrivateProfileInt("INPUT", "ALT_MOUSE_INPUT", 0, configPath);
		ConfigWriteInt("INPUT", "ALT_MOUSE_INPUT", intVal);
	}

	intVal = GetPrivateProfileInt("INPUT", "EXTRA_WIN_MSG_CHECKS", 1, configPath);
	ConfigWriteInt("INPUT", "EXTRA_WIN_MSG_CHECKS", intVal);

	intVal = GetPrivateProfileInt("INPUT", "SCROLLWHEEL_FOCUS_PRIMARY_MENU", 1, configPath);
	ConfigWriteInt("INPUT", "SCROLLWHEEL_FOCUS_PRIMARY_MENU", intVal);

	intVal = GetPrivateProfileInt("EFFECTS", "IS_GRAY_SCALE", 0, configPath);
	ConfigWriteInt("EFFECTS", "IS_GRAY_SCALE", intVal);

	intVal = GetPrivateProfileInt("HI_RES_PANEL", "DISPLAY_LIST_DESCENDING", 1, configPath);
	ConfigWriteInt("HI_RES_PANEL", "DISPLAY_LIST_DESCENDING", intVal);

	intVal = GetPrivateProfileInt("MOVIES", "MOVIE_SIZE", 1, configPath);
	ConfigWriteInt("MOVIES", "MOVIE_SIZE", intVal);

	intVal = GetPrivateProfileInt("MAPS", "EDGE_CLIPPING_ON", 1, configPath);
	ConfigWriteInt("MAPS", "EDGE_CLIPPING_ON", intVal);

	intVal = GetPrivateProfileInt("MAPS", "IGNORE_MAP_EDGES", 0, configPath);
	ConfigWriteInt("MAPS", "IGNORE_MAP_EDGES", intVal);

	intVal = GetPrivateProfileInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", 1, configPath);
	ConfigWriteInt("MAPS", "IGNORE_PLAYER_SCROLL_LIMITS", intVal);

	GetPrivateProfileString("MAPS", "SCROLL_DIST_X", "HALF_SCRN", stringVal, maxStringLength, configPath);
	ConfigWriteString("MAPS", "SCROLL_DIST_X", stringVal);

	GetPrivateProfileString("MAPS", "SCROLL_DIST_Y", "HALF_SCRN", stringVal, maxStringLength, configPath);
	ConfigWriteString("MAPS", "SCROLL_DIST_Y", stringVal);

	intVal = GetPrivateProfileInt("MAPS", "NumPathNodes", 1, configPath);
	ConfigWriteInt("MAPS", "NumPathNodes", intVal);

	intVal = GetPrivateProfileInt("MAPS", "FOG_OF_WAR", 0, configPath);
	ConfigWriteInt("MAPS", "FOG_OF_WAR", intVal);

	intVal = GetPrivateProfileInt("MAPS", "FOG_LIGHT_LEVEL", 4, configPath);
	ConfigWriteInt("MAPS", "FOG_LIGHT_LEVEL", intVal);

	intVal = GetPrivateProfileInt("MAPS", "ZOOM_LEVEL", 1, configPath);
	ConfigWriteInt("MAPS", "ZOOM_LEVEL", intVal);

	intVal = GetPrivateProfileInt("MAPS", "IS_ZOOM_BOUND_BY_EDGES", 0, configPath);
	ConfigWriteInt("MAPS", "IS_ZOOM_BOUND_BY_EDGES", intVal);

	intVal = GetPrivateProfileInt("IFACE", "IFACE_BAR_MODE", 0, configPath);
	ConfigWriteInt("IFACE", "IFACE_BAR_MODE", intVal);

	intVal = GetPrivateProfileInt("IFACE", "IFACE_BAR_SIDE_ART", 1, configPath);
	ConfigWriteInt("IFACE", "IFACE_BAR_SIDE_ART", intVal);

	intVal = GetPrivateProfileInt("IFACE", "IFACE_BAR_SIDES_ORI", 0, configPath);
	ConfigWriteInt("IFACE", "IFACE_BAR_SIDES_ORI", intVal);

	intVal = GetPrivateProfileInt("IFACE", "IFACE_BAR_WIDTH", 800, configPath);
	ConfigWriteInt("IFACE", "IFACE_BAR_WIDTH", intVal);

	intVal = GetPrivateProfileInt("IFACE", "ALTERNATE_AMMO_METRE", 2, configPath);
	ConfigWriteInt("IFACE", "ALTERNATE_AMMO_METRE", intVal);

	intVal = GetPrivateProfileInt("IFACE", "ALTERNATE_AMMO_LIGHT", 0xC4, configPath);
	ConfigWriteInt("IFACE", "ALTERNATE_AMMO_LIGHT", intVal);

	intVal = GetPrivateProfileInt("IFACE", "ALTERNATE_AMMO_DARK", 0x4B, configPath);
	ConfigWriteInt("IFACE", "ALTERNATE_AMMO_DARK", intVal);

	intVal = GetPrivateProfileInt("MAINMENU", "MAIN_MENU_SIZE", 0, configPath);
	ConfigWriteInt("MAINMENU", "MAIN_MENU_SIZE", intVal);

	intVal = GetPrivateProfileInt("MAINMENU", "USE_HIRES_IMAGES", 1, configPath);
	ConfigWriteInt("MAINMENU", "USE_HIRES_IMAGES", intVal);

	intVal = GetPrivateProfileInt("MAINMENU", "SCALE_BUTTONS_AND_TEXT_MENU", 0, configPath);
	ConfigWriteInt("MAINMENU", "SCALE_BUTTONS_AND_TEXT_MENU", intVal);

	intVal = GetPrivateProfileInt("MAINMENU", "MENU_BG_OFFSET_X", -24, configPath);
	ConfigWriteInt("MAINMENU", "MENU_BG_OFFSET_X", intVal);

	intVal = GetPrivateProfileInt("MAINMENU", "MENU_BG_OFFSET_Y", -24, configPath);
	ConfigWriteInt("MAINMENU", "MENU_BG_OFFSET_Y", intVal);

	intVal = GetPrivateProfileInt("STATIC_SCREENS", "DEATH_SCRN_SIZE", 1, configPath);
	ConfigWriteInt("STATIC_SCREENS", "DEATH_SCRN_SIZE", intVal);

	intVal = GetPrivateProfileInt("STATIC_SCREENS", "END_SLIDE_SIZE", 1, configPath);
	ConfigWriteInt("STATIC_SCREENS", "END_SLIDE_SIZE", intVal);

	intVal = GetPrivateProfileInt("STATIC_SCREENS", "HELP_SCRN_SIZE", 1, configPath);
	ConfigWriteInt("STATIC_SCREENS", "HELP_SCRN_SIZE", intVal);

	intVal = GetPrivateProfileInt("STATIC_SCREENS", "SPLASH_SCRN_SIZE", 1, configPath);
	ConfigWriteInt("STATIC_SCREENS", "SPLASH_SCRN_SIZE", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "DIALOG_SCRN_BACKGROUND", 0, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "DIALOG_SCRN_BACKGROUND", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "DIALOG_SCRN_ART_FIX", 1, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "DIALOG_SCRN_ART_FIX", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "SPLASH_SCRN_TIME", 0, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "SPLASH_SCRN_TIME", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "DOUBLE_CLICK_RUNNING", 1, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "DOUBLE_CLICK_RUNNING", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "INV_ADD_ITEMS_AT_TOP", 0, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "INV_ADD_ITEMS_AT_TOP", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "CPU_USAGE_FIX", 1, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "CPU_USAGE_FIX", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "BARTER_PC_INV_DROP_FIX", 1, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "BARTER_PC_INV_DROP_FIX", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "FADE_TIME_MODIFIER", 60, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "FADE_TIME_MODIFIER", intVal);

	intVal = GetPrivateProfileInt("OTHER_SETTINGS", "FADE_TIME_RECALCULATE_ON_FADE", 0, configPath);
	ConfigWriteInt("OTHER_SETTINGS", "FADE_TIME_RECALCULATE_ON_FADE", intVal);

	if (pathCurrent)
		delete[] pathCurrent;
}



//____________________
void ConfigPathSetup() {
	if (pConfigPath != nullptr && pConfigPath[0] != '\0')return;

	if (pConfigPath != nullptr) {
		delete pConfigPath;
		pConfigPath = nullptr;
	}

	DWORD pathSize = ConfigPath_GetCurrent(&pConfigPath);

	LONG UAC_AWARE = ConfigReadInt("MAIN", "UAC_AWARE", 0);

	if (UAC_AWARE && IsWinVistaOrLater()) {

		char AppDatPath[MAX_PATH];
		PFNSHGETFOLDERPATHA pSHGetFolderPath = GetFuncPtr_SHGetFolderPathA();
		if (pSHGetFolderPath)
			pSHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, AppDatPath);


		DWORD appDatPathSize = 0;
		while (AppDatPath[appDatPathSize] != '\0') {
			appDatPathSize++;
		}

		DWORD currentDirSize = GetCurrentDirectory(0, nullptr);
		char* dirCurrent = new char[currentDirSize];//[buffSize];
		ZeroMemory(dirCurrent, currentDirSize);
		GetCurrentDirectory(currentDirSize, dirCurrent);
		int i = 0;

		while (dirCurrent[i] != '\0') {
			if (dirCurrent[i] == ':' || dirCurrent[i] == '\\')
				dirCurrent[i] = '_';
			i++;
		}

		BYTE bHash[16];
		HashData((BYTE*)dirCurrent, currentDirSize, bHash, 16);
		char bHashString[33];

		for (int i = 0; i < 16; ++i) {
			sprintf(&bHashString[i * 2], "%02x", bHash[i]);
		}

		if (pConfigPath != nullptr) {
			delete[] pConfigPath;
			pConfigPath = nullptr;
		}

		/// pathSize = 4 + appDatPathSize + 9 + currentDirSize + 12 +8;
		//uniPrependSize=4, FalloutFolderSize=8 BackSlashSize = 1, hexFolderSize=33, iniSize = 12,
		pathSize = 4 + appDatPathSize + 8 + 1 + 33 + 12;
		pConfigPath = new char[pathSize];
		ZeroMemory(pConfigPath, pathSize);
		strncat(pConfigPath, "\\\\?\\", appDatPathSize);//uniPrependSize
		strncat(pConfigPath, AppDatPath, appDatPathSize);//appDatPathSize
		strncat(pConfigPath, "\\Fallout", 8);

		if (GetFileAttributes(pConfigPath) == INVALID_FILE_ATTRIBUTES) {
			if (!CreateDirectory(pConfigPath, nullptr)) {
				delete[] pConfigPath;
				pConfigPath = nullptr;
				ConfigPath_GetCurrent(&pConfigPath);
				return;
			}
		}

		strncat(pConfigPath, "\\", 1);//BackSlashSize
		strncat(pConfigPath, bHashString, 33);//hexFolderSize
		delete[] dirCurrent;
		if (GetFileAttributes(pConfigPath) == INVALID_FILE_ATTRIBUTES)
			CreateDirectory(pConfigPath, nullptr);

		strncat(pConfigPath, "\\f1_res.ini", 11);
	}

	if (GetFileAttributes(pConfigPath) == INVALID_FILE_ATTRIBUTES)
		ConfigCreate(nullptr);
	else
		ConfigCreate(pConfigPath);
}


//_____________________
void ConfigRefreshCache() {
	ConfigPathSetup();
	WritePrivateProfileString(nullptr, nullptr, nullptr, pConfigPath);
}


//____________________
void SFallPathSetup() {
	if (pSFallPath)return;

	pSFallPath = new char[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pSFallPath);
	strncat(pSFallPath, "\\ddraw.ini", 12);
}


//______________________________________________________________________________________________________________________________
DWORD ConfigReadString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, DWORD nSize) {
	ConfigPathSetup();
	return GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, pConfigPath);
}


//________________________________________________________________________________________
BOOL ConfigWriteString(const char* lpAppName, const char* lpKeyName, const char* lpString) {
	ConfigPathSetup();
	return WritePrivateProfileString(lpAppName, lpKeyName, lpString, pConfigPath);
}


//____________________________________________________________________________
UINT ConfigReadInt(const char* lpAppName, const char* lpKeyName, int nDefault) {
	ConfigPathSetup();
	return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, pConfigPath);
}


//___________________________________________________________________________
BOOL ConfigWriteInt(const char* lpAppName, const char* lpKeyName, int intVal) {
	ConfigPathSetup();
	char lpString[64];
	sprintf(lpString, "%d", intVal);
	return WritePrivateProfileString(lpAppName, lpKeyName, lpString, pConfigPath);
}


//_____________________________________________________________________________________________
BOOL ConfigReadWinData(const char* lpAppName, const char* lpKeyName, WINDOWPLACEMENT* pWinData) {
	ConfigPathSetup();
	pWinData->length = sizeof(WINDOWPLACEMENT);
	return GetPrivateProfileStruct(lpAppName, lpKeyName, pWinData, sizeof(WINDOWPLACEMENT), pConfigPath);
}

/*
//______________________________________________________________________________________________
BOOL ConfigWriteWinData(const char *lpAppName, const char *lpKeyName, WINDOWPLACEMENT *pWinData) {
   ConfigPathSetup();
   pWinData->length = sizeof(WINDOWPLACEMENT);
   return WritePrivateProfileStruct(lpAppName, lpKeyName, pWinData, sizeof(WINDOWPLACEMENT), pConfigPath);
}
*/

//______________________________________________________________________________________________
BOOL ConfigWriteWinData(const char* lpAppName, const char* lpKeyName, WINDOWPLACEMENT* pWinData) {
	ConfigPathSetup();
	WritePrivateProfileStruct(lpAppName, lpKeyName, pWinData, sizeof(WINDOWPLACEMENT), pConfigPath);
	return WritePrivateProfileStruct(nullptr, nullptr, nullptr, 0, pConfigPath);
}

//______________________________________________________________________________________________
BOOL ConfigReadStruct(LPCTSTR lpszSection, LPCTSTR lpszKey, LPVOID lpStruct, UINT uSizeStruct) {
	ConfigPathSetup();
	return GetPrivateProfileStruct(lpszSection, lpszKey, lpStruct, uSizeStruct, pConfigPath);
}


//_______________________________________________________________________________________________
BOOL ConfigWriteStruct(LPCTSTR lpszSection, LPCTSTR lpszKey, LPVOID lpStruct, UINT uSizeStruct) {
	ConfigPathSetup();
	WritePrivateProfileStruct(lpszSection, lpszKey, lpStruct, uSizeStruct, pConfigPath);
	return WritePrivateProfileStruct(nullptr, nullptr, nullptr, 0, pConfigPath);
}


//___________________________________________________________________________
UINT SfallReadInt(const char* lpAppName, const char* lpKeyName, int nDefault) {
	SFallPathSetup();
	return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, pSFallPath);
}


//__________________________________________________________________________
BOOL SfallWriteInt(const char* lpAppName, const char* lpKeyName, int intVal) {
	SFallPathSetup();
	char lpString[64];
	sprintf(lpString, "%d", intVal);
	return WritePrivateProfileString(lpAppName, lpKeyName, lpString, pSFallPath);
}
