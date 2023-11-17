#pragma once
#define _WIN32_WINNT_WIN10 0x0A00 // Windows 10
#define PHNT_VERSION PHNT_THRESHOLD // Windows 10


#pragma clang diagnostic ignored "-Wunused-variable"

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
COLORREF colorDefault, colorOff, colorOn;
bool can_trigger_any_hotkey = TRUE;
bool debug = FALSE;

void MessageLoop();
void setPathToConfigFile();
void setFilterRuleString();
int __cdecl Overlay( LPTSTR );   
DWORD WINAPI my_HotKey( LPVOID lpParm );
__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam );