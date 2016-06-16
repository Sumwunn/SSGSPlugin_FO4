##[PC] [WIPz] SteelSeries GameSense Plugin - Fallout 4

**What is this?**

It's a plugin I made that allows SteelSeries users to use GameSense with Fallout 4.

**What's in the download?**

- Plugin folder which contains the plugin itself along with its dll loader.
- Setup folder which contains the setup with bat files to setup SteelSeries GameSense before you go into game.
- Copy of this thread in a readme & licenses.

**How to install**

1. First, make sure you have the latest SteelSeries Engine 3 installed. (http://steelseries.com/engine)

2. Inside the Setup folder, run ADD.bat. This will setup Fallout 4 with GameSense and create SSGSPlugin_Fallout4.ini in My Games/Fallout4, after that you can customise your devices for Fallout 4.

3. Inside the Plugin folder, copy DllLoader.txt, X3DAudio1_7.dll & SSGSPlugin_Fallout4.dll to Fallout 4's root folder.

4. Play! (If it doesn't work, check your firewall settings and make sure 127.0.0.1 is allowed and all ports for Fallout4.exe (SteelSeries Engine randomises ports on startup)

**How to uninstall**

1. Delete DllLoader.txt, X3DAudio1_7.dll & SSGSPlugin_Fallout4.dll from Fallout 4's root folder.
2. Run REMOVE.bat inside the Setup folder, this will remove Fallout 4 from GameSense and delete it's config from My Games/Fallout4.

**Settings**

Have a look inside SSGSPlugin_Fallout4.ini (My Games/Fallout4). It contains settings for the plugin such as:
- bEnabled (is just a simple on 'n off switch).
- iUpdateInterval (talking to SteelSeries Engine, in milliseconds).
- bConsoleLoggingEnabled (opens up console window with interesting info!).

**Currently supported stats**

Health (In GameSense, 0 = Dead, 100 = Good)

Weight (In GameSense, 0 = Over encumbered, 100 = plenty of room)

Stamina (In GameSense, 0 = Tired, 100 = Good)

RADS (In GameSense, 0 = No RADS, 100 = Dead)

I hope to add XP one day but not having much luck finding a reliable hooking address.

**Discussion**: https://community.bethesda.net/message/127371

**NOTES**

- This plugin retrieves players stats by hooking into Fallout4.exe during runtime.
-- The hook is proving to be reliable so far but is not the best way (currently the only way I know how). So until a better method comes along for retrieving stats, this will be forever marked as WIPz.
- Only zones are supported so far, due to me only owning an APEX keyboard.
- Your devices will flash red (or not, depending on your setup) when loading up Fallout 4 and when loading a game because Fallout4.exe writes random data to the buffers my program reads from during that time. This is normal.
- Written in C++/ASM (Visual Studio 2015 & HJWasm).
- The Dll loader can actually be used to load any dll into Fallout 4 or the Creation Kit. (edit DllLoader.txt for Fallout4.exe & DllLoaderEditor.txt for CreationKit.exe)

**CREDITS**

Bethesda for Fallout 4. (https://store.steampowered.com/app/377160/)

Microsoft for Visual Studio 2015. (https://www.visualstudio.com/)

The HJWasm devs. (https://github.com/Terraspace/HJWasm)

The cURL devs. (https://curl.haxx.se/)

The cURLcpp devs. (https://github.com/JosephP91/curlcpp)

The rapidjson devs. (https://github.com/miloyip/rapidjson)
