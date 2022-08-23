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

#include "F_Msg.h"
#include "memwrite.h"

DWORD KILL_MSG_LST = 0;
DWORD SAVE_MESSAGE_LST = 0;
DWORD LOAD_MSG_LST = 0;

MSGList* pMsgList_Proto = nullptr;

//__________________________________
int DestroyMsgList(MSGList* MsgList) {

	int retVal = 0;
	__asm {
		mov eax, MsgList
		call KILL_MSG_LST
		mov retVal, eax
	}
	return retVal;
}


//________________________________________________________
int SaveMsgList(MSGList* MsgList, const char* MsgFilePath) {

	int retVal = 0;
	__asm {
		mov edx, MsgFilePath
		mov eax, MsgList
		call SAVE_MESSAGE_LST
		mov retVal, eax
	}
	return retVal;
}


//________________________________________________________
int LoadMsgList(MSGList* MsgList, const char* MsgFilePath) {

	int retVal = 0;
	__asm {
		mov edx, MsgFilePath
		mov eax, MsgList
		call LOAD_MSG_LST
		mov retVal, eax
	}
	return retVal;
}


//_________________________________________________
MSGNode* GetMsgNode(MSGList* MsgList, DWORD msgRef) {

	if (MsgList == nullptr)return nullptr;
	if (MsgList->numMsgs <= 0)return nullptr;

	MSGNode* MsgNode = (MSGNode*)MsgList->MsgNodes;

	long last = MsgList->numMsgs - 1;
	long first = 0;
	long mid;

	//Use Binary Search to find msg
	while (first <= last) {
		mid = (first + last) / 2;
		if (msgRef > MsgNode[mid].ref)
			first = mid + 1;
		else if (msgRef < MsgNode[mid].ref)
			last = mid - 1;
		else
			return &MsgNode[mid];
	}
	return nullptr;
}


//______________________________________________________
char* GetMsg(MSGList* MsgList, DWORD msgRef, int msgNum) {

	MSGNode* MsgNode = GetMsgNode(MsgList, msgRef);
	if (MsgNode) {
		if (msgNum == 2)
			return MsgNode->msg2;
		else if (msgNum == 1)
			return MsgNode->msg1;
	}
	return (char*)"Error";
}


//______________
void FMsgSetup() {

	KILL_MSG_LST = FixAddress(0x476B14);
	SAVE_MESSAGE_LST = FixAddress(0x476B84);
	LOAD_MSG_LST = FixAddress(0x476C54);

	pMsgList_Proto = (MSGList*)FixAddress(0x662D68);
}
