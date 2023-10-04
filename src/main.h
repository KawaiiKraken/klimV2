#pragma once
#define _WIN32_WINNT_WIN10 0x0A00 // Windows 10
#define PHNT_VERSION PHNT_THRESHOLD // Windows 10

// these 2 are for the phnt header files
#pragma clang diagnostic ignored "-Wpragma-pack"
#pragma clang diagnostic ignored "-Wmicrosoft-enum-forward-reference"

#pragma clang diagnostic ignored "-Wunused-variable"

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
#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "advapi32.lib" )
#pragma comment( lib, "Psapi.lib" )
#pragma comment( lib, "ntdll.lib" )

//#pragma clang diagnostic ignored "-Wwritable-strings"
HINSTANCE hDLL, hDLL2;               
HHOOK hKeyboardHook;
HANDLE gDoneEvent;
HANDLE hThread = NULL;
HANDLE handle = NULL;
bool can_trigger_any_hotkey = TRUE;
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