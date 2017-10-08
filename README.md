## [PC] [WIPz] SteelSeries GameSense Plugin - Fallout 4

**What is this?**

It's a plugin I made that allows SteelSeries users to use GameSense with Fallout 4.

**What's in the download?**

- Plugin folder which contains the plugin itself along with its dll loader.
- Setup folder which contains the setup with bat files to setup SteelSeries GameSense before you go into game.
- Copy of this thread in a readme & licenses.

**How to install**

1. Install my Dll Loader manually. (You will be prompted to install prior to download)
2. Install this mod via NMM or drop the plugins folder in your games data folder.
3. Play!

**How to uninstall**

1. Uninstall via NMM or manually delete the files.

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

**Compile Notes**

- HJWasm must be setup before opening the project.
- Requisites: PlayerStatsAPI_FO4, and the last three mentioned in the credits.

**NOTES**

- This plugin retrieves players stats by hooking into Fallout4.exe during runtime.
-- The hook is proving to be reliable so far but is not the best way (currently the only way I know how). So until a better method comes along for retrieving stats, this will be forever marked as WIPz.
- Only zones are supported so far, due to me only owning an APEX keyboard.
- Your devices will flash red (or not, depending on your setup) when loading up Fallout 4 and when loading a game because Fallout4.exe writes random data to the buffers my program reads from during that time. This is normal.
- Written in C++/ASM (Visual Studio 2015 & HJWasm).
- The Dll loader can actually be used to load any dll into Fallout 4.

**CREDITS**

Bethesda for Fallout 4. (https://store.steampowered.com/app/377160/)

Microsoft for Visual Studio 2015. (https://www.visualstudio.com/)

The HJWasm devs. (https://github.com/Terraspace/HJWasm)

The cURL devs. (https://curl.haxx.se/)

The cURLcpp devs. (https://github.com/JosephP91/curlcpp)

The rapidjson devs. (https://github.com/miloyip/rapidjson)
