/*
The MIT License (MIT)
Copyright � 2022 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the �Software�), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once


#include "F_Objects.h"

struct UDATA_SP {
       LONG build_tile; //= hexpos | ((level << 29) & 0xE0000000);
       LONG radius;
};
struct UDATA_TM {
       LONG time;
       LONG none;
};
union UDATA {
       UDATA_SP sp;
       UDATA_TM tm;
};

struct SCRIPT_STRUCT {
       DWORD id;                   //0x00
       SCRIPT_STRUCT *next;         //0x04
       UDATA udata;                 //0x08
       LONG flags;                 //0x10
       LONG index;                 //0x14
       DWORD unknown18;            //0x18
       DWORD objID;                //0x1C
       DWORD local_var_offset;     //0x20
       DWORD num_local_vars;       //0x24

       DWORD unknown28;             //0x28
       DWORD unknown2C;             //0x2C
       DWORD unknown30;             //0x30
       OBJStruct *pObj;              //0x34
       DWORD unknown38;             //0x38 //*obj interact with?
       DWORD unknown3C;             //0x3C //*obj interact with? pickup
       DWORD unknown40;             //0x40
       DWORD unknown44;             //0x44//isScenery?
};


LONG F_GetScriptID(OBJStruct *pObj, DWORD *pScriptID);
LONG F_GetScriptStruct(DWORD scriptID, SCRIPT_STRUCT **lpScr);


void F_ScriptsDudeSet();
void F_ScriptsDudeClear();

void F_ScriptsInit();
void F_ScriptsReset();
void F_ScriptsExit();

void F_ScriptsGameInit();
void F_ScriptsGameReset();
void F_ScriptsGameClear();

void F_ScriptsMsgFree();

void FScriptsSetup();
