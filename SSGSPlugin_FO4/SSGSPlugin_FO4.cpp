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
#include <cerrno>
#include <string>
#include <iostream>

using namespace std;

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

using namespace rapidjson;

#include "curl_easy.h"
#include "curl_header.h"
#include "curl_pair.h"
#include "curl_exception.h"

using curl::curl_easy;
using curl::curl_header;
using curl::curl_pair;
using curl::curl_easy_exception;

// Has to be after curl or terrible compile error.
#include <windows.h>
#include <ShlObj.h>

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
// C++
DWORD WINAPI GameSenseDataSender(LPVOID lpParam);
int PostJson(StringBuffer& Buffer, char* Address); // JSSSSSSOOOOOONNNNNNNNNNNN.

// Data we need across multiple functions here.
#define corePropsJsonBufferSize MAX_PATH
#define SSGS_ServerAddressBufferSize 64
char SSGS_corePropsJsonPath[MAX_PATH]; // SteelSeries Engine 3 ProgramData path.
char SSGS_corePropsJsonBuffer[corePropsJsonBufferSize]; // SteelSeries Engine 3 coreProps.json buffer.
char SSGS_ServerAddress[SSGS_ServerAddressBufferSize]; // SteelSeries Engine 3 GameSense server address.
char SSGS_ServerAddress_GameEvent[SSGS_ServerAddressBufferSize]; // Current game data gets sent here.

// INI Settings.
int bEnabled = TRUE;
int iUpdateInterval = 250;
int bConsoleLoggingEnabled = FALSE;

FILE *pConsole = NULL;

// Handles.
HANDLE GameSenseDataSender_hThread = NULL;

int Setup() {

	//////// Setup Part 1 - INI ////////

	// Setup INI.
	PWSTR INIPathPTR = NULL;
	TCHAR INIPath[MAX_PATH];

	// Get paths.
	SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, &INIPathPTR);
	_tcscpy_s(INIPath, MAX_PATH, INIPathPTR);
	_tcscat_s(INIPath, MAX_PATH, L"\\My Games\\Fallout4\\SSGSPlugin_Fallout4.ini");
	bEnabled = GetPrivateProfileInt(L"General", L"bEnabled", TRUE, INIPath);
	iUpdateInterval = GetPrivateProfileInt(L"General", L"iUpdateInterval", 250, INIPath);
	bConsoleLoggingEnabled = GetPrivateProfileInt(L"General", L"bConsoleLoggingEnabled", FALSE, INIPath);

	// Am I enabled?
	if (bEnabled == FALSE) {
		// NOPE.
		return 0;
	}

	// Enable logging via console.
	if (bConsoleLoggingEnabled) {
		AllocConsole();
		SetConsoleTitle(L"SteelSeries GameSense Plugin Logging");
		freopen_s(&pConsole, "CONOUT$", "r+", stdout);
	}

	//////// Setup Part 2 - Addresses ////////

	// Get module of target to writes hooks to.
	HMODULE TargetModule = GetModuleHandle(NULL);

	//// Fallout 4 Version Check ////
	// Using the entry point rva from pe header as my verison check.
	int Result = GetEntryPointRVA(TargetModule);
	if (Result == 0x06BC3310) {
		// Fill in version specific data (v1.5.307.0).
		StatsPointerRetrieverRVA = (void*)0xE3DEE2;
		CurrentCarryWeightRVA = (void*)0x5719050;
		if (bConsoleLoggingEnabled) {
			cout << "Fallout 4 v1.5.307.0 detected." << endl;
		}
	}
#ifdef FO4_BETA_SUPPORT
	else if (Result == 0x0) {
		// Fill in version specific data (BETA).
		StatsPointerRetrieverRVA = (void*)0x0;
		CurrentCarryWeightRVA = (void*)0x0;
		if (bConsoleLoggingEnabled) {
			cout << "Fallout 4 v1.5.210.0 BETA detected." << endl;
		}
	}
#endif
	else {
		// Unsupported version.
		if (bConsoleLoggingEnabled) {
			cout << "Fallout 4 unsupported version detected. Only v1.5.307.0.0 & v1.5.210.0 BETA supported." << endl;
		}
		return 0;
	}

	// Get path to SteelSeries Engine 3's coreProps.json and grab the address from it.
	GetEnvironmentVariableA("PROGRAMDATA", SSGS_corePropsJsonPath, MAX_PATH);
	strcat_s(SSGS_corePropsJsonPath, MAX_PATH, "\\SteelSeries\\SteelSeries Engine 3\\coreProps.json");
	// Grab address.
	FILE* fp;
	if (fopen_s(&fp, SSGS_corePropsJsonPath, "rb") != 0) {
		// No coreProps.json?!
		return 0;
	}
	FileReadStream is(fp, SSGS_corePropsJsonBuffer, corePropsJsonBufferSize);
	Document d;
	d.ParseStream(is);
	strcpy_s(SSGS_ServerAddress, SSGS_ServerAddressBufferSize, d["address"].GetString());
	fclose(fp);
	// Setup multiple addresses.
	// Game event.
	strcpy_s(SSGS_ServerAddress_GameEvent, SSGS_ServerAddressBufferSize, SSGS_ServerAddress);
	strcat_s(SSGS_ServerAddress_GameEvent, SSGS_ServerAddressBufferSize, "/game_event");

	//////// Setup Part 3 - Hooks ////////

	// Get the hook's addresses ready.
	StatsPointerRetrieverVA = ConvertRVAToVA(TargetModule, StatsPointerRetrieverRVA);
	CurrentCarryWeightVA = ConvertRVAToVA(TargetModule, CurrentCarryWeightRVA);

	// Redirect an address from that game that will give us the values we need. (and hopefully consistently)
	// Write Hook.
	HookWrite(StatsPointerRetrieverVA, StatsPointerRetriever, 1);

	// Setup threads.
	GameSenseDataSender_hThread = CreateThread(NULL, NULL, GameSenseDataSender, NULL, NULL, NULL);

	return 0;
}

// Keeps SteelSeries Engine 3 GameSense server updated with game data.
DWORD WINAPI GameSenseDataSender(LPVOID lpParam) {

	// Data.
	const char* GameDataJson = "{\"game\":\"FALLOUT4\",\"event\":\"HEALTH\",\"data\":{\"value\":0}}";
	const char* GameData2Json = "{\"game\":\"FALLOUT4\",\"event\":\"STAMINA\",\"data\":{\"value\":0}}";
	const char* GameData3Json = "{\"game\":\"FALLOUT4\",\"event\":\"RADS\",\"data\":{\"value\":0}}";
	const char* GameData4Json = "{\"game\":\"FALLOUT4\",\"event\":\"WEIGHT\",\"data\":{\"value\":0}}";

	// Parsed JSON.
	Document GameJsonParsed;
	Document GameJson2Parsed;
	Document GameJson3Parsed;
	Document GameJson4Parsed;
	// Processed JSON ready be posted.
	StringBuffer Buffer;
	// Current value.
	int CurrentValue;
	int CurrentValue2;
	int CurrentValueDiff;

	// Parse.
	GameJsonParsed.Parse(GameDataJson);
	GameJson2Parsed.Parse(GameData2Json);
	GameJson3Parsed.Parse(GameData3Json);
	GameJson4Parsed.Parse(GameData4Json);

	// Infinite loop.
	for (;;) {
		Sleep(iUpdateInterval);

		//////////////// -HEALTH- ////////////////

		// In GameSense Interface: 0 = Dead, 100 = Good.

		// Get current HEALTH value and send it to GameSense server.
		CurrentValue = StatsGetValue(1);
		// If current value is negative, set to ZERO.
		if (CurrentValue < 0) {
			Value& STR = GameJsonParsed["data"]["value"]; // Points to WEIGHT->Data->Value.
			STR.SetInt(0); // Set current WEIGHT.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJsonParsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// If health is above 100, set to 100.
		if (CurrentValue >= 100) {
			// Modify it by DOM.
			Value& STR = GameJsonParsed["data"]["value"]; // Points to Health->Data->Value.
			STR.SetInt(100); // Set current health.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJsonParsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// Set HEALTH value.
		else {
			// Modify it by DOM.
			Value& STR = GameJsonParsed["data"]["value"]; // Points to Health->Data->Value.
			STR.SetInt(CurrentValue); // Set current health.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJsonParsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}

		// Cleanup.
		Buffer.Clear();

		//////////////// -STAMINA- ////////////////

		// In GameSense Interface: 0 = None, 100 = Full.

		// Get current STAMINA value and send it to GameSense server.
		CurrentValue = StatsGetValue(2);
		// If current value is negative, set to ZERO.
		if (CurrentValue < 0) {
			Value& STR = GameJson2Parsed["data"]["value"]; // Points to WEIGHT->Data->Value.
			STR.SetInt(0); // Set current WEIGHT.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson2Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// If STAMINA is above 100, set to 100.
		if (CurrentValue >= 100) {
			// Modify it by DOM.
			Value& STR = GameJson2Parsed["data"]["value"]; // Points to STAMINA->Data->Value.
			STR.SetInt(100); // Set current STAMINA.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson2Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// Set STAMINA value.
		else {
			// 2. Modify it by DOM.
			Value& STR = GameJson2Parsed["data"]["value"]; // Points to STAMINA->Data->Value.
			STR.SetInt(CurrentValue); // Set current STAMINA.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson2Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}

		// Cleanup.
		Buffer.Clear();

		//////////////// -RADS- ////////////////

		// In GameSense Interface: 0 = No RADS, 100 = Max RADS.

		// Get current RADS value and send it to GameSense server.
		CurrentValue = StatsGetValue(3);
		// If current value is negative, set to ZERO.
		if (CurrentValue < 0) {
			Value& STR = GameJson3Parsed["data"]["value"]; // Points to WEIGHT->Data->Value.
			STR.SetInt(0); // Set current WEIGHT.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson3Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// If RADS is above 100, set to 100.
		if (CurrentValue >= 100) {
			CurrentValue = CurrentValue / 10; // Divide RADS value to fall between 0 - 100 range instead of 1000.
			Value& STR = GameJson3Parsed["data"]["value"]; // Points to RADS->Data->Value.
			STR.SetInt(CurrentValue); // Set current RADS.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson3Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// Set RADS value.
		else {
			CurrentValue = CurrentValue / 10; // Divide RADS value between 0 - 100 to fall between 0 - 10 range instead of 100.
			Value& STR = GameJson3Parsed["data"]["value"]; // Points to RADS->Data->Value.
			STR.SetInt(CurrentValue); // Set current RADS.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson3Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}

		// Cleanup.
		Buffer.Clear();

		//////////////// -WEIGHT- ////////////////

		// Weight difference = CurrentWeight difference to MaxWeight. Not calculated when difference is over 100.
		// In GameSense Interface: 0 = Over encumbered, 100+ = Plenty left.

		// Get current WEIGHT difference and send it to GameSense server.
		CurrentValue = CarryWeightGetCurrentValue();
		CurrentValue2 = StatsGetValue(4); // Get max weight.
		CurrentValueDiff = abs(CurrentValue - CurrentValue2);
		// If current values are negative, set to ZERO.
		if (CurrentValue < 0 || CurrentValue2 < 0) {
			Value& STR = GameJson4Parsed["data"]["value"]; // Points to WEIGHT->Data->Value.
			STR.SetInt(0); // Set current WEIGHT.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson4Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// If current weight exceeds max weight, set to ZERO.
		if (CurrentValue > CurrentValue2) {
			Value& STR = GameJson4Parsed["data"]["value"]; // Points to WEIGHT->Data->Value.
			STR.SetInt(0); // Set current WEIGHT.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson4Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// If difference is over 100, set to 100. (lots of room left)
		else if (CurrentValueDiff >= 100) {
			Value& STR = GameJson4Parsed["data"]["value"]; // Points to WEIGHT->Data->Value.
			STR.SetInt(100); // Set current WEIGHT.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson4Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}
		// Set current WEIGHT difference value.
		else {
			Value& STR = GameJson4Parsed["data"]["value"]; // Points to WEIGHT->Data->Value.
			STR.SetInt(CurrentValueDiff); // Set current WEIGHT difference.
			// Stringify the DOM.
			Writer<StringBuffer> writer(Buffer);
			GameJson4Parsed.Accept(writer);
			// Post.
			PostJson(Buffer, SSGS_ServerAddress_GameEvent);
		}

		// Cleanup.
		Buffer.Clear();
	}

	return NULL;
}

int PostJson(StringBuffer& Buffer, char* Address) {

	curl_easy easy;
	curl_header header;
	header.add("Content-Type: application/json");
	easy.add(curl_pair<CURLoption, curl_header>(CURLOPT_HTTPHEADER, header));
	easy.add(curl_pair<CURLoption, string>(CURLOPT_URL, Address));
	easy.add(curl_pair<CURLoption, string>(CURLOPT_POSTFIELDS, Buffer.GetString()));
	try {
		easy.perform();
	}
	catch (curl_easy_exception error) {
		error.print_traceback();
	}

	return 0;
}