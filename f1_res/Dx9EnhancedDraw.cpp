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
#include "F_Mapper.h"
#include "configTools.h"

#include "Dx9Enhanced.h"

#include <ddraw.h>

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "shaderPal.h"

HRESULT(__stdcall* pPal_GetEntries)(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries) = nullptr;

LPDIRECT3D9 lpD3D = nullptr;
LPDIRECT3DDEVICE9 lpD3DDevice = nullptr;
LPDIRECT3DTEXTURE9 lpD3DTexturePal = nullptr;
LPDIRECT3DTEXTURE9 lpD3DTexturePalBack = nullptr;

LPDIRECT3D9* plpD3D = nullptr;
///LPDIRECT3DDEVICE9 *plpD3DDevice = nullptr;
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
bool isDrawingGameWin = false;
bool isDDrawing = false;
void* F_DRAW_GAME_SCENE = nullptr;
void* F_DRAW_GAME_SCENE_SCROLL = nullptr;
void* F_DRAW_BUTTONS = nullptr;

float ScaleX_game = 1;
LONG xPosTrue = 0;
LONG yPosTrue = 0;
RECT rcGameTrue;

bool isMouseHeld = 0;

bool isMainPalActive = false;

LONG* pKeyValScreenShot = nullptr;


//____________________________________________________________________________
void ClearTexture(LPDIRECT3DTEXTURE9 plpTex, DWORD m_dwWidth, DWORD m_dwHeight) {

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(plpTex->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD))) {
		memset(lockedRect.pBits, 0xFF, lockedRect.Pitch * m_dwHeight);
		plpTex->UnlockRect(0);
	}
}


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


class DX9Mouse {
	float x;
	float y;
	float z;
	DWORD width;
	DWORD height;
	float scale;
	DWORD tileWidth;

	D3DXMATRIX mxManipulation;
	D3DXMATRIX mxScaling;
	D3DXMATRIX mxWorld;

	LPDIRECT3DDEVICE9 lpWinDev;
	LPDIRECT3DVERTEXBUFFER9 lpWinVB;
	LPDIRECT3DTEXTURE9 plpWinTex;
	LPDIRECT3DTEXTURE9 plpWinTexBack;
public:
	DX9Mouse(LPDIRECT3DDEVICE9 inlpD3DDev, float inX, float inY, DWORD inWidth, DWORD inHeight, float inScale);
	~DX9Mouse() {
		if (!lpWinDev)
			return;
		if (lpWinVB != nullptr) {
			lpWinVB->Release();
			lpWinVB = nullptr;
		}
		DWORD t = 0;
		if (plpWinTex) {
			plpWinTex->Release();
			plpWinTex = nullptr;
		}
		if (plpWinTexBack) {
			plpWinTexBack->Release();
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
	bool CreateMouseVB();
	void DrawTile8(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth);
	void DrawTile32(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth);
	void Draw(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, LONG tX, LONG tY);
	void UpdateScene();
	void SetPosition(float inX, float inY) {
		if (!isMouseHeld) {
			x = inX; y = inY;
		}
	};
};
DX9Mouse* dx9Mouse = nullptr;


//__________________________________________________________________________________________________________________
DX9Mouse::DX9Mouse(LPDIRECT3DDEVICE9 inlpD3DDev, float inX, float inY, DWORD inWidth, DWORD inHeight, float inScale) {
	if (!inlpD3DDev) {
		x = 0;
		y = 0;
		z = 0;
		width = 0;
		height = 0;
		scale = 1.0f;
		tileWidth = 0;
		lpWinDev = nullptr;
		lpWinVB = nullptr;
		plpWinTex = nullptr;
		plpWinTexBack = nullptr;
		return;
	}

	lpWinDev = inlpD3DDev;

	scale = inScale;
	x = inX + scale / 2;
	y = inY + scale / 2;
	z = 0;
	width = inWidth;
	height = inHeight;

	DWORD tileWidthTemp = 0;
	if (width > height)
		tileWidthTemp = width;
	else
		tileWidthTemp = height;

	tileWidth = 32;
	while (tileWidthTemp > tileWidth)
		tileWidth = tileWidth << 1;

	if (!CreateMouseVB()) {
		//MessageBox( nullptr, "CreateMouseVB failed", "Hi-Res patch Error", MB_ICONERROR);
		return;
	}

	D3DFORMAT colour = D3DFMT_A8R8G8B8;
	if (isShader2_0)
		colour = D3DFMT_L8;

	if (lpWinDev->CreateTexture(tileWidth, tileWidth, 1, 0, colour, D3DPOOL_DEFAULT, &plpWinTex, nullptr) == D3D_OK)
		ClearTexture(plpWinTex, tileWidth, tileWidth);
	else MessageBox(nullptr, "CreateMouseTexture failed", "Hi-Res patch Error", MB_ICONERROR);
	if (lpWinDev->CreateTexture(tileWidth, tileWidth, 1, 0, colour, D3DPOOL_SYSTEMMEM, &plpWinTexBack, nullptr) == D3D_OK)
		ClearTexture(plpWinTexBack, tileWidth, tileWidth);
	else MessageBox(nullptr, "CreateMouseTexture2 failed", "Hi-Res patch Error", MB_ICONERROR);
	lpWinDev->SetFVF(D3DFVF_SCRNVERTEX);
	lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX));

}


//____________________________
bool DX9Mouse::CreateMouseVB() {

	DWORD d3dUsageFlags = D3DUSAGE_WRITEONLY;
	if (isSoftwareVertex)
		d3dUsageFlags = d3dUsageFlags | D3DUSAGE_SOFTWAREPROCESSING;

	HRESULT hresult = 0;
	if (hresult = lpWinDev->CreateVertexBuffer(sizeof(SCRNVERTEX) * 4, d3dUsageFlags, D3DFVF_SCRNVERTEX, D3DPOOL_MANAGED, &lpWinVB, nullptr) != D3D_OK) {
		if (hresult == D3DERR_OUTOFVIDEOMEMORY)
			MessageBox(nullptr, "CreateMouseVB D3DERR_OUTOFVIDEOMEMORY", "Hi-Res patch Error", MB_ICONERROR);
		else if (hresult == D3DERR_INVALIDCALL)
			MessageBox(nullptr, "CreateMouseVB D3DERR_INVALIDCALL", "Hi-Res patch Error", MB_ICONERROR);
		else if (hresult == E_OUTOFMEMORY)
			MessageBox(nullptr, "CreateMouseVB E_OUTOFMEMORY", "Hi-Res patch Error", MB_ICONERROR);
		else
			MessageBox(nullptr, "CreateMouseVB Error", "Hi-Res patch Error", MB_ICONERROR);

		lpWinVB = nullptr;
		return false;
	}
	SCRNVERTEX* pVertices = nullptr;
	if (lpWinVB->Lock(0, sizeof(SCRNVERTEX) * 4, (void**)&pVertices, 0) != D3D_OK) {
		MessageBox(nullptr, "Can't lock Mouse Quad vertices", "Hi-Res patch Error", MB_ICONERROR);
		return false;
	}

	float left = -0.5f;
	float top = -0.5f;
	float right = left + (float)tileWidth;
	float bottom = top + (float)tileWidth;


	pVertices[0].x = left;
	pVertices[0].y = top;
	pVertices[0].z = 0.1f;
	pVertices[0].tu = 0.0f;
	pVertices[0].tv = 0.0f;

	pVertices[1].x = right;
	pVertices[1].y = top;
	pVertices[1].z = 0.1f;
	pVertices[1].tu = 1.0f;
	pVertices[1].tv = 0.0f;

	pVertices[2].x = right;
	pVertices[2].y = bottom;
	pVertices[2].z = 0.1f;
	pVertices[2].tu = 1.0f;
	pVertices[2].tv = 1.0f;

	pVertices[3].x = left;
	pVertices[3].y = bottom;
	pVertices[3].z = 0.1f;
	pVertices[3].tu = 0.0f;
	pVertices[3].tv = 1.0f;

	lpWinVB->Unlock();
	return true;
}


//_________________________________________________________________________________________________________________________________________________________________
void DX9Mouse::DrawTile8(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth) {
	toBuff.b += tY * toWidth + tX;
	fBuff += fY * fWidth + fX;

	for (DWORD y = 0; y < subHeight; y++) {
		for (DWORD x = 0; x < subWidth; x++)
			toBuff.b[x] = fBuff[x];
		toBuff.b += toWidth;
		fBuff += fWidth;
	}
}


//_____________________________________________________________________________________________________________________________________________________________________
void DX9Mouse::DrawTile32(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth) {
	toWidth = toWidth >> 2;
	toBuff.d += tY * toWidth + tX;
	fBuff += fY * fWidth + fX;

	D3DLOCKED_RECT primlRect;
	if (lpD3DTexturePal->LockRect(0, &primlRect, nullptr, D3DLOCK_READONLY) != D3D_OK)
		return;

	BUFF pbuff{ nullptr };
	pbuff.b = (BYTE*)primlRect.pBits;

	for (DWORD y = 0; y < subHeight; y++) {
		for (DWORD x = 0; x < subWidth; x++)
			toBuff.d[x] = pbuff.d[fBuff[x]];
		toBuff.d += toWidth;
		fBuff += fWidth;
	}
	lpD3DTexturePal->UnlockRect(0);
}


//___________________________________________________________________________________________________________________________________
void DX9Mouse::Draw(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, LONG tX, LONG tY) {

	if (!*is_winActive || isMapperSizing)
		return;

	if (lpWinDev == nullptr)
		return;
	if (plpWinTex == nullptr) {
		MessageBox(nullptr, "plpWinTex == nullptr", "Hi-Res patch Error", MB_ICONERROR);
		return;
	}
	if (plpWinTexBack == nullptr) {
		MessageBox(nullptr, "plpWinTexBack == nullptr", "Hi-Res patch Error", MB_ICONERROR);
		return;
	}
	lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX));

	RECT rcTile = { tX, tY, tX + (LONG)subWidth, tY + (LONG)subHeight };
	RECT rcMain = { 0, 0, (LONG)width - 1, (LONG)height - 1 };
	RECT rcFrom = { fX, fY, (LONG)subWidth, (LONG)subHeight };

	D3DLOCKED_RECT primlRect;
	if (plpWinTexBack->LockRect(0, &primlRect, &rcTile, D3DLOCK_NO_DIRTY_UPDATE) == D3D_OK) {

		BUFF tbuff{ nullptr };
		tbuff.b = (BYTE*)primlRect.pBits;
		if (isShader2_0)
			DrawTile8(fBuff, fWidth, fHeight, rcFrom.left, rcFrom.top, rcFrom.right, rcFrom.bottom, tbuff, 0, 0, primlRect.Pitch);
		else
			DrawTile32(fBuff, fWidth, fHeight, rcFrom.left, rcFrom.top, rcFrom.right, rcFrom.bottom, tbuff, 0, 0, primlRect.Pitch);

		plpWinTexBack->AddDirtyRect(&rcTile);
		plpWinTexBack->UnlockRect(0);
		lpWinDev->UpdateTexture(plpWinTexBack, plpWinTex);
		lpWinDev->SetTexture(0, plpWinTex);
	}

	Dx9Present();
}


//__________________________
void DX9Mouse::UpdateScene() {

	if (!*is_winActive || isMapperSizing)
		return;

	if (lpWinDev == nullptr)
		return;
	if (plpWinTex == nullptr)
		return;

	lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX));

	if (isShader2_0) {
		D3DXMatrixTranslation(&mxManipulation, (float)x, (float)y, (float)z);
		D3DXMatrixScaling(&mxScaling, (float)scale, (float)scale, (float)1.0f);
		D3DXMatrixMultiply(&mxWorld, &mxScaling, &mxManipulation);

		D3DXMATRIX mxWorldPrjection;
		lpWinDev->GetTransform(D3DTS_PROJECTION, &mxWorldPrjection);
		mxWorld = mxWorld * mxWorldPrjection;

		static D3DXHANDLE hPalShaderWorld = 0;
		hPalShaderWorld = pPalEffect->GetParameterByName(0, "WorldViewProj");
		pPalEffect->SetMatrix(hPalShaderWorld, &mxWorld);
	}
	else {
		D3DXMatrixTranslation(&mxManipulation, (float)x, (float)y, (float)z);
		D3DXMatrixScaling(&mxScaling, (float)scale, (float)scale, (float)1.0f);
		D3DXMatrixMultiply(&mxWorld, &mxManipulation, &mxScaling);
		lpWinDev->SetTransform(D3DTS_WORLD, &mxWorld);
	}

	lpWinDev->SetTexture(0, plpWinTex);

	if (isShader2_0) {
		UINT numPasses;
		pPalEffect->Begin(&numPasses, 0);
		pPalEffect->BeginPass(0);
	}
	lpWinDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	if (isShader2_0) {
		pPalEffect->EndPass();
		pPalEffect->End();
	}

}


class DX9win;

class WinStructDx {
public:
	LONG ref;//0x00
	DWORD flags;//0x04
	RECT rect;//left 0x08, top 0x0C, right 0x10, bottom 0x14
	LONG width;//0x18
	LONG height;//0x1C
	DWORD colour;//0x20//colour index offset?
	DWORD unknown24;//0x24//x?     //type related
	DWORD unknown28;//0x28//y?    //background image used when clearing window (optional)//type related
	BYTE* buff;//0x2C         // bytes frame data ref to palette
	ButtonStruct* ButtonList;//0x30//button struct list?
	DWORD unknown34;//0x34
	DWORD unknown38;//0x38
	DWORD unknown3C;//0x3C
	void (*pBlit)(BYTE* fBuff, LONG subWidth, LONG subHeight, LONG fWidth, BYTE* LONG, DWORD tWidth);//0x40//drawing func address
	DX9win* winDx;
};
WinStructDx* winDxCurrent = nullptr;


//__________
class DX9win {
	LONG* pRef;
	float x;
	float y;
	float z;
	DWORD width;
	DWORD height;
	float scaleX;
	float scaleY;
	DWORD tilesW;
	DWORD tilesH;
	DWORD tilesTotal;
	bool* tileUpdated;

	D3DXMATRIX mxManipulation;
	D3DXMATRIX mxScaling;
	D3DXMATRIX mxWorld;

	LPDIRECT3DDEVICE9 lpWinDev;
	LPDIRECT3DVERTEXBUFFER9 lpWinVB;
	LPDIRECT3DTEXTURE9* plpWinTex;
	LPDIRECT3DTEXTURE9* plpWinTexBack;
public:
	DX9win(LPDIRECT3DDEVICE9 inlpD3DDev, float inX, float inY, DWORD inWidth, DWORD inHeight, float inScaleX, float inScaleY, LONG* pInRef);
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
	void ClearTile8(DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth);
	void ClearTile32(DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth);
	void Clear(DWORD subWidth, DWORD subHeight, LONG tX, LONG tY);
	void UpdateScene();
	void SetPosition(float inX, float inY) { x = inX; y = inY; };
};


//_____________________________________________________________________________________________________________________________________________
DX9win::DX9win(LPDIRECT3DDEVICE9 inlpD3DDev, float inX, float inY, DWORD inWidth, DWORD inHeight, float inScaleX, float inScaleY, LONG* pInRef) {

	if (!inlpD3DDev) {
		pRef = nullptr;
		x = 0;
		y = 0;
		z = 0;
		width = 0;
		height = 0;
		scaleX = 1.0f;
		scaleY = 1.0f;
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
	pRef = pInRef;
	scaleX = inScaleX;
	scaleY = inScaleY;
	x = inX + scaleX / 2;
	y = inY + scaleY / 2;
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
		///MessageBox( nullptr, "CreateWinVB failed", "Hi-Res patch Error", MB_ICONERROR);
		return;
	}
	plpWinTex = new LPDIRECT3DTEXTURE9[tilesTotal];
	plpWinTexBack = new LPDIRECT3DTEXTURE9[tilesTotal];

	D3DFORMAT colour = D3DFMT_A8R8G8B8;
	if (isShader2_0)
		colour = D3DFMT_L8;

	for (DWORD t = 0; t < tilesTotal; t++) {
		lpWinDev->CreateTexture(256, 256, 1, 0, colour, D3DPOOL_DEFAULT, &plpWinTex[t], nullptr);//!=D3D_OK)
		ClearTexture(plpWinTex[t], 256, 256);
		//MessageBox( nullptr, "CreateTexture failed", "Hi-Res patch Error", MB_ICONERROR);
		lpWinDev->CreateTexture(256, 256, 1, 0, colour, D3DPOOL_SYSTEMMEM, &plpWinTexBack[t], nullptr);//!=D3D_OK)
		ClearTexture(plpWinTexBack[t], 256, 256);
		//MessageBox( nullptr, "CreateTexture2 failed", "Hi-Res patch Error", MB_ICONERROR);
	}
	lpWinDev->SetFVF(D3DFVF_SCRNVERTEX);
	lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX));

	Clear(width, height, 0, 0);
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
			MessageBox(nullptr, "CreateWinVB D3DERR_OUTOFVIDEOMEMORY", "Hi-Res patch Error", MB_ICONERROR);
		else if (hresult == D3DERR_INVALIDCALL)
			MessageBox(nullptr, "CreateWinVB D3DERR_INVALIDCALL", "Hi-Res patch Error", MB_ICONERROR);
		else if (hresult == E_OUTOFMEMORY)
			MessageBox(nullptr, "CreateWinVB E_OUTOFMEMORY", "Hi-Res patch Error", MB_ICONERROR);
		else
			MessageBox(nullptr, "CreateWinVB Error", "Hi-Res patch Error", MB_ICONERROR);

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

	BUFF pbuff{ nullptr };
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

	RECT rcTile{ 0,0,0,0 };
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

			BUFF tbuff{ nullptr };
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

	if (!*drawWindowsFlag) {
		lpWinDev->BeginScene();

		WinStructDx** pwinDxArray = (WinStructDx**)pWinArray;
		LONG i = 1;
		while (pwinDxArray[i]->ref != *pRef) {
			if (pwinDxArray[i]->winDx) {
				if (!(pwinDxArray[i]->flags & F_WIN_HIDDEN))
					pwinDxArray[i]->winDx->UpdateScene();
			}
			i++;
		}
		UpdateScene();

		lpWinDev->EndScene();
	}
	dxPresentFlag = true;
}


//____________________________________________________________________________________________________
void DX9win::ClearTile8(DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth) {
	toBuff.b += tY * toWidth + tX;

	for (DWORD y = 0; y < subHeight; y++) {
		for (DWORD x = 0; x < subWidth; x++)
			toBuff.b[x] = 0xFF;
		toBuff.b += toWidth;
	}
}


//_____________________________________________________________________________________________________
void DX9win::ClearTile32(DWORD subWidth, DWORD subHeight, BUFF toBuff, LONG tX, LONG tY, DWORD toWidth) {
	toWidth = toWidth >> 2;
	toBuff.d += tY * toWidth + tX;

	D3DLOCKED_RECT primlRect;
	if (lpD3DTexturePal->LockRect(0, &primlRect, nullptr, D3DLOCK_READONLY) != D3D_OK)
		return;

	BUFF pbuff{ nullptr };
	pbuff.b = (BYTE*)primlRect.pBits;

	for (DWORD y = 0; y < subHeight; y++) {
		for (DWORD x = 0; x < subWidth; x++)
			toBuff.d[x] = pbuff.d[0xFF];
		toBuff.d += toWidth;
	}
	lpD3DTexturePal->UnlockRect(0);
}



//___________________________________________________________________
void DX9win::Clear(DWORD subWidth, DWORD subHeight, LONG tX, LONG tY) {

	if (!*is_winActive || isMapperSizing)
		return;

	if (lpWinDev == nullptr)
		return;
	if (plpWinTex == nullptr)
		return;
	if (plpWinTexBack == nullptr)
		return;

	RECT rcMain = { tX, tY, tX + (LONG)subWidth, tY + (LONG)subHeight };

	RECT rcTile{ 0,0,0,0 };
	URECT32 rcFrom = { 0, 0, subWidth, subHeight };
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
		rcFrom.top = 0;
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
			rcFrom.left = 0;
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

			BUFF tbuff{ nullptr };
			tbuff.b = (BYTE*)primlRect.pBits;
			if (isShader2_0)
				ClearTile8(rcFrom.right, rcFrom.bottom, tbuff, 0, 0, primlRect.Pitch);
			else
				ClearTile32(rcFrom.right, rcFrom.bottom, tbuff, 0, 0, primlRect.Pitch);

			plpWinTexBack[tileNum]->AddDirtyRect(&rcTile);
			plpWinTexBack[tileNum]->UnlockRect(0);
			lpWinDev->UpdateTexture(plpWinTexBack[tileNum], plpWinTex[tileNum]);
			lpWinDev->SetTexture(0, plpWinTex[tileNum]);

			tileUpdated[tileNum] = true;
		}
		tileYOff += tilesW;
	}

	if (!*drawWindowsFlag) {
		lpWinDev->BeginScene();
		WinStructDx** pwinDxArray = (WinStructDx**)pWinArray;
		LONG i = 1;
		while (pwinDxArray[i]->ref != *pRef) {
			if (pwinDxArray[i]->winDx) {
				if (!(pwinDxArray[i]->flags & F_WIN_HIDDEN))
					pwinDxArray[i]->winDx->UpdateScene();
			}
			i++;
		}
		UpdateScene();
		lpWinDev->EndScene();
	}
	dxPresentFlag = true;

}


//________________________
void DX9win::UpdateScene() {

	if (!*is_winActive || isMapperSizing)
		return;

	if (lpWinDev == nullptr)
		return;
	if (plpWinTex == nullptr)
		return;

	lpWinDev->SetStreamSource(0, lpWinVB, 0, sizeof(SCRNVERTEX));



	if (isShader2_0) {

		D3DXMatrixTranslation(&mxManipulation, (float)x, (float)y, (float)z);
		D3DXMatrixScaling(&mxScaling, (float)scaleX, (float)scaleY, (float)1.0f);
		D3DXMatrixMultiply(&mxWorld, &mxScaling, &mxManipulation);

		D3DXMATRIX mxWorldPrjection;
		lpWinDev->GetTransform(D3DTS_PROJECTION, &mxWorldPrjection);
		mxWorld = mxWorld * mxWorldPrjection;

		static D3DXHANDLE hPalShaderWorld = 0;
		hPalShaderWorld = pPalEffect->GetParameterByName(0, "WorldViewProj");
		pPalEffect->SetMatrix(hPalShaderWorld, &mxWorld);
	}
	else {
		D3DXMatrixTranslation(&mxManipulation, (float)x, (float)y, (float)z);
		D3DXMatrixScaling(&mxScaling, (float)scaleX, (float)scaleY, (float)1.0f);
		D3DXMatrixMultiply(&mxWorld, &mxManipulation, &mxScaling);
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

	lpD3DDevice->BeginScene();

	lpD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 0, 0);

	WinStructDx** pwinDxArray = (WinStructDx**)pWinArray;
	for (LONG i = 1; i < *numWindows; i++) {

		if (pwinDxArray[i]->winDx) {
			if (!(pwinDxArray[i]->flags & F_WIN_HIDDEN))
				pwinDxArray[i]->winDx->UpdateScene();
		}
	}

	if (dx9Mouse && !IsMouseHidden() && isMainPalActive)
		dx9Mouse->UpdateScene();

	lpD3DDevice->EndScene();

	if (lpD3DDevice->Present(nullptr, nullptr, hGameWnd, nullptr) == D3DERR_DEVICELOST) {
		if (lpD3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ReSetDisplayModeDx();
	}

	dxPresentFlag = false;
}


//___________________________________________
void __declspec(naked) set_present_flag(void) {

	__asm {
		mov dxPresentFlag, 1
		ret;
	}
}


//_______________________________________
void __declspec(naked) ddraw_stuff2(void) {

	__asm {
		push ecx
		push esi
		push edi
		push ebp
		sub esp, 0x34
		mov ecx, 0x4D6FDF
		jmp ecx
		ret;
	}
}


//_____________________________________________
void HandleScreenshot(IDirect3DDevice9* device) {
	DWORD tcHandleScreenshot = GetTickCount();
	LPDIRECT3DSURFACE9 pd3dsBack = nullptr;
	LPDIRECT3DSURFACE9 pd3dsTemp = nullptr;

	// Grab the back buffer into a surface
	if (SUCCEEDED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pd3dsBack))) {
		D3DSURFACE_DESC desc;
		pd3dsBack->GetDesc(&desc);

		LPDIRECT3DSURFACE9 pd3dsCopy = nullptr;
		if (desc.MultiSampleType != D3DMULTISAMPLE_NONE) {
			if (SUCCEEDED(device->CreateRenderTarget(desc.Width, desc.Height, desc.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &pd3dsCopy, nullptr))) {
				if (SUCCEEDED(device->StretchRect(pd3dsBack, nullptr, pd3dsCopy, nullptr, D3DTEXF_NONE))) {
					pd3dsBack->Release();
					pd3dsBack = pd3dsCopy;
				}
				else {
					pd3dsCopy->Release();
				}
			}
		}

		if (SUCCEEDED(device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pd3dsTemp, nullptr))) {
			DWORD tmpTimeGRTD = GetTickCount();
			if (SUCCEEDED(device->GetRenderTargetData(pd3dsBack, pd3dsTemp))) {
				D3DLOCKED_RECT lockedSrcRect;
				if (SUCCEEDED(pd3dsTemp->LockRect(&lockedSrcRect, nullptr, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK | D3DLOCK_NO_DIRTY_UPDATE))) {
					switch (desc.Format) {
					case D3DFMT_A8R8G8B8:
					case D3DFMT_X8R8G8B8:
						SaveBMP24(desc.Width, desc.Height, 4, lockedSrcRect.Pitch, (BYTE*)lockedSrcRect.pBits);
						break;
					default:
						break;
					}

				}
			}
			pd3dsTemp->Release();
		}
		pd3dsBack->Release();
	}
}


//___________________
void TakeScreenShot() {
	HandleScreenshot(lpD3DDevice);
}


//_______________________
void TakeScreenShot_Win() {
	LONG mouseX = 0, mouseY = 0;
	F_GetMousePos(&mouseX, &mouseY);
	LONG winRef = F_GetWinAtPos(mouseX, mouseY);
	WinStruct* win = GetWinStruct(winRef);
	SaveBMP8(win->width, win->height, win->buff, pCURRENT_PAL);

}


//__________________________________________
void __declspec(naked)take_screen_shot(void) {

	__asm {
		pushad
		call TakeScreenShot
		popad
		ret
	}
}


//___________________________________________
void __declspec(naked)screen_shot_check(void) {

	__asm {

		cmp eax, 0x18A
		jne exitFunc
		pushad
		call TakeScreenShot_Win
		popad
		add dword ptr ss : [esp] , 7
		ret
		exitFunc :
		push ebx
			mov ebx, pKeyValScreenShot
			cmp eax, dword ptr ds : [ebx]
			pop ebx
			ret
	}
}


//________________________________________________
void F_DrawButtons(WinStructDx* win, RECT* rcDraw) {

	__asm {
		mov edx, rcDraw
		mov eax, win
		call F_DRAW_BUTTONS
	}

}


//___________________________________________________________
void DDrawStuff(WinStructDx* win, RECT* rcDraw, BYTE* toBuff) {

	if (!win)
		return;
	if (win->flags & F_WIN_HIDDEN)
		return;


	if (rcDraw->left > win->rect.right)
		return;
	if (rcDraw->right < win->rect.left)
		return;
	if (rcDraw->top > win->rect.bottom)
		return;
	if (rcDraw->bottom < win->rect.top)
		return;

	RECT rcWinDraw{ 0,0,0,0 };

	if (rcDraw->left > win->rect.left)
		rcWinDraw.left = rcDraw->left - win->rect.left;
	else
		rcWinDraw.left = 0;
	if (rcDraw->right > win->rect.right)
		rcWinDraw.right = win->width - 1;
	else
		rcWinDraw.right = rcDraw->right - win->rect.left;

	if (rcDraw->top > win->rect.top)
		rcWinDraw.top = rcDraw->top - win->rect.top;
	else
		rcWinDraw.top = 0;
	if (rcDraw->bottom > win->rect.bottom)
		rcWinDraw.bottom = win->height - 1;
	else
		rcWinDraw.bottom = rcDraw->bottom - win->rect.top;


	DWORD skipTemp = Dx9PresentSkip;
	Dx9PresentSkip = 1;
	F_DrawButtons(win, rcDraw);
	Dx9PresentSkip = skipTemp;

	DX9win* dxWin = win->winDx;
	if (dxWin) {
		dxWin->Draw(win->buff, win->width, win->height, rcWinDraw.left, rcWinDraw.top, rcWinDraw.right - rcWinDraw.left + 1, rcWinDraw.bottom - rcWinDraw.top + 1, rcWinDraw.left, rcWinDraw.top);
	}

	isDDrawing = true;
	if (!IsMouseHidden() && IsMouseInRect(rcDraw))

		F_ShowMouse();
	isDDrawing = false;
	Dx9Present();

}


//______________________________________
void __declspec(naked) ddraw_stuff(void) {

	__asm {
		pushad
		push ebx
		push edx
		push eax
		call DDrawStuff
		add esp, 0xC
		popad
		ret;
	}
}


//__________________________________
void DrawFalloutWindow(LONG winRef) {

}


//_____________________________________________
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

	LONG backWinNum = 1;

	for (LONG i = *numWindows - 1; i > 0; i--) {
		if (pWinArray[i]->rect.left <= pRect->left && pWinArray[i]->rect.top <= pRect->top && pWinArray[i]->rect.right >= pRect->right && pWinArray[i]->rect.bottom >= pRect->bottom)
			backWinNum = i;
	}

	for (LONG i = backWinNum; i < *numWindows; i++) {

		if (pWinArray[i]->ref == *pWinRef_GameArea) {
			RECT rcGame{
				(LONG)(pRect->left / ScaleX_game),
				(LONG)(pRect->top / ScaleX_game),
				(LONG)(pRect->right / ScaleX_game),
				(LONG)(pRect->bottom / ScaleX_game) };
			F_DrawToScrn(pWinArray[i], &rcGame, pBuff);
		}
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

	WinStruct* pWin = GetWinStruct(*pWinRef_GameArea);
	if (!pWin)
		return;

	RECT rcNew = {
		(LONG)(pRect->left / ScaleX_game),
		(LONG)(pRect->top / ScaleX_game),
		(LONG)(pRect->right / ScaleX_game),
		(LONG)(pRect->bottom / ScaleX_game) };

	rcNew.left += pWin->rect.left;
	rcNew.top += pWin->rect.top;
	rcNew.right += pWin->rect.left;
	rcNew.bottom += pWin->rect.top;

	F_DrawToScrn(pWin, &pWin->rect, nullptr);

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


//_____________________________________
DWORD RGB_XBIT(BYTE r, BYTE g, BYTE b) { GrayScale(&r, &g, &b); return(r << 16) | (g << 8) | (b); }


//_______________________________________________________
void SetPalEntriesX(BYTE* palette, int offset, int count) {

	int oriOffset = offset;
	RECT tRect = { offset,0,offset + count,1 };


	D3DLOCKED_RECT primlRect;
	if (lpD3DTexturePalBack->LockRect(0, &primlRect, &tRect, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK)
		return;

	DWORD* pBuff = (DWORD*)primlRect.pBits;
	offset = 0;

	while (count) {
		pBuff[offset] = RGB_XBIT(palette[0] << 2, palette[1] << 2, palette[2] << 2);

		if (oriOffset + offset == 0xFF && isMainPalActive)
			pBuff[offset] = pBuff[offset] | (0 << 24);
		else pBuff[offset] = pBuff[offset] | (255 << 24);

		palette += 3;
		offset++;
		count--;
	}
	lpD3DTexturePalBack->AddDirtyRect(&tRect);
	lpD3DTexturePalBack->UnlockRect(0);
	lpD3DDevice->UpdateTexture(lpD3DTexturePalBack, lpD3DTexturePal);

	FDrawWinRect(SCRN_RECT);
}


//____________________________________________________
void SetPalEntryX(int offset, BYTE r, BYTE g, BYTE b) {

	RECT tRect = { offset,0,offset + 1,1 };
	D3DLOCKED_RECT primlRect;
	if (lpD3DTexturePalBack->LockRect(0, &primlRect, &tRect, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK)
		return;

	DWORD* pBuff = (DWORD*)primlRect.pBits;

	pBuff[0] = RGB_XBIT(r << 2, g << 2, b << 2);

	if (offset == 0xFF && isMainPalActive)
		pBuff[0] = pBuff[0] | (0 << 24);
	else pBuff[0] = pBuff[0] | (255 << 24);

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

	if (dx9Mouse)
		delete dx9Mouse;
	dx9Mouse = nullptr;

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
	else {
		hresult = lpD3DDevice->Reset(&d3dpp);
	}

	return hresult;
}


//___________________
HRESULT SetSurfaces() {
	if (lpD3DDevice == nullptr)
		return -1;
	HRESULT hresult;

	//Setup rendering states
	hresult = lpD3DDevice->SetRenderState(D3DRS_LIGHTING, false);//TRUE);
	hresult = lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);//false);//TRUE);
	hresult = lpD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	hresult = lpD3DDevice->SetRenderState(D3DRS_ZENABLE, false);
	hresult = lpD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);


	hresult = lpD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	hresult = lpD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);



	D3DXMATRIX Ortho2D;
	D3DXMatrixOrthoOffCenterLH(&Ortho2D, 0.0f, (float)(SCR_WIDTH), (float)(SCR_HEIGHT), 0.0f, -0.5f, 0.5f);

	hresult = lpD3DDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);

	initPalShader(&Ortho2D);

	return hresult;
}


//_______________________________________________________
BYTE* SetDxMouse(DWORD buffSize, LONG width, LONG height) {

	if (dx9Mouse)
		delete dx9Mouse;
	dx9Mouse = new DX9Mouse(lpD3DDevice, 0, 0, width, height, 1.0f);

	return FAllocateMemory(buffSize);
}


//_________________
void ResetDxMouse() {

	RECT rcMouse;
	F_GetMouseRect(&rcMouse);

	if (dx9Mouse)
		delete dx9Mouse;
	dx9Mouse = new DX9Mouse(lpD3DDevice, (float)rcMouse.left, (float)rcMouse.top, rcMouse.right - rcMouse.left + 1, rcMouse.bottom - rcMouse.top + 1, 1.0f);
}


//___________________________
void SetDxWin(WinStruct* win) {
	if (graphicsMode <= 1)
		return;
	WinStructDx* winStructDx = (WinStructDx*)win;
	if (!winStructDx)
		return;
	if (winStructDx->winDx)
		delete winStructDx->winDx;

	if (winStructDx->ref == *pWinRef_GameArea) {
		float scaleX = (float)SCR_WIDTH / (float)winStructDx->width;
		float scaleY = (float)(SCR_HEIGHT - 100) / (float)winStructDx->height;

		winStructDx->winDx = new DX9win(lpD3DDevice, (float)winStructDx->rect.left, (float)winStructDx->rect.top, winStructDx->width, winStructDx->height, scaleX, scaleY, &winStructDx->ref);
	}
	else
		winStructDx->winDx = new DX9win(lpD3DDevice, (float)winStructDx->rect.left, (float)winStructDx->rect.top, winStructDx->width, winStructDx->height, 1.0f, 1.0f, &winStructDx->ref);
}


//_____________________________________________
void __declspec(naked) allocate_mem_mouse(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		push ebx//new mouse height
		push edx//new mouse width
		push eax//new mouse bufferSize
		call SetDxMouse
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}

}


//_______________________________
void ClearMemMouse(BYTE* mseBuff) {

	RECT rcMouse;
	F_GetMouseRect(&rcMouse);
	DWORD buffSize = (rcMouse.right - rcMouse.left + 1) * (rcMouse.bottom - rcMouse.top + 1);
	memset(mseBuff, 0xFF, buffSize);
}


//__________________________________________
void __declspec(naked) clear_mem_mouse(void) {

	__asm {
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		push eax//mouse buffer
		call ClearMemMouse
		add esp, 0x4

		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	}
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

	if (lpD3DDevice->CreateTexture(256, 1, 1, usage, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &lpD3DTexturePal, nullptr) != D3D_OK)
		return -1;
	if (lpD3DDevice->CreateTexture(256, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &lpD3DTexturePalBack, nullptr) != D3D_OK)
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

	if (dx9Mouse)
		delete dx9Mouse;
	dx9Mouse = nullptr;

	for (LONG i = 1; i < *numWindows; i++) {
		WinStructDx** pwinDxArray = (WinStructDx**)pWinArray;
		if (pwinDxArray[i]->winDx)
			delete pwinDxArray[i]->winDx;
		pwinDxArray[i]->winDx = nullptr;
	}

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

	if (lpD3DDevice->CreateTexture(256, 1, 1, usage, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &lpD3DTexturePal, nullptr) != D3D_OK)
		return -1;

	lpD3DTexturePalBack->AddDirtyRect(nullptr);
	lpD3DDevice->UpdateTexture(lpD3DTexturePalBack, lpD3DTexturePal);

	hresult = SetSurfaces();

	ResetDxMouse();
	return hresult;
}


//____________________________________________________________________________________________________________
void DxDraw(BYTE* fBuff, int fWidth, int fHeight, int fX, int fY, int subWidth, int subHeight, int tX, int tY) {

	if (winDxCurrent) {
		DX9win* win = winDxCurrent->winDx;
		if (win)
			win->Draw(fBuff, fWidth, fHeight, fX, fY, subWidth, subHeight, tX - winDxCurrent->rect.left, tY - winDxCurrent->rect.top);
	}
	return;
}


//_________________________________________________________________________________________________________________
void DxDrawMouse(BYTE* fBuff, int fWidth, int fHeight, int fX, int fY, int subWidth, int subHeight, int tX, int tY) {

	if (dx9Mouse) {
		dx9Mouse->SetPosition((float)tX - fX, (float)tY - fY);
		dx9Mouse->Draw(fBuff, fWidth, fHeight, 0, 0, fWidth, fHeight, 0, 0);

	}
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


//__________________________________
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


//______________________________________________________________________________________________________________________________________________________
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


//_________________________________________________
void SetWinPosDx(LONG winRef, LONG xPos, LONG yPos) {

	WinStructDx* win = (WinStructDx*)GetWinStruct(winRef);

	RECT rcWin;
	CopyRect(&rcWin, &win->rect);

	if (xPos < 0)
		xPos = 0;
	if (yPos < 0)
		yPos = 0;

	win->rect.left = xPos;
	win->rect.top = yPos;
	win->rect.right = xPos + win->width - 1;
	win->rect.bottom = yPos + win->height - 1;

	if (!win->winDx)
		win->winDx = new DX9win(lpD3DDevice, (float)win->rect.left, (float)win->rect.top, win->width, win->height, 1.0f, 1.0f, &win->ref);//(lpD3DDevice, 0, 0, 8000, 3600);

	if (win->flags & F_WIN_HIDDEN)
		return;

	dxPresentFlag = true;
	DrawFalloutWindows(&rcWin, nullptr);
}


//___________________________________________
void __declspec(naked) allocate_win_mem(void) {

	__asm {
		add eax, 0x4
		push eax
		call FAllocateMemory
		mov dword ptr ds : [eax + 0x44] , 0;
		add esp, 0x4
			ret
	}
}


//____________________________________________
void __declspec(naked) create_dx_win_pos(void) {

	__asm {
		pushad
		//push 1
		push ebx//y
		push edx//x
		push eax//winRef
		call SetWinPosDx
		add esp, 0xC
		popad
		ret
	}
}


//_________________________________
void DestroyWinDx(WinStructDx* win) {
	if (!win)
		return;
	if (win->winDx)
		delete win->winDx;
	win->winDx = 0;
}


//_____________________________________________
void __declspec(naked) destroy_dx_win_pos(void) {

	__asm {
		push eax
		call GetWinStruct
		add esp, 0x4
		pushad
		push eax//winRef
		call DestroyWinDx
		add esp, 0x4
		popad
		ret
	}
}


//____________________________________
void LoadedPaletteCheck(char* palName) {
	isMainPalActive = CompareCharArray_IgnoreCase(palName, "color.pal", 9);
}


//_______________________________________________
void __declspec(naked) loaded_palette_check(void) {

	__asm {
		pushad
		push eax
		call LoadedPaletteCheck
		add esp, 0x4
		popad

		mov edx, 0x200
		ret
	}
}


//____________________________________________________
void __declspec(naked) loaded_palette_splash_fix(void) {

	__asm {
		mov isMainPalActive, 0
		mov ebx, 0x300
		ret
	}
}


//_________________________
void DirectX9_Fixes() {

	if (graphicsMode != 2)
		return;

	GraphicsLibSetup = &DxSetup;
	pPal_GetEntries = &GetPalEntriesX;

	FuncWrite32(0x4B5BD9, 0x014B, (DWORD)&dx_setup);

	MemWrite8(0x4B5F3C, 0x53, 0xE9);
	FuncWrite32(0x4B5F3D, 0x55575251, (DWORD)&DxExit);


	MemWrite32(0x671F7C, 0, (DWORD)&DxDrawMouse);//mouseDraw
	MemWrite32(0x671F78, 0, 0);//(DWORD)&DxDraw);//clearMouse background 16bit draw
	MemWrite32(0x6721B8, 0, (DWORD)&DxDraw);//regularDraw



	plpD3D = (LPDIRECT3D9*)FixAddress(0x539ED0);
	pm_D3DTexture = (LPDIRECT3DTEXTURE9*)FixAddress(0x539ED4);
	pm_D3DTexture2 = (LPDIRECT3DTEXTURE9*)FixAddress(0x539ED8);
	plpDDPalette = (void**)FixAddress(0x539EDC);


	MemWrite8(0x4B5FA4, 0x56, 0xE9);
	FuncWrite32(0x4B5FA5, 0xEC835557, (DWORD)&set_pal_entryX);

	MemWrite8(0x4B609C, 0x51, 0xE9);
	FuncWrite32(0x4B609D, 0x81555756, (DWORD)&set_pal_entriesX);

	MemWrite8(0x4B62F4, 0x53, 0xE9);
	FuncWrite32(0x4B62F5, 0x57565251, (DWORD)&set_all_pal_entriesX);


	MemWrite16(0x4D8BF2, 0x7674, 0x9090);
	MemWrite16(0x4D8BFB, 0x6D74, 0x9090);

	MemWrite8(0x4D8C0F, 0x8B, 0xE8);
	FuncWrite32(0x4D8C10, 0x6450FF01, (DWORD)&MVE_Buff_Lock);

	MemWrite8(0x4D8C39, 0x8B, 0xE8);
	FuncWrite32(0x4D8C3A, 0x6450FF02, (DWORD)&MVE_Buff_Lock2);

	MemWrite8(0x4D8C70, 0x6A, 0xC3);

	MemWrite16(0x4D8B51, 0x8B51, 0xE890);
	FuncWrite32(0x4D8B53, 0x1850FF01, (DWORD)&CreateMVEBuff1);

	MemWrite16(0x4D8B76, 0x8B51, 0xE890);
	FuncWrite32(0x4D8B78, 0x1850FF01, (DWORD)&CreateMVEBuff2);


	MemWrite16(0x478C2E, 0x128B, 0xC031);
	MemWrite8(0x478C3D, 0xFF, 0xE8);
	FuncWrite32(0x478C3E, 0xC0856452, (DWORD)&mve_buff_lock);

	MemWrite16(0x478D24, 0x8B, 0xC031);

	MemWrite16(0x478D27, 0x90FF, 0xC483);
	MemWrite32(0x478D29, 0x80, 0x90909008);



	MemWrite8(0x4791ED, 0xFF, 0xE8);
	FuncWrite32(0x4791EE, 0xC0856453, (DWORD)&mve_buff_lock);

	MemWrite16(0x479255, 0x92FF, 0xC483);
	FuncWrite32(0x479257, 0x80, 0x90909008);

	MemWrite8(0x4D8CA0, 0xA1, 0xE9);
	FuncWrite32(0x4D8CA1, FixAddress(0x53ACDC), (DWORD)&MVE_BuffFlip);


	MemWrite32(0x473E42, FixAddress(0x47607C), (DWORD)&draw_game_win_rect);

	drawWindowsFlag = (LONG*)FixAddress(0x6AC2E8);

	FuncReplace32(0x4C3B91, 0x0267, (DWORD)&draw_fallout_windows_rect);

	MemWrite8(0x4C35C4, 0x51, 0xE9);
	FuncWrite32(0x4C35C5, 0x83555756, (DWORD)&ddraw_stuff);
	MemWrite16(0x4C35C9, 0x34EC, 0x9090);


	FuncReplace32(0x4B4E9D, 0xFFFFA29F, (DWORD)&allocate_mem_mouse);

	FuncReplace32(0x4B50F3, 0xECE9, (DWORD)&clear_mem_mouse);

	FuncReplace32(0x4C2873, 0xFFFEC8C9, (DWORD)&allocate_win_mem);

	FuncReplace32(0x4C245E, 0xFFFECCDE, (DWORD)&allocate_win_mem);

	FuncReplace32(0x4C29B7, 0x0AD1, (DWORD)&create_dx_win_pos);

	FuncReplace32(0x4C2A65, 0x1407, (DWORD)&destroy_dx_win_pos);

	F_DRAW_BUTTONS = (void*)FixAddress(0x4C6040);


	//prevent new_window function from checking window size against screen size.
	MemWrite8(0x4C284D, 0xA1, 0xEB);
	MemWrite32(0x4C284E, FixAddress(0x672198), 0x9090901E);


	MemWrite8(0x4B3CC4, 0x53, 0xE9);
	FuncWrite32(0x4B3CC5, 0x57565251, (DWORD)&take_screen_shot);

	pKeyValScreenShot = (LONG*)FixAddress(0x671F04);

	MemWrite16(0x4B39A3, 0x053B, 0xE890);
	FuncWrite32(0x4B39A5, (DWORD)pKeyValScreenShot, (DWORD)&screen_shot_check);


	MemWrite8(0x4C09BC, 0xBA, 0xE8);
	FuncWrite32(0x4C09BD, 0x200, (DWORD)&loaded_palette_check);

	MemWrite8(0x485538, 0xBB, 0xE8);
	FuncWrite32(0x485539, 0x300, (DWORD)&loaded_palette_splash_fix);
}
