/*                             The MIT License (MIT)

Copyright (c) 2016 Sumwunn @ github.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include "stdafx.h"
#include <windows.h>
#include <fstream>
#include <string>
#include <atlstr.h>

// Externing these for ExportsFunctions.asm.
extern "C" void* DllProxyFunction01Addr;
extern "C" void* DllProxyFunction02Addr;

// For ExportFunctions.asm.
extern "C" int SetupDllProxy();

// For SetupDllProxy.
int LoadDlls();

void* DllProxyFunction01Addr;
void* DllProxyFunction02Addr;

BOOL HasProxyDllBeenSetup = FALSE;

int SetupDllProxy() {

	// Data.
	TCHAR DllProxyWinDirPath[MAX_PATH];

	HMODULE DllProxy_hModule;

	LPCTSTR DllProxyName = L"X3DAudio1_7.dll";
	LPCTSTR DllSystemPath = L"\\System32\\";
	LPCSTR DllProxyFunction01Name = "X3DAudioCalculate";
	LPCSTR DllProxyFunction02Name = "X3DAudioInitialize";

	if (HasProxyDllBeenSetup == TRUE) {
		return 0;
	}

	GetWindowsDirectory(DllProxyWinDirPath, MAX_PATH);
	_tcscat_s(DllProxyWinDirPath, DllSystemPath);
	_tcscat_s(DllProxyWinDirPath, DllProxyName);
	DllProxy_hModule = LoadLibrary(DllProxyWinDirPath);

	DllProxyFunction01Addr = GetProcAddress(DllProxy_hModule, DllProxyFunction01Name);
	DllProxyFunction02Addr = GetProcAddress(DllProxy_hModule, DllProxyFunction02Name);

	// This works great because X3DAudio's function gets called just before Fallout 4's intro starts.
	// So no waiting for Steam decryption, yay!
	LoadDlls();

	HasProxyDllBeenSetup = TRUE;

	return 0;
}

int LoadDlls() {

	LPCTSTR ExpectedEXE = L"Fallout4.exe";
	LPCTSTR ExpectedEditorEXE = L"CreationKit.exe";
	HMODULE hModule = NULL;

	// Check for expected exe.
	if (GetModuleHandleEx(NULL, ExpectedEXE, &hModule) != NULL) {
		// Open up file with dlls.
		std::ifstream DllLoaderTxtFile;
		DllLoaderTxtFile.open("DllLoader.txt");
		// No dll loader list found!
		if (!DllLoaderTxtFile) {
			return 0;
		}

		// Read file with dlls.
		std::string DllToLoad;
		while (std::getline(DllLoaderTxtFile, DllToLoad))
		{
			TCHAR DllToLoadTCHAR[MAX_PATH];
			_tcscpy_s(DllToLoadTCHAR, CA2T(DllToLoad.c_str()));
			LoadLibrary(DllToLoadTCHAR);
		}
		// Cleanup.
		DllLoaderTxtFile.close();
		return 0;
	}

	// Check for expected editor exe.
	if (GetModuleHandleEx(NULL, ExpectedEditorEXE, &hModule) != NULL) {
		// Open up file with dlls.
		std::ifstream DllLoaderTxtFile;
		DllLoaderTxtFile.open("DllLoaderEditor.txt");
		// No dll loader list found!
		if (!DllLoaderTxtFile) {
			return 0;
		}

		// Read file with dlls.
		std::string DllToLoad;
		while (std::getline(DllLoaderTxtFile, DllToLoad))
		{
			TCHAR DllToLoadTCHAR[MAX_PATH];
			_tcscpy_s(DllToLoadTCHAR, CA2T(DllToLoad.c_str()));
			LoadLibrary(DllToLoadTCHAR);
		}
		// Cleanup.
		DllLoaderTxtFile.close();
		return 0;
	}

	return 0;
}
