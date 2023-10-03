#define _WIN32_WINNT_WIN10 0x0A00 // Windows 10
#define PHNT_VERSION PHNT_THRESHOLD // Windows 10

// these 2 are for the phnt header files
#pragma clang diagnostic ignored "-Wpragma-pack"
#pragma clang diagnostic ignored "-Wmicrosoft-enum-forward-reference"

#pragma clang diagnostic ignored "-Wunused-variable"

#include "../phnt/phnt_windows.h"
#include "../phnt/phnt.h"
#include "../WinDivert/windivert.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <stdbool.h>
#include <iostream>
#include <wchar.h>
#include <shellapi.h>
#include <psapi.h>
#include "helperFunctions.h"
#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "advapi32.lib" )
#pragma comment( lib, "Psapi.lib" )
#pragma comment( lib, "ntdll.lib" )

//#pragma clang diagnostic ignored "-Wwritable-strings"
typedef UINT ( CALLBACK* LPFNDLLSTARTOVERLAY )( bool, int );
typedef UINT ( CALLBACK* LPFNDLLUPDATEOVERLAYLINE )( LPTSTR, int, COLORREF );
HINSTANCE hDLL, hDLL2;               
LPFNDLLSTARTOVERLAY lpfnDllStartOverlay;    
LPFNDLLUPDATEOVERLAYLINE lpfnDllUpdateOverlayLine;    
HHOOK hKeyboardHook;
HANDLE gDoneEvent;
HANDLE hThread = NULL;
HANDLE handle = NULL;
bool can_trigger_any_hotkey = TRUE;
wchar_t pathToIni[MAX_PATH];
wchar_t szFilePathSelf[MAX_PATH];
char hotkey_exitapp, hotkey_3074, hotkey_3074_UL, hotkey_27k, hotkey_27k_UL, hotkey_30k, hotkey_7k, hotkey_game, hotkey_suspend;
char modkey_exitapp, modkey_3074, modkey_3074_UL, modkey_27k, modkey_27k_UL, modkey_30k, modkey_7k, modkey_game, modkey_suspend;
bool state3074 = FALSE;
bool state3074_UL = FALSE;
bool state27k = FALSE; 
bool state27k_UL = FALSE; 
bool state30k = FALSE;
bool state7k = FALSE;
bool state_game = FALSE;
bool state_suspend = FALSE;
bool debug = FALSE;
int __cdecl Overlay( LPTSTR );   
void toggle3074();
void toggle3074_UL();
void toggle27k(); 
void toggle27k_UL(); 
void toggle30k(); 
void toggle7k(); 
void toggleSuspend(); 
void toggleGame(); 
void combinerules();
char myNetRules[1000];
const char *err_str;
INT16 priority = 1000;
COLORREF colorDefault, colorOff, colorOn;