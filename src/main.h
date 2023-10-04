#pragma once
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
char hotkey_exitapp, modkey_exitapp;
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
COLORREF colorDefault, colorOff, colorOn;