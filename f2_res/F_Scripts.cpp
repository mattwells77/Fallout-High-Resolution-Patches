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
#include "F_Scripts.h"

void* SCRIPTS_SET_DUDE_SCRIPT = nullptr;
void* SCRIPTS_CLEAR_DUDE_SCRIPT = nullptr;

void* SCRIPTS_INIT = nullptr;
void* SCRIPTS_RESET = nullptr;
void* SCRIPTS_EXIT = nullptr;

void* SCRIPTS_GAME_INIT = nullptr;
void* SCRIPTS_GAME_RESET = nullptr;
void* SCRIPTS_GAME_CLEAR = nullptr;

void* SCRIPT_MSG_FREE = nullptr;

void* F_GET_SCRIPT_STRUCT = nullptr;
void* F_GET_SCRIPT_ID = nullptr;

void* F_GET_SCRIPT_PATH = nullptr;


//______________________________
void F_GetScriptPath(char* buff) {

	if (!buff)
		return;
	__asm {
		mov eax, buff
		call F_GET_SCRIPT_PATH
	}
	return;
}


//___________________________________________________
LONG F_GetScriptID(OBJStruct* pObj, DWORD* pScriptID) {

	LONG retVal = -1;
	__asm {
		mov edx, pScriptID
		mov eax, pObj
		call F_GET_SCRIPT_ID
		mov retVal, eax
	}
	return retVal;
}


//___________________________________________________________
LONG F_GetScriptStruct(DWORD scriptID, SCRIPT_STRUCT** lpScr) {

	LONG retVal = -1;
	__asm {
		mov edx, lpScr
		mov eax, scriptID
		call F_GET_SCRIPT_STRUCT
		mov retVal, eax
	}
	return retVal;
}


//_____________________
void F_ScriptsDudeSet() {

	__asm {
		call SCRIPTS_SET_DUDE_SCRIPT
	}
}


//_______________________
void F_ScriptsDudeClear() {

	__asm {
		call SCRIPTS_CLEAR_DUDE_SCRIPT
	}
}


//__________________
void F_ScriptsInit() {

	__asm {
		call SCRIPTS_INIT
	}
}


//__________________
void F_ScriptsReset() {

	__asm {
		call SCRIPTS_RESET
	}
}


//__________________
void F_ScriptsExit() {

	__asm {
		call SCRIPTS_EXIT
	}
}


//______________________
void F_ScriptsGameInit() {

	__asm {
		call SCRIPTS_GAME_INIT
	}
}


//_______________________
void F_ScriptsGameReset() {

	__asm {
		call SCRIPTS_GAME_RESET
	}
}


//_______________________
void F_ScriptsGameClear() {

	__asm {
		call SCRIPTS_GAME_CLEAR
	}
}


//_____________________
void F_ScriptsMsgFree() {

	__asm {
		call SCRIPT_MSG_FREE
	}
}


//______________________________
void FScriptsSetup(DWORD region) {

	if (region == 4) {
		F_GET_SCRIPT_ID = (void*)0x4996A0;

		F_GET_SCRIPT_STRUCT = (void*)0x4A4B34;

		SCRIPTS_SET_DUDE_SCRIPT = (void*)0x4A3C90;

		SCRIPTS_CLEAR_DUDE_SCRIPT = (void*)0x4A3D44;

		SCRIPTS_INIT = (void*)0x4A3DA8;

		SCRIPTS_RESET = (void*)0x4A3E20;

		SCRIPTS_EXIT = (void*)0x4A3F74;

		SCRIPTS_GAME_INIT = (void*)0x4A3E38;

		SCRIPTS_GAME_RESET = (void*)0x4A3F40;

		SCRIPTS_GAME_CLEAR = (void*)0x4A405C;

		SCRIPT_MSG_FREE = (void*)0x4A3FF4;
	}
	else {
		F_GET_SCRIPT_ID = (void*)FixAddress(0x49A9A0);

		F_GET_SCRIPT_STRUCT = (void*)FixAddress(0x4A5E34);

		SCRIPTS_SET_DUDE_SCRIPT = (void*)FixAddress(0x4A4F90);

		SCRIPTS_CLEAR_DUDE_SCRIPT = (void*)FixAddress(0x4A5044);

		SCRIPTS_INIT = (void*)FixAddress(0x4A50A8);

		SCRIPTS_RESET = (void*)FixAddress(0x4A5120);

		SCRIPTS_EXIT = (void*)FixAddress(0x4A5274);

		SCRIPTS_GAME_INIT = (void*)FixAddress(0x4A5138);

		SCRIPTS_GAME_RESET = (void*)FixAddress(0x4A5240);

		SCRIPTS_GAME_CLEAR = (void*)FixAddress(0x4A535C);

		SCRIPT_MSG_FREE = (void*)FixAddress(0x4A52F4);

		F_GET_SCRIPT_PATH = (void*)FixAddress(0x4A47BC);
	}
}
