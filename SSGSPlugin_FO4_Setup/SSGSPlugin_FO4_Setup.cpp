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

// Has to be after curl or massive compile error.
#include <windows.h>
#include <Shlwapi.h>
#include "Shlobj.h"

// Defined functions.
int PostJson(StringBuffer& Buffer, char* Address);

#define corePropsJsonBufferSize MAX_PATH
#define SSGS_ServerAddressBufferSize 64
#define INFO_NO_ARGUMENTS -1
#define ERR_COREPROPS_NOT_FOUND -2

// Data we need across multiple functions here.
char SSGS_corePropsJsonPath[MAX_PATH]; // SteelSeries Engine 3 ProgramData path.
char SSGS_corePropsJsonBuffer[corePropsJsonBufferSize]; // SteelSeries Engine 3 coreProps.json buffer.
char SSGS_ServerAddress[SSGS_ServerAddressBufferSize]; // SteelSeries Engine 3 GameSense server address.
char SSGS_ServerAddress_GameEvent[SSGS_ServerAddressBufferSize]; // Current game data gets sent here.
char SSGS_ServerAddress_GameMetadata[SSGS_ServerAddressBufferSize]; // Inform the interface what game you have.
char SSGS_ServerAddress_RegisterGameEvent[SSGS_ServerAddressBufferSize]; // Inform the interface what values of said game can be customised.
char SSGS_ServerAddress_RemoveGameEvent[SSGS_ServerAddressBufferSize]; // Inform the interface that X event is to be removed.
char SSGS_ServerAddress_RemoveGame[SSGS_ServerAddressBufferSize]; // Inform the interface that X game is to be removed.

int _tmain(int argc, char *argv[])
{
	int AddToSSGS();
	int RemoveFromSSGS();
	LPCTSTR RegisterGameArg = L"/add";
	LPCTSTR UnregisterGameArg = L"/remove";
	LPCTSTR CommandLineArgs = NULL;

	// Exit if no arguments found.
	if (argc >= 2) {
		// Setup args for processing.
		CommandLineArgs = GetCommandLine();
		PathGetArgs(CommandLineArgs);
	}
	else {
		return INFO_NO_ARGUMENTS;
	}

	// Get path to SteelSeries Engine 3's coreProps.json and grab the address from it.
	GetEnvironmentVariableA("PROGRAMDATA", SSGS_corePropsJsonPath, MAX_PATH);
	strcat_s(SSGS_corePropsJsonPath, MAX_PATH, "\\SteelSeries\\SteelSeries Engine 3\\coreProps.json");
	// Grab address.
	FILE* fp;
	if (fopen_s(&fp, SSGS_corePropsJsonPath, "rb") != 0) {
		// No coreProps.json?!
		return ERR_COREPROPS_NOT_FOUND;
	}
	FileReadStream is(fp, SSGS_corePropsJsonBuffer, corePropsJsonBufferSize);
	Document d;
	d.ParseStream(is);
	strcpy_s(SSGS_ServerAddress, SSGS_ServerAddressBufferSize, d["address"].GetString());
	fclose(fp);

	// Setup special addresses.
	// Game event.
	strcpy_s(SSGS_ServerAddress_GameEvent, SSGS_ServerAddressBufferSize, SSGS_ServerAddress);
	strcat_s(SSGS_ServerAddress_GameEvent, SSGS_ServerAddressBufferSize, "/game_event");
	// Game Metadata.
	strcpy_s(SSGS_ServerAddress_GameMetadata, SSGS_ServerAddressBufferSize, SSGS_ServerAddress);
	strcat_s(SSGS_ServerAddress_GameMetadata, SSGS_ServerAddressBufferSize, "/game_metadata");
	// Register Game Events.
	strcpy_s(SSGS_ServerAddress_RegisterGameEvent, SSGS_ServerAddressBufferSize, SSGS_ServerAddress);
	strcat_s(SSGS_ServerAddress_RegisterGameEvent, SSGS_ServerAddressBufferSize, "/register_game_event");
	// Remove Game event.
	strcpy_s(SSGS_ServerAddress_RemoveGameEvent, SSGS_ServerAddressBufferSize, SSGS_ServerAddress);
	strcat_s(SSGS_ServerAddress_RemoveGameEvent, SSGS_ServerAddressBufferSize, "/remove_game_event");
	// Remove Game.
	strcpy_s(SSGS_ServerAddress_RemoveGame, SSGS_ServerAddressBufferSize, SSGS_ServerAddress);
	strcat_s(SSGS_ServerAddress_RemoveGame, SSGS_ServerAddressBufferSize, "/remove_game");

	// AddGameArg.
	if (StrStrI(CommandLineArgs, RegisterGameArg) != NULL) {
		cout << "\nAdding Fallout 4 to SteelSeries Engine 3 GameSense.\n\n";
		AddToSSGS();
		cout << "\n\nFallout 4 added to SteelSeries Engine 3 GameSense & Config in My Games\\Fallout4 created.\n";
	}
	// RemoveGameArg.
	else if (StrStrI(CommandLineArgs, UnregisterGameArg) != NULL) {
		cout << "\n\nRemoving Fallout 4 from SteelSeries Engine 3 GameSense.\n\n";
		RemoveFromSSGS();
		cout << "\n\nFallout 4 removed from SteelSeries Engine 3 GameSense & Config in My Games\\Fallout4 deleted.\n";
	}

    return 0;
}

// Adds the game to SteelSeries Engine 3 GameSense.
int AddToSSGS() {

	//////////////// -GameDataJson- ////////////////

	// Data.
	const char* GameDataJson = "{\"game\":\"FALLOUT4\",\"event\":\"HEALTH\",\"data\":{\"value\":0}}";
	const char* GameData2Json = "{\"game\":\"FALLOUT4\",\"event\":\"STAMINA\",\"data\":{\"value\":0}}";
	const char* GameData3Json = "{\"game\":\"FALLOUT4\",\"event\":\"RADS\",\"data\":{\"value\":0}}";
	const char* GameData4Json = "{\"game\":\"FALLOUT4\",\"event\":\"WEIGHT\",\"data\":{\"value\":0}}";

	// Parsed JSON.
	Document GameJsonParsed;
	// Processed JSON ready be posted.
	StringBuffer Buffer;

	//// -HEALTH- ////

	// Parse.
	GameJsonParsed.Parse(GameDataJson);
	// Insert default value.
	Value& STR = GameJsonParsed["data"]["value"];
	STR.SetInt(100); // 100 Health default.
					 // Format into buffer.
	Writer<StringBuffer>writer(Buffer);
	GameJsonParsed.Accept(writer);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_GameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -STAMINA- ////

	// Parse.
	GameJsonParsed.Parse(GameData2Json);
	// Insert default value.
	Value& STR2 = GameJsonParsed["data"]["value"];
	STR2.SetInt(100); // 100 Stamina default.
					  // Format into buffer.
	Writer<StringBuffer>writer2(Buffer);
	GameJsonParsed.Accept(writer2);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_GameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -RADS- ////

	// Parse.
	GameJsonParsed.Parse(GameData3Json);
	// Insert default value.
	Value& STR3 = GameJsonParsed["data"]["value"];
	STR3.SetInt(0); // 0 RADS default.
					// Format into buffer.
	Writer<StringBuffer>writer3(Buffer);
	GameJsonParsed.Accept(writer3);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_GameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -WEIGHT- ////

	// Parse.
	GameJsonParsed.Parse(GameData4Json);
	// Insert default value.
	Value& STR4 = GameJsonParsed["data"]["value"];
	STR4.SetInt(100); // 100 WEIGHT default.
					  // Format into buffer.
	Writer<StringBuffer>writer4(Buffer);
	GameJsonParsed.Accept(writer4);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_GameEvent);
	// Cleanup.
	Buffer.Clear();

	//////////////// -GameInterfaceJson- ////////////////

	// Data.
	const char* GameInterfaceJson = "{\"game\":\"FALLOUT4\",\"game_display_name\":\"Fallout 4\",\"icon_color_id\":5}";

	//// -Interface- ////

	// Parse.
	GameJsonParsed.Parse(GameInterfaceJson);
	// Insert default value.
	Writer<StringBuffer>writer5(Buffer);
	GameJsonParsed.Accept(writer5);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_GameMetadata);
	// Cleanup.
	Buffer.Clear();

	//////////////// -GameInterfaceEventsJson- ////////////////

	// Data.
	const char* GameInterfaceEventsJson = "{\"game\":\"FALLOUT4\",\"event\":\"HEALTH\",\"min_value\":0,\"max_value\":100,\"icon_id\":1}";
	const char* GameInterfaceEvents2Json = "{\"game\":\"FALLOUT4\",\"event\":\"STAMINA\",\"min_value\":0,\"max_value\":100,\"icon_id\":11}";
	const char* GameInterfaceEvents3Json = "{\"game\":\"FALLOUT4\",\"event\":\"RADS\",\"min_value\":0,\"max_value\":100,\"icon_id\":6}";
	const char* GameInterfaceEvents4Json = "{\"game\":\"FALLOUT4\",\"event\":\"WEIGHT\",\"min_value\":0,\"max_value\":100,\"icon_id\":17}";

	//// -HEALTH- ////

	// Parse.
	GameJsonParsed.Parse(GameInterfaceEventsJson);
	// Insert default value.
	Writer<StringBuffer>writer6(Buffer);
	GameJsonParsed.Accept(writer6);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RegisterGameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -STAMINA- ////

	// Parse.
	GameJsonParsed.Parse(GameInterfaceEvents2Json);
	// Insert default value.
	Writer<StringBuffer>writer7(Buffer);
	GameJsonParsed.Accept(writer7);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RegisterGameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -RADS- ////

	// Parse.
	GameJsonParsed.Parse(GameInterfaceEvents3Json);
	// Insert default value.
	Writer<StringBuffer>writer8(Buffer);
	GameJsonParsed.Accept(writer8);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RegisterGameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -WEIGHT- ////

	// Parse.
	GameJsonParsed.Parse(GameInterfaceEvents4Json);
	// Insert default value.
	Writer<StringBuffer>writer9(Buffer);
	GameJsonParsed.Accept(writer9);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RegisterGameEvent);
	// Cleanup.
	Buffer.Clear();

	// Setup INI.
	PWSTR INIPathPTR = 0;
	TCHAR INIPath[MAX_PATH];

	// Get paths.
	SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, &INIPathPTR);
	_tcscpy_s(INIPath, MAX_PATH, INIPathPTR);
	_tcscat_s(INIPath, MAX_PATH, L"\\My Games\\Fallout4\\SSGSPlugin_Fallout4.ini");
	// Create fresh copy.
	HANDLE hFile = CreateFile(INIPath, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(hFile);
	// Write default settings.
	WritePrivateProfileString(L"General", L"bEnabled", L"1", INIPath);
	WritePrivateProfileString(L"General", L"iUpdateInterval", L"250", INIPath);
	WritePrivateProfileString(L"General", L"bConsoleLoggingEnabled", L"0", INIPath);

	return 0;
}

// Removes Fallout 4 from GS, and it's config from My Games.
int RemoveFromSSGS() {

	//////// Remove Game Events & Game Name from SSGS ////////

	//////////////// -RemoveGameEvent- ////////////////

	// Data.
	const char* GameDataJson = "{\"game\":\"FALLOUT4\",\"event\":\"HEALTH\"}";
	const char* GameData2Json = "{\"game\":\"FALLOUT4\",\"event\":\"STAMINA\"}";
	const char* GameData3Json = "{\"game\":\"FALLOUT4\",\"event\":\"RADS\"}";
	const char* GameData4Json = "{\"game\":\"FALLOUT4\",\"event\":\"WEIGHT\"}";

	// Parsed JSON.
	Document GameJsonParsed;
	// Processed JSON ready be posted.
	StringBuffer Buffer;

	//// -HEALTH- ////

	// Parse.
	GameJsonParsed.Parse(GameDataJson);
	// Format into buffer.
	Writer<StringBuffer>writer(Buffer);
	GameJsonParsed.Accept(writer);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RemoveGameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -STAMINA- ////

	// Parse.
	GameJsonParsed.Parse(GameData2Json);
	// Format into buffer.
	Writer<StringBuffer>writer2(Buffer);
	GameJsonParsed.Accept(writer2);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RemoveGameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -RADS- ////

	// Parse.
	GameJsonParsed.Parse(GameData3Json);
	// Format into buffer.
	Writer<StringBuffer>writer3(Buffer);
	GameJsonParsed.Accept(writer3);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RemoveGameEvent);
	// Cleanup.
	Buffer.Clear();

	//// -WEIGHT- ////

	// Parse.
	GameJsonParsed.Parse(GameData4Json);
	// Format into buffer.
	Writer<StringBuffer>writer4(Buffer);
	GameJsonParsed.Accept(writer4);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RemoveGameEvent);
	// Cleanup.
	Buffer.Clear();

	//////////////// -RemoveGame- ////////////////

	// Data.
	const char* GameNameJson = "{\"game\":\"FALLOUT4\"}";

	// Parse.
	GameJsonParsed.Parse(GameNameJson);
	// Format into buffer.
	Writer<StringBuffer>writer5(Buffer);
	GameJsonParsed.Accept(writer5);
	// Post.
	PostJson(Buffer, SSGS_ServerAddress_RemoveGame);
	// Cleanup.
	Buffer.Clear();

	// Setup INI.
	TCHAR INIPath[MAX_PATH];

	GetEnvironmentVariable(L"USERPROFILE", INIPath, MAX_PATH);
	_tcscat_s(INIPath, MAX_PATH, L"\\Documents\\My Games\\Fallout4\\SSGSPlugin_Fallout4.ini");
	DeleteFile(INIPath);

	return 0;
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