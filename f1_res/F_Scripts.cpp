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


//____________________
void F_ScriptsMsgFree() {

	__asm {
		call SCRIPT_MSG_FREE
	}
}


//__________________
void FScriptsSetup() {

	F_GET_SCRIPT_ID = (void*)FixAddress(0x48A400);

	F_GET_SCRIPT_STRUCT = (void*)FixAddress(0x494478);

	SCRIPTS_SET_DUDE_SCRIPT = (void*)FixAddress(0x4935B8);

	SCRIPTS_CLEAR_DUDE_SCRIPT = (void*)FixAddress(0x49366C);

	SCRIPTS_INIT = (void*)FixAddress(0x4936D0);

	SCRIPTS_RESET = (void*)FixAddress(0x49373C);

	SCRIPTS_EXIT = (void*)FixAddress(0x4938A0);

	SCRIPTS_GAME_INIT = (void*)FixAddress(0x493754);

	SCRIPTS_GAME_RESET = (void*)FixAddress(0x493864);

	SCRIPTS_GAME_CLEAR = (void*)FixAddress(0x493964);

	SCRIPT_MSG_FREE = (void*)FixAddress(0x4938FC);
}
