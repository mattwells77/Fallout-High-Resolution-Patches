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

#include <ddraw.h>
#pragma comment(lib,"ddraw.lib") 
#pragma comment(lib,"dxguid.lib") 

union PIXEL {
	PALETTEENTRY pe;
	DWORD p32;
	WORD p16;
};
static PIXEL palMain[256];


LPDIRECTDRAW7 FAR* plpDD = nullptr;
LPDIRECTDRAWSURFACE7 FAR* plpDDSurface = nullptr;
LPDIRECTDRAWSURFACE7 FAR* plpDDSurface2 = nullptr;
LPDIRECTDRAWPALETTE FAR* plpDDPalette = nullptr;

LPDIRECTDRAW7 FAR lpDD = nullptr;
LPDIRECTDRAWSURFACE7 FAR lpDDSurface = nullptr;
LPDIRECTDRAWSURFACE7 FAR lpDDBackSurface = nullptr;
LPDIRECTDRAWPALETTE FAR lpDDPalette = nullptr;

//_______________________________________
void GrayScale(BYTE* r, BYTE* g, BYTE* b) {
	if (!isGrayScale)
		return;
	*r = (BYTE)(((*r << 2) + (*r << 1) + (*g << 3) + (*b << 1)) >> 4);
	*g = *r;
	*b = *r;
}

DWORD RGB_8BIT(BYTE r, BYTE g, BYTE b) { GrayScale(&r, &g, &b); return(PC_NOCOLLAPSE << 24) | (b << 16) | (g << 8) | (r); }

DWORD RGB_32BIT(BYTE r, BYTE g, BYTE b) { GrayScale(&r, &g, &b); return(r << 16) | (g << 8) | (b); }


//________________________________________
DWORD RGB_16BIT565(BYTE r, BYTE g, BYTE b) {
	GrayScale(&r, &g, &b);
	r = r >> 3;
	g = g >> 2;
	b = b >> 3;
	return(r << 11) | (g << 5) | (b);
}


//________________________________________
DWORD RGB_16BIT555(BYTE r, BYTE g, BYTE b) {
	GrayScale(&r, &g, &b);
	r = r >> 3;
	g = g >> 3;
	b = b >> 3;
	return(r << 10) | (g << 5) | (b);
}


typedef DWORD(*P_RGB_FIX)(BYTE r, BYTE g, BYTE b);
P_RGB_FIX RGB_FIX;


//____________________________________________________________________________________________________________________________________________________
typedef void (*BLIT_FUNC)(BYTE* fBuff, int fWidth, int fHeight, int fX, int fY, int subWidth, int subHeight, BUFF toBuff, int tX, int tY, int toWidth);

BLIT_FUNC Blit = nullptr;


//______________________________________________________________________________________________________________________________________
void Blit8(BYTE* fBuff, int fWidth, int fHeight, int fX, int fY, int subWidth, int subHeight, BUFF toBuff, int tX, int tY, int toWidth) {

	toBuff.b += tY * toWidth + tX;
	fBuff += fY * fWidth + fX;

	for (int y = 0; y < subHeight; y++) {
		for (int x = 0; x < subWidth; x++) {
			toBuff.b[x] = fBuff[x];
		}
		toBuff.b += toWidth;
		fBuff += fWidth;
	}
}


//______________________________________________________________________________________________________________________________________
void Blit16(BYTE* fBuff, int fWidth, int fHeight, int fX, int fY, int subWidth, int subHeight, BUFF toBuff, int tX, int tY, int toWidth) {
	toWidth = toWidth >> 1;
	toBuff.w += tY * toWidth + tX;
	fBuff += fY * fWidth + fX;

	for (int y = 0; y < subHeight; y++) {
		for (int x = 0; x < subWidth; x++) {
			toBuff.w[x] = palMain[fBuff[x]].p16;
		}
		toBuff.w += toWidth;
		fBuff += fWidth;
	}
}


//______________________________________________________________________________________________________________________________________
void Blit32(BYTE* fBuff, int fWidth, int fHeight, int fX, int fY, int subWidth, int subHeight, BUFF toBuff, int tX, int tY, int toWidth) {
	toWidth = toWidth >> 2;
	toBuff.d += tY * toWidth + tX;
	fBuff += fY * fWidth + fX;

	for (int y = 0; y < subHeight; y++) {
		for (int x = 0; x < subWidth; x++) {
			toBuff.d[x] = palMain[fBuff[x]].p32;
		}
		toBuff.d += toWidth;
		fBuff += fWidth;
	}
}


//_________________________________________________________
void SetPalEntries(BYTE* palette, int palOffset, int count) {
	int num = count;
	DWORD e = palOffset;

	while (num) {
		palMain[e].p32 = RGB_FIX(palette[0] << 2, palette[1] << 2, palette[2] << 2);
		palette += 3;
		e++;
		num--;
	}
	if (lpDDPalette != nullptr)
		lpDDPalette->SetEntries(0, palOffset, count, &palMain[palOffset].pe);
	else
		FDrawWinRect(SCRN_RECT);
}


//______________________________________________________
void SetPalEntry(int palOffset, BYTE r, BYTE g, BYTE b) {
	palMain[palOffset].p32 = RGB_FIX(r << 2, g << 2, b << 2);

	if (lpDDPalette != nullptr)
		lpDDPalette->SetEntries(0, palOffset, 1, &palMain[palOffset].pe);
	else
		FDrawWinRect(SCRN_RECT);
}


//_______________________________________
void __declspec(naked) set_pal_entry(void) {

	__asm {
		push esi
		push edi
		push ebp

		push ecx
		push ebx
		push edx
		push eax
		call SetPalEntry
		add esp, 0x10

		pop ebp
		pop edi
		pop esi
		ret
	}
}


//__________________________________________
void __declspec(naked) set_pal_entries(void) {

	__asm {
		push ecx
		push esi
		push edi
		push ebp

		push ebx
		push edx
		push eax
		call SetPalEntries
		add esp, 0xC

		pop ebp
		pop edi
		pop esi
		pop ecx
		ret
	}
}


//______________________________________________
void __declspec(naked) set_all_pal_entries(void) {

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
		call SetPalEntries
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


//____________________________________________________________________________________________________
HRESULT __stdcall GetPalEntries(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries) {
	if (lpEntries == nullptr)
		return DD_FALSE;

	for (DWORD num = 0; num < dwNumEntries; num++) {
		if (dwBase < 256)
			lpEntries[num] = palMain[dwBase].pe;
		dwBase++;
	}
	return DD_OK;
}


//___________________________________________________________________________________________________________
void DDraw(BYTE* fBuff, int fWidth, int fHeight, int fX, int fY, int subWidth, int subHeight, int tX, int tY) {

	if (!*is_winActive || isMapperSizing)
		return;

	DDSURFACEDESC2 DDSurfaceDesc;
	ZeroMemory(&DDSurfaceDesc, sizeof(DDSurfaceDesc));
	DDSurfaceDesc.dwSize = sizeof(DDSurfaceDesc);

	while (lpDDBackSurface->Lock(nullptr, &DDSurfaceDesc, DDLOCK_WAIT, 0) == DDERR_SURFACELOST)
		lpDDBackSurface->Restore();

	if (tY + subHeight > (LONG)DDSurfaceDesc.dwHeight) {
		if (tY > (LONG)DDSurfaceDesc.dwHeight)
			return;
		else subHeight = DDSurfaceDesc.dwHeight - tY;
	}

	if (tX + subWidth > (LONG)DDSurfaceDesc.dwWidth) {
		if (tX > (LONG)DDSurfaceDesc.dwWidth)
			return;
		else subWidth = (LONG)DDSurfaceDesc.dwWidth - tX;
	}

	BUFF tbuff{ nullptr };
	tbuff.b = (BYTE*)DDSurfaceDesc.lpSurface;
	Blit(fBuff, fWidth, fHeight, fX, fY, subWidth, subHeight, tbuff, tX, tY, DDSurfaceDesc.lPitch);

	lpDDBackSurface->Unlock(nullptr);

	POINT p{ 0,0 };
	ClientToScreen(hGameWnd, &p);

	RECT fromRect{ tX, tY, tX + subWidth, tY + subHeight };

	RECT destRect{
	(fromRect.left << scaler) + p.x,
	(fromRect.top << scaler) + p.y,
	(fromRect.right << scaler) + p.x,
	(fromRect.bottom << scaler) + p.y };


	while (lpDDSurface->Blt(&destRect, lpDDBackSurface, &fromRect, DDBLT_WAIT, nullptr) == DDERR_SURFACELOST) {
		lpDDSurface->Restore();
	}

}


//_______________
void DDrawClear() {
	if (!*is_winActive || isMapperSizing)
		return;

	DDSURFACEDESC2 DDSurfaceDesc;
	ZeroMemory(&DDSurfaceDesc, sizeof(DDSurfaceDesc));
	DDSurfaceDesc.dwSize = sizeof(DDSurfaceDesc);
	while (lpDDBackSurface->Lock(nullptr, &DDSurfaceDesc, DDLOCK_WAIT, 0) == DDERR_SURFACELOST)
		lpDDBackSurface->Restore();

	memset(DDSurfaceDesc.lpSurface, '\0', DDSurfaceDesc.dwHeight * DDSurfaceDesc.lPitch);

	lpDDBackSurface->Unlock(nullptr);

	POINT p{ 0,0 };
	ClientToScreen(hGameWnd, &p);

	RECT fromRect{ 0, 0, (LONG)DDSurfaceDesc.dwWidth, (LONG)DDSurfaceDesc.dwHeight };

	RECT destRect{
		fromRect.left + p.x,
		fromRect.top + p.y,
		fromRect.right + p.x,
		fromRect.bottom + p.y };

	while (lpDDSurface->Blt(&destRect, lpDDBackSurface, &fromRect, DDBLT_WAIT, nullptr) == DDERR_SURFACELOST)
		lpDDSurface->Restore();

}


//_______________________
HRESULT Restore_Palette() {
	HRESULT hresult = 0;
	if (lpDDSurface == nullptr)
		return hresult;

	DDSURFACEDESC2 DDSurfaceDesc;
	ZeroMemory(&DDSurfaceDesc, sizeof(DDSurfaceDesc));
	DDSurfaceDesc.dwSize = sizeof(DDSurfaceDesc);
	if (hresult = lpDDSurface->GetSurfaceDesc(&DDSurfaceDesc) != DD_OK) {
		MessageBox(nullptr, "GetSurfaceDesc Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return hresult;
	}

	lpDDSurface->SetPalette(nullptr);

	if (lpDDPalette)
		lpDDPalette->Release();
	lpDDPalette = nullptr;
	*plpDDPalette = nullptr;


	if (DDSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 8) {

		if (hresult = lpDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, (PALETTEENTRY*)palMain, &lpDDPalette, 0) != DD_OK) {
			MessageBox(nullptr, "Palette Creation Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
			return hresult;
		}
		if (hresult = lpDDSurface->SetPalette(lpDDPalette) != DD_OK) {
			MessageBox(nullptr, "Set Palette Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
			return hresult;
		}
		*plpDDPalette = lpDDPalette;

		RGB_FIX = (DWORD(__cdecl*)(BYTE, BYTE, BYTE))RGB_8BIT;
		Blit = &Blit8;
	}
	else if (DDSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16) {
		if (DDSurfaceDesc.ddpfPixelFormat.dwGBitMask == 0x7E0)
			RGB_FIX = (DWORD(__cdecl*)(BYTE, BYTE, BYTE))RGB_16BIT565;
		else
			RGB_FIX = (DWORD(__cdecl*)(BYTE, BYTE, BYTE))RGB_16BIT555;

		Blit = &Blit16;
	}
	else if (DDSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 24 || DDSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32) {
		RGB_FIX = (DWORD(__cdecl*)(BYTE, BYTE, BYTE))RGB_32BIT;
		Blit = &Blit32;
	}

	return hresult;
}


//______________________________
HRESULT Restore_PrimarySurface() {
	if (!lpDD)
		return 0;

	HRESULT hresult = 0;

	if (lpDDSurface)
		lpDDSurface->Release();
	lpDDSurface = nullptr;
	*plpDDSurface = nullptr;
	*plpDDSurface2 = nullptr;

	DDSURFACEDESC2 DDSurfaceDesc;
	ZeroMemory(&DDSurfaceDesc, sizeof(DDSurfaceDesc));
	DDSurfaceDesc.dwSize = sizeof(DDSurfaceDesc);
	DDSurfaceDesc.dwFlags = DDSD_CAPS;
	DDSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if (hresult = lpDD->CreateSurface(&DDSurfaceDesc, &lpDDSurface, 0) != DD_OK) {
		MessageBox(nullptr, "Primary Surface Creation Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	*plpDDSurface = lpDDSurface;
	*plpDDSurface2 = lpDDSurface;

	LPDIRECTDRAWCLIPPER FAR lpDDClipper;

	if (hresult = lpDD->CreateClipper(0, &lpDDClipper, 0) != DD_OK) {
		MessageBox(nullptr, "CreateClipper Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	if (hresult = lpDDClipper->SetHWnd(0, hGameWnd) != DD_OK) {
		MessageBox(nullptr, "SetHWnd Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	if (hresult = lpDDSurface->SetClipper(lpDDClipper) != DD_OK) {
		MessageBox(nullptr, "SetClipper Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	return hresult;
}


//___________________________
HRESULT Restore_BackSurface() {
	if (!lpDD)
		return 0;

	HRESULT hresult = 0;

	if (lpDDBackSurface)
		lpDDBackSurface->Release();
	lpDDBackSurface = nullptr;

	DDSURFACEDESC2 DDSurfaceDesc;
	ZeroMemory(&DDSurfaceDesc, sizeof(DDSurfaceDesc));
	DDSurfaceDesc.dwSize = sizeof(DDSurfaceDesc);

	DDSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	DDSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	DDSurfaceDesc.dwWidth = SCR_WIDTH;
	DDSurfaceDesc.dwHeight = SCR_HEIGHT;

	if (hresult = lpDD->CreateSurface(&DDSurfaceDesc, &lpDDBackSurface, 0) != DD_OK) {
		MessageBox(nullptr, "Back Surface Creation Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
		return hresult;
	}

	return hresult;
}


//______________
int DDrawSetup() {

	if (!lpDD) {

		LPDIRECTDRAW FAR lpDDtemp = nullptr;
		if (DirectDrawCreate(0, &lpDDtemp, 0) != DD_OK) {
			MessageBox(nullptr, "Direct Draw Creation Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
			return -1;
		}
		if (lpDDtemp->QueryInterface(IID_IDirectDraw7, (LPVOID*)&lpDD) != DD_OK) {
			MessageBox(nullptr, "QueryInterface Direct Draw 7 Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
			return -1;
		}

		if (lpDDtemp) {
			lpDDtemp->Release();
			lpDDtemp = nullptr;
		}

		*plpDD = lpDD;
	}
	int CLIENT_WIDTH = SCR_WIDTH << scaler;
	int CLIENT_HEIGHT = SCR_HEIGHT << scaler;

	HRESULT hResult = 0;
	if (isWindowed) {
		if (lpDD->SetCooperativeLevel(hGameWnd, DDSCL_NORMAL) != DD_OK) {
			MessageBox(nullptr, "SetCooperativeLevel 'Normal' Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
			return -1;
		}
	}
	else {
		if (lpDD->SetCooperativeLevel(hGameWnd, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_MULTITHREADED) != DD_OK) {
			MessageBox(nullptr, "SetCooperativeLevel 'Exclusive' Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
			return -1;
		}
		if (lpDD->SetDisplayMode(CLIENT_WIDTH, CLIENT_HEIGHT, COLOUR_BITS, REFRESH_RATE, 0) == DDERR_UNSUPPORTED) {
			MessageBox(nullptr, "The selected Display Mode is unsupported", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
			return -1;
		}
	}


	if (Restore_PrimarySurface())
		return -1;
	if (Restore_BackSurface())
		return -1;
	if (Restore_Palette())
		return -1;

	return 0;
}


//________________________________________
void __declspec(naked) ddraw_setup(void) {

	__asm {
		push edx
		push ebx
		push ecx
		push esi
		push edi
		call DDrawSetup
		pop edi
		pop esi
		pop ecx
		pop ebx
		pop edx
		ret
	}

}


//______________
void DDrawExit() {
	if (!lpDD)
		return;
	if (lpDDSurface)
		lpDDSurface->Release();
	lpDDSurface = nullptr;
	*plpDDSurface = nullptr;
	*plpDDSurface2 = nullptr;

	if (lpDDBackSurface)
		lpDDBackSurface->Release();
	lpDDBackSurface = nullptr;
	if (lpDDPalette)
		lpDDPalette->Release();
	lpDDPalette = nullptr;
	*plpDDPalette = nullptr;

	lpDD->RestoreDisplayMode();

	lpDD->Release();
	lpDD = nullptr;
	*plpDD = nullptr;
}


//________________________
void  ReSizeDDrawSurface() {
	if (!lpDD)
		return;
	Restore_BackSurface();
}


//____________________________
void RestoreDDrawDisplayMode() {
	if (!lpDD)
		return;

	if (lpDD->RestoreDisplayMode() != DD_OK)
		MessageBox(nullptr, "RestoreDisplayMode Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);

	if (lpDD->SetCooperativeLevel(hGameWnd, DDSCL_NORMAL) != DD_OK)
		MessageBox(nullptr, "SetCooperativeLevel 'Normal' Failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);
}


//_____________________
void ReSetDisplayMode() {

	if (!lpDD)
		return;

	if (lpDDSurface)
		lpDDSurface->Release();
	lpDDSurface = nullptr;
	*plpDDSurface = nullptr;
	*plpDDSurface2 = nullptr;

	if (lpDDBackSurface)
		lpDDBackSurface->Release();
	lpDDBackSurface = nullptr;
	if (lpDDPalette)
		lpDDPalette->Release();
	lpDDPalette = nullptr;
	*plpDDPalette = nullptr;

	DDrawSetup();
	SetPalEntries(pMAIN_PAL, 0, 256);
}


//_____________________________________________________________________________________________________________________________________
HRESULT _stdcall CreateMovieSurface(DDSURFACEDESC* DDSurfaceDesc, LPDIRECTDRAWSURFACE7 FAR* plpDDMovieSurface, IUnknown FAR* pUnkOuter) {

	DDSURFACEDESC2 DDSurfaceDesc2;
	ZeroMemory(&DDSurfaceDesc2, sizeof(DDSurfaceDesc2));
	DDSurfaceDesc2.dwSize = sizeof(DDSurfaceDesc2);

	DDSurfaceDesc2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	DDSurfaceDesc2.dwHeight = DDSurfaceDesc->dwHeight;
	DDSurfaceDesc2.dwWidth = DDSurfaceDesc->dwWidth;

	DDSurfaceDesc2.ddpfPixelFormat.dwFlags = DDSurfaceDesc->ddpfPixelFormat.dwFlags;
	DDSurfaceDesc2.ddpfPixelFormat.dwRGBBitCount = DDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
	DDSurfaceDesc2.ddpfPixelFormat.dwRBitMask = DDSurfaceDesc->ddpfPixelFormat.dwRBitMask;//
	DDSurfaceDesc2.ddpfPixelFormat.dwGBitMask = DDSurfaceDesc->ddpfPixelFormat.dwGBitMask;//
	DDSurfaceDesc2.ddpfPixelFormat.dwBBitMask = DDSurfaceDesc->ddpfPixelFormat.dwBBitMask;//

	DDSurfaceDesc2.ddsCaps.dwCaps = DDSurfaceDesc->ddsCaps.dwCaps;

	HRESULT hresult = lpDD->CreateSurface(&DDSurfaceDesc2, plpDDMovieSurface, pUnkOuter);
	if (hresult != DD_OK)
		MessageBox(nullptr, "MovieDD surface creation failed", "Hi-Res Patch Error", MB_ICONEXCLAMATION | MB_OK);

	return hresult;
}


//______________________
void DirectDraw7_Fixes() {

	if (graphicsMode != 1)
		return;

	GraphicsLibSetup = &DDrawSetup;
	pPal_GetEntries = &GetPalEntries;

	MemWrite8(0x4B5F3C, 0x53, 0xE9);
	FuncWrite32(0x4B5F3D, 0x55575251, (DWORD)&DDrawExit);

	MemWrite32(0x671F7C, 0, (DWORD)&DDraw);//mouseDraw
	MemWrite32(0x671F78, 0, 0);//(DWORD)&DDraw);clearMouse background 16bit draw
	MemWrite32(0x6721B8, 0, (DWORD)&DDraw);//regularDraw


	plpDD = (LPDIRECTDRAW7 FAR*)FixAddress(0x539ED0);
	plpDDSurface = (LPDIRECTDRAWSURFACE7 FAR*)FixAddress(0x539ED4);
	plpDDSurface2 = (LPDIRECTDRAWSURFACE7 FAR*)FixAddress(0x539ED8);
	plpDDPalette = (LPDIRECTDRAWPALETTE FAR*)FixAddress(0x539EDC);


	MemWrite8(0x4B5FA4, 0x56, 0xE9);
	FuncWrite32(0x4B5FA5, 0xEC835557, (DWORD)&set_pal_entry);

	MemWrite8(0x4B609C, 0x51, 0xE9);
	FuncWrite32(0x4B609D, 0x81555756, (DWORD)&set_pal_entries);

	MemWrite8(0x4B62F4, 0x53, 0xE9);
	FuncWrite32(0x4B62F5, 0x57565251, (DWORD)&set_all_pal_entries);

	MemWrite16(0x4D8B51, 0x8B51, 0xE890);
	FuncWrite32(0x4D8B53, 0x1850FF01, (DWORD)&CreateMovieSurface);

	MemWrite16(0x4D8B76, 0x8B51, 0xE890);
	FuncWrite32(0x4D8B78, 0x1850FF01, (DWORD)&CreateMovieSurface);
}
