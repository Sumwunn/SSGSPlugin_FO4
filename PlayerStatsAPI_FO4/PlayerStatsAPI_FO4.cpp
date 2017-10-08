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
#include <cstdlib>

// Fallout 4 version specific data.
void* StatsPointerRetrieverRVA = (void*)NULL; // Address that will give us Health & Stamina values.
void* StatsPointerRetrieverVA = (void*)NULL;
void* CurrentCarryWeightRVA = (void*)NULL; // Address that will give us the players current weight value.
extern "C" void* CurrentCarryWeightVA; // For StatsRetriver.asm.
void* CurrentCarryWeightVA = (void*)NULL;

// Defined functions.
// ASM
extern "C" int GetEntryPointRVA(HMODULE Module);
extern "C" int HookWrite(void* HookAddress, void* HookDestination, DWORD HookRegister);
extern "C" void* ConvertRVAToVA(HMODULE Module, void* Address);
extern "C" int StatsPointerRetriever();
extern "C" int StatsGetValue(DWORD StatsValue);
extern "C" int CarryWeightGetCurrentValue();

#ifndef PLAYERSTATSAPI_FO4_STATICLIB
extern "C" __declspec(dllexport) int InstallHook() {
#else
int InstallHook() {
#endif

	//////// Setup Part 1 - Addresses ////////

	// Get module of target to writes hooks to.
	HMODULE TargetModule = GetModuleHandle(L"Fallout4.exe");
	if (TargetModule == NULL) {
		// Incorrect process.
		return -1;
	}

	//// Fallout 4 Version Check ////
	// Using the entry point rva from pe header as my verison check.
	int Result = GetEntryPointRVA(TargetModule);
	if (Result == 0x06CD6310) {
		// Fill in version specific data (v1.10.20.0.0).
		StatsPointerRetrieverRVA = (void*)0xD78187;
		CurrentCarryWeightRVA = (void*)0x58F2E60;
	}
#ifdef FO4_BETA_SUPPORT
	else if (Result == 0) {
		// Fill in version specific data ().
		StatsPointerRetrieverRVA = (void*)0;
		CurrentCarryWeightRVA = (void*)0;
	}
#endif
	else {
		// Unsupported version.
		return -2;
	}

	//////// Setup Part 2 - Hooks ////////

	// Get the hook's addresses ready.
	StatsPointerRetrieverVA = ConvertRVAToVA(TargetModule, StatsPointerRetrieverRVA);
	CurrentCarryWeightVA = ConvertRVAToVA(TargetModule, CurrentCarryWeightRVA);

	// Write Hook.
	HookWrite(StatsPointerRetrieverVA, StatsPointerRetriever, 8);

	return TRUE;
}

#ifndef PLAYERSTATSAPI_FO4_STATICLIB
extern "C" __declspec(dllexport) int PlayerStatsGetValue(int StatsValue) {
#else
int PlayerStatsGetValue(int StatsValue) {
#endif

	// StatsValue.
	// 1 = Health.
	// 2 = Stamina.
	// 3 = Rads.
	// 4 = Weight.

	// Current value.
	int CurrentValue = NULL;
	int CurrentValue2 = NULL;
	int CurrentValueDiff = NULL;

	//////////////// -HEALTH- ////////////////
	if (StatsValue == 1) {
		// In GameSense Interface: 0 = Dead, 100 = Good.

		// Get current HEALTH value and return it to caller.
		CurrentValue = StatsGetValue(1);
		// If current value is negative, set to ZERO.
		if (CurrentValue < 0) {
			return 0;
		}
		// If HEALTH is above 100, set to 100.
		if (CurrentValue > 100) {
			return 100;
		}
		// Set HEALTH value.
		else {
			return CurrentValue;
		}
	}

	//////////////// -STAMINA- ////////////////
	else if (StatsValue == 2) {
		// In GameSense Interface: 0 = None, 100 = Full.

		// Get current STAMINA value and return it to caller.
		CurrentValue = StatsGetValue(2);
		// If current value is negative, set to ZERO.
		if (CurrentValue < 0) {
			return 0;
		}
		// If STAMINA is above 100, set to 100.
		if (CurrentValue > 100) {
			return 100;
		}
		// Set STAMINA value.
		else {
			return CurrentValue;
		}
	}

	//////////////// -RADS- ////////////////
	else if (StatsValue == 3) {
		// In GameSense Interface: 0 = No RADS, 100 = Max RADS.

		// Get current RADS value and return it to caller.
		CurrentValue = StatsGetValue(3);
		// Divide RADS value to fall between 0 - 100 range instead of 1000.
		CurrentValue = CurrentValue / 10;
		// If current value is negative, set to ZERO.
		if (CurrentValue < 0) {
			return 0;
		}
		// If RADS is above 100, set to 100.
		if (CurrentValue > 100) {
			return 100;
		}
		// Set RADS value.
		else {
			return CurrentValue;
		}
	}

	//////////////// -WEIGHT- ////////////////
	else if (StatsValue == 4) {
		// WEIGHT difference = CurrentWeight difference to max WEIGHT. Not calculated when difference is over 100.
		// In GameSense Interface: 0 = Over encumbered, 100+ = Plenty left.

		// Get current WEIGHT difference and return it to caller.
		CurrentValue = CarryWeightGetCurrentValue();
		CurrentValue2 = StatsGetValue(4); // Get current max WEIGHT value.
		CurrentValueDiff = abs(CurrentValue - CurrentValue2);
		// If current values are negative, set to ZERO.
		if (CurrentValue < 0 || CurrentValue2 < 0) {
			return 0;
		}
		// If current weight exceeds max WEIGHT, set to ZERO.
		if (CurrentValue > CurrentValue2) {
			return 0;
		}
		// If difference is over 100, set to 100. (lots of room left)
		else if (CurrentValueDiff > 100) {
			return 100;
		}
		// Return current WEIGHT difference value.
		else {
			return CurrentValueDiff;
		}
	}

	return -1;
}