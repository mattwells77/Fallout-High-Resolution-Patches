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

#include "WinFall.h"
#include "Graphics.h"

#include "fixes.h"
#include "memwrite.h"
#include "F_Windows.h"
#include "configTools.h"

#include "Dx9.h"

#include <ddraw.h>

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "shaderPal.h"

LPDIRECT3D9 lpD3D = nullptr;
LPDIRECT3DDEVICE9 lpD3DDevice = nullptr;
LPDIRECT3DTEXTURE9 lpD3DTexturePal = nullptr;
LPDIRECT3DTEXTURE9 lpD3DTexturePalBack = nullptr;

LPDIRECT3D9* plpD3D = nullptr;
LPDIRECT3DTEXTURE9* pm_D3DTexture = nullptr;
LPDIRECT3DTEXTURE9* pm_D3DTexture2 = nullptr;

void** plpDDPalette = nullptr;

struct SCRNVERTEX {
	FLOAT x, y, z;
	FLOAT tu, tv;
};

#define D3DFVF_SCRNVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

bool isSoftwareVertex = false;
bool isShader2_0 = true;

static ID3DXEffect* pPalEffect = 0;

bool dxPresentFlag = false;
DWORD Dx9PresentSkip = 0;
LONG* drawWindowsFlag = nullptr;

void* F_DRAW_GAME_SCENE = nullptr;
void* F_DRAW_GAME_SCENE_SCROLL = nullptr;


//___________________________________
void initPalShader(D3DXMATRIX* world) {

	if (!isShader2_0)
		return;
	static D3DXHANDLE hPalShaderPal = 0;
	static D3DXHANDLE hPalShaderWorld = 0;

	if (pPalEffect) {
		pPalEffect->OnResetDevice();
		pPalEffect->SetMatrix(hPalShaderWorld, world);
		pPalEffect->SetTexture(hPalShaderPal, lpD3DTexturePal);
		return;
	}

	LPD3DXBUFFER errors = nullptr;
	if (D3DXCreateEffect(lpD3DDevice, pshaderPal_mem, sizeof(pshaderPal_mem), 0, 0, 0, 0, &pPalEffect, &errors) == D3D_OK) {
		hPalShaderWorld = pPalEffect->GetParameterByName(0, "WorldViewProj");
		hPalShaderPal = pPalEffect->GetParameterByName(0, "PaletteTexture");
		pPalEffect->SetMatrix(hPalShaderWorld, world);
		pPalEffect->SetTexture(hPalShaderPal, lpD3DTexturePal);
	}
	else {
		if (errors)
			MessageBox(nullptr, (char*)errors->GetBufferPointer(), "Hi-Res patch Error", MB_ICONERROR);
		else MessageBox(nullptr, "Failed to load palShader,\nDisabling shader effects.", "Hi-Res patch Error", MB_ICONERROR);
		isShader2_0 = false;
	}
	return;
}



class DX9win {

	float x;
	float y;
	float z;
	DWORD width;
	DWORD height;
	DWORD tilesW;
	DWORD tilesH;
	DWORD tilesTotal;
	bool* tileUpdated;

	D3DXMATRIX mxManipulation;
	D3DXMATRIX mxWorld;

	LPDIRECT3DDEVICE9 lpWinDev;
	LPDIRECT3DVERTEXBUFFER9 lpWinVB;
	LPDIRECT3DTEXTURE9* plpWinTex;
	LPDIRECT3DTEXTURE9* plpWinTexBack;
public:
	DX9win(LPDIRECT3DDEVICE9 inlpD3DDev, float inX, float inY, DWORD inWidth, DWORD inHeight);
	~DX9win() {
		if (tileUpdated)
			delete[]tileUpdated;

		if (!lpWinDev)
			return;
		if (lpWinVB != nullptr) {
			lpWinVB->Release();
			lpWinVB = nullptr;
		}
		DWORD t = 0;
		if (plpWinTex) {
			for (t = 0; t < tilesTotal; t++) {
				if (plpWinTex[t]) {
					plpWinTex[t]->Release();
					plpWinTex[t] = nullptr;
				}
			}
			delete[] plpWinTex;
			plpWinTex = nullptr;
		}
		if (plpWinTexBack) {
			for (t = 0; t < tilesTotal; t++) {
				if (plpWinTexBack[t]) {
					plpWinTexBack[t]->Release();
					plpWinTexBack[t] = nullptr;
				}
			}
			delete[] plpWinTexBack;
			plpWinTexBack = nullptr;
		}
		lpWinDev = nullptr;
	}
	bool SetWinVB() {
		if (lpWinDev->SetFVF(D3DFVF_SCRNVERTEX) != D3D_OK)
			return false;
		if (lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX)) != D3D_OK)
			return false;
		return true;
	}
	bool CreateWinVB();
	void DrawTile8(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth);
	void DrawTile32(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth);
	void Draw(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, LONG tX, LONG tY);
	void UpdateScene();
	void SetPosition(float inX, float inY) { x = inX; y = inY; };
};

DX9win* dx9Gui = nullptr;


DX9win::DX9win(LPDIRECT3DDEVICE9 inlpD3DDev, float inX, float inY, DWORD inWidth, DWORD inHeight) {

	if (!inlpD3DDev) {
		x = 0;
		y = 0;
		z = 0;
		width = 0;
		height = 0;
		tilesW = 0;
		tilesH = 0;
		tilesTotal = 0;
		tileUpdated = nullptr;
		lpWinDev = nullptr;
		lpWinVB = nullptr;
		plpWinTex = nullptr;
		plpWinTexBack = nullptr;
		return;
	}

	lpWinDev = inlpD3DDev;
	x = inX;
	y = inY;
	z = 0;
	width = inWidth;
	height = inHeight;
	tilesW = width >> 8;
	if ((tilesW << 8) < width)
		tilesW++;
	tilesH = height >> 8;
	if ((tilesH << 8) < height)
		tilesH++;
	tilesTotal = tilesW * tilesH;

	tileUpdated = new bool[tilesTotal];
	memset(tileUpdated, 0, sizeof(tileUpdated));

	if (!CreateWinVB()) {
		MessageBox(nullptr, "CreateWinVB failed", "Hi-Res patch Error", MB_ICONERROR);
		return;
	}
	plpWinTex = new LPDIRECT3DTEXTURE9[tilesTotal];
	plpWinTexBack = new LPDIRECT3DTEXTURE9[tilesTotal];

	D3DFORMAT colour = D3DFMT_X8R8G8B8;
	if (isShader2_0)
		colour = D3DFMT_L8;

	for (DWORD t = 0; t < tilesTotal; t++) {
		lpWinDev->CreateTexture(256, 256, 1, 0, colour, D3DPOOL_DEFAULT, &plpWinTex[t], nullptr);//!=D3D_OK)
		//MessageBox( nullptr, "CreateTexture failed", "Hi-Res patch Error", MB_ICONERROR);
		lpWinDev->CreateTexture(256, 256, 1, 0, colour, D3DPOOL_SYSTEMMEM, &plpWinTexBack[t], nullptr);//!=D3D_OK)
		//MessageBox( nullptr, "CreateTexture2 failed", "Hi-Res patch Error", MB_ICONERROR);
	}
	lpWinDev->SetFVF(D3DFVF_SCRNVERTEX);
	lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX));
}


//________________________
bool DX9win::CreateWinVB() {

	int numVX = tilesW * 4;
	int numV = numVX * tilesH;

	DWORD d3dUsageFlags = D3DUSAGE_WRITEONLY;
	if (isSoftwareVertex)
		d3dUsageFlags = d3dUsageFlags | D3DUSAGE_SOFTWAREPROCESSING;

	HRESULT hresult = 0;
	if (hresult = lpWinDev->CreateVertexBuffer(numV * sizeof(SCRNVERTEX), d3dUsageFlags, D3DFVF_SCRNVERTEX, D3DPOOL_MANAGED, &lpWinVB, nullptr) != D3D_OK) {
		if (hresult == D3DERR_OUTOFVIDEOMEMORY)
			MessageBox(nullptr, "D3DERR_OUTOFVIDEOMEMORY", "Hi-Res patch Error", MB_ICONERROR);
		else if (hresult == D3DERR_INVALIDCALL)
			MessageBox(nullptr, "D3DERR_INVALIDCALL", "Hi-Res patch Error", MB_ICONERROR);
		else if (hresult == E_OUTOFMEMORY)
			MessageBox(nullptr, "E_OUTOFMEMORY", "Hi-Res patch Error", MB_ICONERROR);

		lpWinVB = nullptr;
		return false;
	}
	SCRNVERTEX* pVertices = nullptr;
	if (lpWinVB->Lock(0, numV * sizeof(SCRNVERTEX), (void**)&pVertices, 0) != D3D_OK) {
		MessageBox(nullptr, "Can't lock Win Quad vertices", "Hi-Res patch Error", MB_ICONERROR);
		return false;
	}

	float left = -0.5f;
	float top = -0.5f;
	float right = left + 256.0f;
	float bottom = top + 256.0f;

	for (int gridY = 0; gridY < numV; gridY += numVX) {
		for (int gridX = 0; gridX < numVX; gridX += 4) {
			pVertices[gridY + gridX + 0].x = left;
			pVertices[gridY + gridX + 0].y = top;
			pVertices[gridY + gridX + 0].z = 0.1f;
			pVertices[gridY + gridX + 0].tu = 0.0f;
			pVertices[gridY + gridX + 0].tv = 0.0f;

			pVertices[gridY + gridX + 1].x = right;
			pVertices[gridY + gridX + 1].y = top;
			pVertices[gridY + gridX + 1].z = 0.1f;
			pVertices[gridY + gridX + 1].tu = 1.0f;
			pVertices[gridY + gridX + 1].tv = 0.0f;

			pVertices[gridY + gridX + 2].x = right;
			pVertices[gridY + gridX + 2].y = bottom;
			pVertices[gridY + gridX + 2].z = 0.1f;
			pVertices[gridY + gridX + 2].tu = 1.0f;
			pVertices[gridY + gridX + 2].tv = 1.0f;

			pVertices[gridY + gridX + 3].x = left;
			pVertices[gridY + gridX + 3].y = bottom;
			pVertices[gridY + gridX + 3].z = 0.1f;
			pVertices[gridY + gridX + 3].tu = 0.0f;
			pVertices[gridY + gridX + 3].tv = 1.0f;

			left += 256.0f;
			right = left + 256.0f;
		}
		left = -0.5f;
		right = left + 256.0f;
		top += 256.0f;
		bottom = top + 256.0f;
	}
	lpWinVB->Unlock();
	return true;
}



//_______________________________________________________________________________________________________________________________________________________________
void DX9win::DrawTile8(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth) {

	toBuff.b += tY * toWidth + tX;
	fBuff += fY * fWidth + fX;

	for (DWORD y = 0; y < subHeight; y++) {
		for (DWORD x = 0; x < subWidth; x++)
			toBuff.b[x] = fBuff[x];
		toBuff.b += toWidth;
		fBuff += fWidth;
	}
}


//________________________________________________________________________________________________________________________________________________________________
void DX9win::DrawTile32(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth) {

	toWidth = toWidth >> 2;
	toBuff.d += tY * toWidth + tX;
	fBuff += fY * fWidth + fX;

	D3DLOCKED_RECT primlRect;
	if (lpD3DTexturePal->LockRect(0, &primlRect, nullptr, D3DLOCK_READONLY) != D3D_OK)
		return;

	BUFF pbuff{nullptr};
	pbuff.b = (BYTE*)primlRect.pBits;

	for (DWORD y = 0; y < subHeight; y++) {
		for (DWORD x = 0; x < subWidth; x++)
			toBuff.d[x] = pbuff.d[fBuff[x]];
		toBuff.d += toWidth;
		fBuff += fWidth;
	}
	lpD3DTexturePal->UnlockRect(0);
}


//______________________________________________________________________________________________________________________________
void DX9win::Draw(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, LONG tX, LONG tY) {

	if (!*is_winActive || isMapperSizing)
		return;

	if (lpWinDev == nullptr)
		return;
	if (plpWinTex == nullptr)
		return;
	if (plpWinTexBack == nullptr)
		return;

	RECT rcMain = { tX, tY, tX + (LONG)subWidth, tY + (LONG)subHeight };

	RECT rcTile{0,0,0,0};
	URECT32 rcFrom = { (DWORD)fX, (DWORD)fY, subWidth, subHeight };
	URECT32 rcGrid = { (tX & 0xFFFFFF00) >> 8, (tY & 0xFFFFFF00) >> 8, (((tX + subWidth) & 0xFFFFFF00) + 256) >> 8, (((tY + subHeight) & 0xFFFFFF00) + 256) >> 8 };

	int xPos = 0;
	int yPos = 0;

	lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX));

	DWORD tileYOff = rcGrid.top * tilesW;
	DWORD tileNum = 0;
	for (DWORD yGrid = rcGrid.top; yGrid < rcGrid.bottom; yGrid++) {
		yPos = yGrid << 8;
		rcTile.top = rcMain.top - yPos;
		rcTile.bottom = rcTile.top + subHeight;
		rcFrom.top = fY;
		rcFrom.bottom = subHeight;

		if (rcTile.top < 0) {
			rcFrom.top -= rcTile.top;//+ a neg
			rcFrom.bottom += rcTile.top;//- a neg
			rcTile.top = 0;
			rcTile.bottom = rcFrom.bottom;
		}

		if (rcTile.bottom > 256) {
			rcFrom.bottom -= (rcTile.bottom - 256);
			rcTile.bottom = 256;
		}

		for (DWORD xGrid = rcGrid.left; xGrid < rcGrid.right; xGrid++) {
			xPos = xGrid << 8;
			rcTile.left = rcMain.left - xPos;
			rcFrom.left = fX;
			rcTile.right = rcTile.left + subWidth;
			rcFrom.right = subWidth;

			if (rcTile.left < 0) {
				rcFrom.left -= rcTile.left;//+ a neg
				rcFrom.right += rcTile.left;//- a neg
				rcTile.left = 0;
				rcTile.right = rcFrom.right;
			}

			if (rcTile.right > 256) {
				rcFrom.right -= (rcTile.right - 256);
				rcTile.right = 256;
			}

			if (xGrid >= tilesW)
				continue;
			tileNum = xGrid + tileYOff;
			if (tileNum >= tilesTotal)
				continue;
			D3DLOCKED_RECT primlRect;
			if (plpWinTexBack[tileNum]->LockRect(0, &primlRect, &rcTile, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK)
				continue;

			BUFF tbuff{nullptr};
			tbuff.b = (BYTE*)primlRect.pBits;
			if (isShader2_0)
				DrawTile8(fBuff, fWidth, fHeight, rcFrom.left, rcFrom.top, rcFrom.right, rcFrom.bottom, tbuff, 0, 0, primlRect.Pitch);
			else
				DrawTile32(fBuff, fWidth, fHeight, rcFrom.left, rcFrom.top, rcFrom.right, rcFrom.bottom, tbuff, 0, 0, primlRect.Pitch);

			plpWinTexBack[tileNum]->AddDirtyRect(&rcTile);
			plpWinTexBack[tileNum]->UnlockRect(0);
			lpWinDev->UpdateTexture(plpWinTexBack[tileNum], plpWinTex[tileNum]);
			lpWinDev->SetTexture(0, plpWinTex[tileNum]);
			tileUpdated[tileNum] = true;
		}
		tileYOff += tilesW;
	}

	if (!Dx9PresentSkip) {
		UpdateScene();
		dxPresentFlag = false;
		if (lpWinDev->Present(nullptr, nullptr, hGameWnd, nullptr) == D3DERR_DEVICELOST) {
			if (lpWinDev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
				ReSetDisplayModeDx();
		}
	}
	else dxPresentFlag = true;
}


//________________________
void DX9win::UpdateScene() {

	if (!*is_winActive || isMapperSizing)
		return;

	if (lpWinDev == nullptr)
		return;
	if (plpWinTex == nullptr)
		return;

	lpWinDev->BeginScene();
	lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX));

	if (isShader2_0) {
		D3DXMatrixTranslation(&mxWorld, (float)x, (float)y, (float)z);

		D3DXMATRIX mxWorldPrjection;
		lpWinDev->GetTransform(D3DTS_PROJECTION, &mxWorldPrjection);
		mxWorld = mxWorld * mxWorldPrjection;

		static D3DXHANDLE hPalShaderWorld = 0;
		hPalShaderWorld = pPalEffect->GetParameterByName(0, "WorldViewProj");
		pPalEffect->SetMatrix(hPalShaderWorld, &mxWorld);
	}
	else {
		D3DXMatrixTranslation(&mxWorld, (float)x, (float)y, (float)z);
		lpWinDev->SetTransform(D3DTS_WORLD, &mxWorld);
	}

	for (DWORD tileNum = 0; tileNum < tilesTotal; tileNum++) {
		if (!tileUpdated[tileNum])
			continue;
		lpWinDev->SetTexture(0, plpWinTex[tileNum]);

		if (isShader2_0) {
			UINT numPasses;
			pPalEffect->Begin(&numPasses, 0);
			pPalEffect->BeginPass(0);
		}
		lpWinDev->DrawPrimitive(D3DPT_TRIANGLEFAN, (tileNum << 2), 2);
		if (isShader2_0) {
			pPalEffect->EndPass();
			pPalEffect->End();
		}

	}

	lpWinDev->EndScene();
}


//_______________
void Dx9Present() {

	if (!*is_winActive || isMapperSizing)
		return;

	if (Dx9PresentSkip)
		return;
	if (!lpD3DDevice)
		return;
	if (!dxPresentFlag)
		return;
	if (dx9Gui)
		dx9Gui->UpdateScene();
	if (lpD3DDevice->Present(nullptr, nullptr, hGameWnd, nullptr) == D3DERR_DEVICELOST) {
		if (lpD3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ReSetDisplayModeDx();
	}
	dxPresentFlag = false;
}


//_________________________________
void DrawFalloutWindow(LONG winRef) {

	WinStruct* win = GetWinStruct(winRef);
	if (!win)
		return;

	F_DrawToScrn(win, &win->rect, nullptr);
}


//______________________________________________
void __declspec(naked) draw_fallout_window(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push ebp

		push eax
		call DrawFalloutWindow
		add esp, 0x4

		pop ebp
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//_______________________________________________
void DrawFalloutWindows(RECT* pRect, BYTE* pBuff) {
	Dx9PresentSkip = 1;
	*drawWindowsFlag = 1;
	for (LONG i = 0; i < *numWindows; i++) {
		F_DrawToScrn(pWinArray[i], pRect, pBuff);
	}
	*drawWindowsFlag = 0;

	if (!pBuff && !IsMouseHidden()) {
		if (IsMouseInRect(pRect))
			F_ShowMouse();
	}
	Dx9PresentSkip = 0;
}


//____________________________________________________
void __declspec(naked) draw_fallout_windows_rect(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		push edx
		push eax
		call DrawFalloutWindows
		add esp, 0x8

		pushad
		call Dx9Present
		popad

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//__________________________________________________
void DrawFalloutWindowRect(LONG winRef, RECT* pRect) {

	WinStruct* pWin = GetWinStruct(winRef);
	if (!pWin)
		return;
	RECT rcNew = { 0,0,0,0 };

	CopyRect(&rcNew, pRect);
	rcNew.left += pWin->rect.left;
	rcNew.top += pWin->rect.top;
	rcNew.right += pWin->rect.left;
	rcNew.bottom += pWin->rect.top;

	F_DrawToScrn(pWin, &rcNew, nullptr);
}


//_____________________________________________
void __declspec(naked) draw_game_win_rect(void) {
	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push ebp

		mov Dx9PresentSkip, 1
		push eax
		mov eax, pWinRef_GameArea
		push dword ptr ds : [eax]
		call DrawFalloutWindowRect
		add esp, 0x8
		mov Dx9PresentSkip, 0

		pop ebp
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
}


//____________________________________
DWORD RGB_XBIT(BYTE r, BYTE g, BYTE b) { GrayScale(&r, &g, &b); return(r << 16) | (g << 8) | (b); }


//_______________________________________________________
void SetPalEntriesX(BYTE* palette, int offset, int count) {

	RECT tRect = { offset,0,offset + count,1 };

	D3DLOCKED_RECT primlRect;
	if (lpD3DTexturePalBack->LockRect(0, &primlRect, &tRect, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK)
		return;

	DWORD* pBuff = (DWORD*)primlRect.pBits;
	offset = 0;
	//palette
	while (count) {
		pBuff[offset] = RGB_XBIT(palette[0] << 2, palette[1] << 2, palette[2] << 2);
		palette += 3;
		offset++;
		count--;
	}
	lpD3DTexturePalBack->AddDirtyRect(&tRect);
	lpD3DTexturePalBack->UnlockRect(0);
	lpD3DDevice->UpdateTexture(lpD3DTexturePalBack, lpD3DTexturePal);


	FDrawWinRect(SCRN_RECT);
}


//___________________________________________________
void SetPalEntryX(int offset, BYTE r, BYTE g, BYTE b) {

	RECT tRect = { offset,0,offset + 1,1 };
	D3DLOCKED_RECT primlRect;
	if (lpD3DTexturePalBack->LockRect(0, &primlRect, &tRect, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK)
		return;

	DWORD* pBuff = (DWORD*)primlRect.pBits;

	pBuff[0] = RGB_XBIT(r << 2, g << 2, b << 2);

	lpD3DTexturePalBack->AddDirtyRect(&tRect);
	lpD3DTexturePalBack->UnlockRect(0);
	lpD3DDevice->UpdateTexture(lpD3DTexturePalBack, lpD3DTexturePal);
	FDrawWinRect(SCRN_RECT);
}


//_________________________________________
void __declspec(naked) set_pal_entryX(void) {

	__asm {
		push esi
		push edi
		push ebp

		push ecx
		push ebx
		push edx
		push eax
		call SetPalEntryX
		add esp, 0x10

		pop ebp
		pop edi
		pop esi
		ret
	}
}



//___________________________________________
void __declspec(naked) set_pal_entriesX(void) {

	__asm {
		push ecx
		push esi
		push edi
		push ebp

		push ebx
		push edx
		push eax
		call SetPalEntriesX
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop ecx
		ret
	}
}


//_______________________________________________
void __declspec(naked) set_all_pal_entriesX(void) {

	__asm {
		push edx
		push ebx
		push ecx
		push esi
		push edi
		push ebp

		push 256
		push 0
		push eax
		call SetPalEntriesX
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop ecx
		pop ebx
		pop edx
		ret
	}
}


//_________________________________________________________________________________________________________
HRESULT __stdcall GetPalEntriesX(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries) {

	if (lpEntries == nullptr)
		return DD_FALSE;

	RECT tRect = { (LONG)dwBase,0,(LONG)dwBase + (LONG)dwNumEntries,1 };

	D3DLOCKED_RECT primlRect;
	if (lpD3DTexturePalBack->LockRect(0, &primlRect, &tRect, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK)
		return DD_FALSE;

	PALETTEENTRY* pBuff = (PALETTEENTRY*)primlRect.pBits;
	dwBase = 0;

	while (dwNumEntries) {
		lpEntries[dwBase] = pBuff[dwBase];
		dwBase++;
		dwNumEntries--;
	}

	lpD3DTexturePalBack->UnlockRect(0);

	return DD_OK;
}


//___________
void DxExit() {
	if (dx9Gui)
		delete dx9Gui;
	dx9Gui = nullptr;

	if (pPalEffect != nullptr) {
		pPalEffect->Release();
		pPalEffect = nullptr;
	}
	if (lpD3DTexturePal != nullptr) {
		lpD3DTexturePal->Release();
		lpD3DTexturePal = nullptr;
	}
	if (lpD3DTexturePalBack != nullptr) {
		lpD3DTexturePalBack->Release();
		lpD3DTexturePalBack = nullptr;
	}
	if (lpD3DDevice != nullptr) {
		lpD3DDevice->Release();
		lpD3DDevice = nullptr;
	}
	if (lpD3D != nullptr) {
		lpD3D->Release();
		lpD3D = nullptr;
	}
}



//____________________
HRESULT SetD3DDevice() {

	if (lpD3D == nullptr)
		return -1;
	HRESULT hresult;
	// Get the current desktop display mode
	D3DFORMAT d3dFormat;
	D3DDISPLAYMODE d3ddm;
	if (lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm) == D3D_OK)
		d3dFormat = d3ddm.Format;
	else
		d3dFormat = D3DFMT_UNKNOWN;

	if (!isWindowed) {
		switch (COLOUR_BITS) {
		case 16:
			if (lpD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_R5G6B5, D3DFMT_R5G6B5, false) == D3D_OK)
				d3dFormat = D3DFMT_R5G6B5;
			break;
		case 32:
		case 24:
			if (lpD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, false) == D3D_OK)
				d3dFormat = D3DFMT_X8R8G8B8;
			break;
		default:
			break;
		}
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferWidth = SCR_WIDTH << scaler;
	d3dpp.BackBufferHeight = SCR_HEIGHT << scaler;
	d3dpp.BackBufferFormat = d3dFormat;
	d3dpp.BackBufferCount = 1;
	d3dpp.hDeviceWindow = hGameWnd;
	if (isWindowed) {
		d3dpp.Windowed = true;//
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.FullScreen_RefreshRateInHz = 0;
	}
	else {
		d3dpp.Windowed = false;//
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.FullScreen_RefreshRateInHz = REFRESH_RATE;
	}

	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;//D3DPRESENT_INTERVAL_DEFAULT;//D3DPRESENT_INTERVAL_ONE;//D3DPRESENT_DONOTWAIT;//

	// Create D3D Device
	if (lpD3DDevice == nullptr) {
		if (hresult = lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hGameWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &lpD3DDevice) != D3D_OK) {
			isSoftwareVertex = true;
			if (hresult = lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hGameWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpD3DDevice) != D3D_OK)
				return hresult;
		}
	}
	// Reset D3D Device
	else 
		hresult = lpD3DDevice->Reset(&d3dpp);

	return hresult;
}


//___________________
HRESULT SetSurfaces() {

	if (lpD3DDevice == nullptr)
		return -1;
	HRESULT hresult;

	//Setup rendering states
	hresult = lpD3DDevice->SetRenderState(D3DRS_LIGHTING, false);//TRUE);
	hresult = lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);//TRUE);
	hresult = lpD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	hresult = lpD3DDevice->SetRenderState(D3DRS_ZENABLE, false);
	hresult = lpD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	hresult = lpD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	hresult = lpD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	D3DXMATRIX Ortho2D;
	D3DXMatrixOrthoOffCenterLH(&Ortho2D, 0.0f, (float)(SCR_WIDTH), (float)(SCR_HEIGHT), 0.0f, -0.5f, 0.5f);

	hresult = lpD3DDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);

	initPalShader(&Ortho2D);
	if (dx9Gui)
		delete dx9Gui;
	dx9Gui = new DX9win(lpD3DDevice, 0, 0, SCR_WIDTH, SCR_HEIGHT);

	return hresult;
}


//___________
int DxSetup() {

	if (lpD3D == nullptr) {
		// Create the D3D object, which is needed to create the D3DDevice.
		if ((lpD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
			return -1;
	}
	*plpD3D = lpD3D;


	if (lpD3DDevice != nullptr)
		return 0;
	HRESULT hresult = 0;

	hresult = SetD3DDevice();
	if (hresult != D3D_OK)
		return -1;


	D3DCAPS9 caps;
	lpD3DDevice->GetDeviceCaps(&caps);
	// check for shader support of version 2
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		isShader2_0 = false;
	if (caps.PixelShaderVersion < D3DVS_VERSION(2, 0))
		isShader2_0 = false;


	DWORD usage = 0;
	if (!isShader2_0)
		usage = D3DUSAGE_DYNAMIC;

	if (lpD3DDevice->CreateTexture(256, 1, 1, usage, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &lpD3DTexturePal, nullptr) != D3D_OK)
		return -1;
	if (lpD3DDevice->CreateTexture(256, 1, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &lpD3DTexturePalBack, nullptr) != D3D_OK)
		return -1;

	hresult = SetSurfaces();
	if (hresult != D3D_OK)
		return -1;

	return 0;
}


//___________________________________
void __declspec(naked) dx_setup(void) {

	__asm {
		push edx
		push ebx
		push ecx
		push esi
		push edi

		call DxSetup

		pop edi
		pop esi
		pop ecx
		pop ebx
		pop edx
		ret
	}

}


//__________________________
HRESULT ReSetDisplayModeDx() {

	if (lpD3DDevice == nullptr)
		return -1;

	HRESULT hresult = 0;

	if (dx9Gui)
		delete dx9Gui;
	dx9Gui = nullptr;

	if (lpD3DTexturePal != nullptr) {
		lpD3DTexturePal->Release();
		lpD3DTexturePal = nullptr;
	}

	if (pPalEffect != nullptr) {
		pPalEffect->OnLostDevice();
		pPalEffect->Release();
		pPalEffect = nullptr;
	}

	hresult = SetD3DDevice();
	if (hresult != D3D_OK)
		return hresult;

	DWORD usage = 0;
	if (!isShader2_0)
		usage = D3DUSAGE_DYNAMIC;

	if (lpD3DDevice->CreateTexture(256, 1, 1, usage, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &lpD3DTexturePal, nullptr) != D3D_OK)
		return -1;

	lpD3DTexturePalBack->AddDirtyRect(nullptr);
	lpD3DDevice->UpdateTexture(lpD3DTexturePalBack, lpD3DTexturePal);

	hresult = SetSurfaces();

	return hresult;
}


//____________________________________________________________________________________________________________
void DxDraw(BYTE* fBuff, int fWidth, int fHeight, int fX, int fY, int subWidth, int subHeight, int tX, int tY) {

	if (dx9Gui)
		dx9Gui->Draw(fBuff, fWidth, fHeight, fX, fY, subWidth, subHeight, tX, tY);
	return;
}


//____________________________________________________________________________________________________________________________________
HRESULT LockDx(LPDIRECT3DTEXTURE9 lpD3DTextureMovie, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) {

	D3DLOCKED_RECT primlRect;
	if (lpD3DTextureMovie->LockRect(0, &primlRect, lpDestRect, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK) {
		MessageBox(nullptr, "LockDx failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	lpDDSurfaceDesc->lPitch = primlRect.Pitch;
	lpDDSurfaceDesc->lpSurface = primlRect.pBits;

	return 0;
}


//___________________________________________________________________
HRESULT UnlockDx(LPDIRECT3DTEXTURE9 lpD3DTextureMovie, LPRECT lpRect) {

	if (lpD3DTextureMovie->UnlockRect(0) != D3D_OK) {
		MessageBox(nullptr, "UnlockDx failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	return 0;
}


//__________________________________
void __declspec(naked) lock_dx(void) {

	__asm {
		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		call LockDx
		add esp, 0x14
		test eax, eax
		ret 0x14
	}
}


//____________________________________
void __declspec(naked) unlock_dx(void) {

	__asm {
		push dword ptr ss : [esp + 0x8]
		push dword ptr ss : [esp + 0x8]
		call UnlockDx
		add esp, 0x8
		ret 0x8
	}
}


//__________________________________________________________________________________________________________________________________
HRESULT _stdcall CreateMovieSurfaceDx(DDSURFACEDESC* DDSurfaceDesc, LPDIRECT3DTEXTURE9* plpD3DTextureMovie, IUnknown FAR* pUnkOuter) {

	HRESULT hresult = 0;
	if (lpD3DDevice == nullptr)
		hresult = -1;
	else {
		LPDIRECT3DTEXTURE9 lpD3DTextureMovie = nullptr;
		if (DDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 8)
			hresult = lpD3DDevice->CreateTexture(DDSurfaceDesc->dwWidth, DDSurfaceDesc->dwHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &lpD3DTextureMovie, nullptr);
		else if (DDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 16)
			hresult = lpD3DDevice->CreateTexture(DDSurfaceDesc->dwWidth, DDSurfaceDesc->dwHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_X1R5G5B5, D3DPOOL_SYSTEMMEM, &lpD3DTextureMovie, nullptr);
		*plpD3DTextureMovie = lpD3DTextureMovie;
	}

	if (hresult != DD_OK)
		MessageBox(nullptr, "MovieDx surface creation failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);

	return hresult;
}


DDSURFACEDESC DDSurfaceDescBuff;
DDSURFACEDESC DDSurfaceDescBuff2;


//____________________________________________________________________________________________________________________________
HRESULT _stdcall CreateMVEBuff1(DDSURFACEDESC* DDSurfaceDesc, LPDIRECT3DTEXTURE9* plpD3DTextureMovie, IUnknown FAR* pUnkOuter) {

	DDSurfaceDescBuff.dwSize = sizeof(DDSURFACEDESC);
	DDSurfaceDescBuff.dwWidth = DDSurfaceDesc->dwWidth;
	DDSurfaceDescBuff.dwHeight = DDSurfaceDesc->dwHeight;
	DDSurfaceDescBuff.lPitch = DDSurfaceDesc->dwWidth;
	DDSurfaceDescBuff.ddpfPixelFormat.dwRGBBitCount = DDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;

	DWORD buffSize = DDSurfaceDescBuff.dwWidth * DDSurfaceDescBuff.dwHeight;
	if (DDSurfaceDescBuff.ddpfPixelFormat.dwRGBBitCount == 16)
		buffSize = buffSize * 2;


	if (DDSurfaceDescBuff.lpSurface)
		delete DDSurfaceDescBuff.lpSurface;
	DDSurfaceDescBuff.lpSurface = new BYTE[buffSize];
	memset(DDSurfaceDescBuff.lpSurface, 0, buffSize);

	return DD_OK;
}


//____________________________________________________________________________________________________________________________
HRESULT _stdcall CreateMVEBuff2(DDSURFACEDESC* DDSurfaceDesc, LPDIRECT3DTEXTURE9* plpD3DTextureMovie, IUnknown FAR* pUnkOuter) {

	DDSurfaceDescBuff2.dwSize = sizeof(DDSURFACEDESC);
	DDSurfaceDescBuff2.dwWidth = DDSurfaceDesc->dwWidth;
	DDSurfaceDescBuff2.dwHeight = DDSurfaceDesc->dwHeight;
	DDSurfaceDescBuff2.lPitch = DDSurfaceDesc->dwWidth;
	DDSurfaceDescBuff2.ddpfPixelFormat.dwRGBBitCount = DDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;

	DWORD buffSize = DDSurfaceDescBuff2.dwWidth * DDSurfaceDescBuff2.dwHeight;
	if (DDSurfaceDescBuff2.ddpfPixelFormat.dwRGBBitCount == 16)
		buffSize = buffSize * 2;

	if (DDSurfaceDescBuff2.lpSurface)
		delete DDSurfaceDescBuff2.lpSurface;
	DDSurfaceDescBuff2.lpSurface = new BYTE[buffSize];
	memset(DDSurfaceDescBuff2.lpSurface, 0, buffSize);

	return DD_OK;
}


//____________________________________________________________________________________________________________________________________________________
HRESULT _stdcall MVE_Buff_Lock(LPDIRECT3DTEXTURE9 lpD3DTextureMovie, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) {

	lpDDSurfaceDesc->dwWidth = DDSurfaceDescBuff.dwWidth;
	lpDDSurfaceDesc->dwHeight = DDSurfaceDescBuff.dwHeight;
	lpDDSurfaceDesc->lPitch = DDSurfaceDescBuff.lPitch;//primlRect.Pitch;
	lpDDSurfaceDesc->lpSurface = DDSurfaceDescBuff.lpSurface;//primlRect.pBits;
	return DD_OK;
}


//________________________________________
void __declspec(naked) mve_buff_lock(void) {

	__asm {
		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		push dword ptr ss : [esp + 0x14]
		call MVE_Buff_Lock

		test eax, eax
		ret 0x14
	}
}


//_____________________________________________________________________________________________________________________________________________________
HRESULT _stdcall MVE_Buff_Lock2(LPDIRECT3DTEXTURE9 lpD3DTextureMovie, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) {

	lpDDSurfaceDesc->dwWidth = DDSurfaceDescBuff2.dwWidth;
	lpDDSurfaceDesc->dwHeight = DDSurfaceDescBuff2.dwHeight;
	lpDDSurfaceDesc->lPitch = DDSurfaceDescBuff2.lPitch;//primlRect.Pitch;
	lpDDSurfaceDesc->lpSurface = DDSurfaceDescBuff2.lpSurface;//primlRect.pBits;
	return DD_OK;
}


//_________________
void MVE_BuffFlip() {

	LPVOID buff = DDSurfaceDescBuff.lpSurface;

	DDSurfaceDescBuff.lpSurface = DDSurfaceDescBuff2.lpSurface;
	DDSurfaceDescBuff2.lpSurface = buff;
}


//______________________
void DirectX9_Fixes_CH() {

	FuncWrite32(0x4CC9F4, 0x023D, (DWORD)&dx_setup);

	MemWrite8(0x4CCF0C, 0x53, 0xE9);
	FuncWrite32(0x4CCF0D, 0x57565251, (DWORD)&DxExit);

	MemWrite32(0x6BCF84, 0, (DWORD)&DxDraw);
	MemWrite32(0x6BCF88, 0, 0);//(DWORD)&DDraw);
	MemWrite32(0x6BCD44, 0, 0);//(DWORD)&DDraw);
	MemWrite32(0x6BCD48, 0, (DWORD)&DxDraw);

	plpD3D = (LPDIRECT3D9*)FixAddress(0x52E090);
	pm_D3DTexture = (LPDIRECT3DTEXTURE9*)FixAddress(0x52E094);
	pm_D3DTexture2 = (LPDIRECT3DTEXTURE9*)FixAddress(0x52E098);
	plpDDPalette = (void**)FixAddress(0x52E09C);

	MemWrite8(0x4CCFB2, 0x56, 0xE9);
	FuncWrite32(0x4CCFB3, 0xE5895557, (DWORD)&set_pal_entryX);

	MemWrite8(0x4CD103, 0x51, 0xE9);
	FuncWrite32(0x4CD104, 0x89555756, (DWORD)&set_pal_entriesX);

	MemWrite8(0x4CD4DD, 0x53, 0xE9);
	FuncWrite32(0x4CD4DE, 0x57565251, (DWORD)&set_all_pal_entriesX);

	MemWrite16(0x502CC2, 0x7674, 0x9090);
	MemWrite16(0x502CCB, 0x6D74, 0x9090);

	MemWrite8(0x502CDF, 0x8B, 0xE8);
	FuncWrite32(0x502CE0, 0x6450FF01, (DWORD)&MVE_Buff_Lock);

	MemWrite8(0x502D09, 0x8B, 0xE8);
	FuncWrite32(0x502D0A, 0x6450FF02, (DWORD)&MVE_Buff_Lock2);

	MemWrite8(0x502D40, 0x6A, 0xC3);

	MemWrite16(0x502C21, 0x8B51, 0xE890);
	FuncWrite32(0x502C23, 0x1850FF01, (DWORD)&CreateMVEBuff1);

	MemWrite16(0x502C46, 0x8B51, 0xE890);
	FuncWrite32(0x502C48, 0x1850FF01, (DWORD)&CreateMVEBuff2);

	MemWrite16(0x485D7E, 0x128B, 0xC031);

	MemWrite8(0x485D8D, 0xFF, 0xE8);
	FuncWrite32(0x485D8E, 0xC0856452, (DWORD)&mve_buff_lock);

	MemWrite16(0x485E77, 0x90FF, 0xC483);
	MemWrite32(0x485E79, 0x80, 0x90909008);

	MemWrite16(0x485E74, 0x8B, 0xC031);


	MemWrite8(0x48633D, 0xFF, 0xE8);
	FuncWrite32(0x48633E, 0xC0856453, (DWORD)&mve_buff_lock);

	MemWrite16(0x4863A5, 0x92FF, 0xC483);
	FuncWrite32(0x4863A7, 0x80, 0x90909008);

	MemWrite8(0x502D70, 0xA1, 0xE9);
	FuncWrite32(0x502D71, 0x52EBE4, (DWORD)&MVE_BuffFlip);

	MemWrite8(0x4DD278, 0x68, 0xE9);
	FuncWrite32(0x4DD279, 0x24, (DWORD)&draw_fallout_window);

	MemWrite32(0x4811EA, 0x00483300, (DWORD)&draw_game_win_rect);

	drawWindowsFlag = (LONG*)0x6BE494;

	FuncReplace32(0x4DDB70, 0x0349, (DWORD)&draw_fallout_windows_rect);
}


//_________________________
void DirectX9_Fixes_MULTI() {

	FuncWrite32(0x4CAE39, 0x015F, (DWORD)&dx_setup);

	MemWrite8(0x4CB1B0, 0x53, 0xE9);
	FuncWrite32(0x4CB1B1, 0x55575251, (DWORD)&DxExit);

	MemWrite32(0x6ACA18, 0, (DWORD)&DxDraw);
	MemWrite32(0x6ACA1C, 0, 0);//(DWORD)&DDraw);
	MemWrite32(0x6AC7D8, 0, 0);//(DWORD)&DDraw);
	MemWrite32(0x6AC7DC, 0, (DWORD)&DxDraw);//&DxDrawMouse);

	plpD3D = (LPDIRECT3D9*)FixAddress(0x51E2B0);
	pm_D3DTexture = (LPDIRECT3DTEXTURE9*)FixAddress(0x51E2B4);
	pm_D3DTexture2 = (LPDIRECT3DTEXTURE9*)FixAddress(0x51E2B8);
	plpDDPalette = (void**)FixAddress(0x51E2BC);

	MemWrite8(0x4CB218, 0x56, 0xE9);
	FuncWrite32(0x4CB219, 0xEC835557, (DWORD)&set_pal_entryX);

	MemWrite8(0x4CB310, 0x51, 0xE9);
	FuncWrite32(0x4CB311, 0x81555756, (DWORD)&set_pal_entriesX);

	MemWrite8(0x4CB568, 0x53, 0xE9);
	FuncWrite32(0x4CB569, 0x57565251, (DWORD)&set_all_pal_entriesX);

	MemWrite16(0x4F5E72, 0x7674, 0x9090);
	MemWrite16(0x4F5E7B, 0x6D74, 0x9090);

	MemWrite8(0x4F5E8F, 0x8B, 0xE8);
	FuncWrite32(0x4F5E90, 0x6450FF01, (DWORD)&MVE_Buff_Lock);

	MemWrite8(0x4F5EB9, 0x8B, 0xE8);
	FuncWrite32(0x4F5EBA, 0x6450FF02, (DWORD)&MVE_Buff_Lock2);

	MemWrite8(0x4F5EF0, 0x6A, 0xC3);

	MemWrite16(0x4F5DD1, 0x8B51, 0xE890);
	FuncWrite32(0x4F5DD3, 0x1850FF01, (DWORD)&CreateMVEBuff1);

	MemWrite16(0x4F5DF6, 0x8B51, 0xE890);
	FuncWrite32(0x4F5DF8, 0x1850FF01, (DWORD)&CreateMVEBuff2);

	MemWrite16(0x48698E, 0x128B, 0xC031);

	MemWrite8(0x48699D, 0xFF, 0xE8);
	FuncWrite32(0x48699E, 0xC0856452, (DWORD)&mve_buff_lock);

	MemWrite16(0x486A87, 0x90FF, 0xC483);
	MemWrite32(0x486A89, 0x80, 0x90909008);

	MemWrite16(0x486A84, 0x8B, 0xC031);

	MemWrite8(0x486F4D, 0xFF, 0xE8);
	FuncWrite32(0x486F4E, 0xC0856453, (DWORD)&mve_buff_lock);

	MemWrite16(0x486FB5, 0x92FF, 0xC483);
	FuncWrite32(0x486FB7, 0x80, 0x90909008);

	MemWrite8(0x4F5F20, 0xA1, 0xE9);
	FuncWrite32(0x4F5F21, FixAddress(0x51EE04), (DWORD)&MVE_BuffFlip);

	MemWrite16(0x4D6F5C, 0x5253, 0x9090);
	MemWrite8(0x4D6F5E, 0xE8, 0xE9);
	FuncWrite32(0x4D6F5F, 0x0925, (DWORD)&draw_fallout_window);

	MemWrite32(0x481DBA, FixAddress(0x483ED0), (DWORD)&draw_game_win_rect);

	drawWindowsFlag = (LONG*)FixAddress(0x6ADF38);

	FuncReplace32(0x4D75A9, 0x0267, (DWORD)&draw_fallout_windows_rect);
}


//_____________________________
void DirectX9_Fixes(int region) {

	if (graphicsMode != 2)
		return;

	GraphicsLibSetup = &DxSetup;
	pPal_GetEntries = &GetPalEntriesX;

	if (region == 4)
		DirectX9_Fixes_CH();
	else
		DirectX9_Fixes_MULTI();
}
