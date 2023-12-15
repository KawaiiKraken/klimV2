#pragma once
#define _WIN32_WINNT_WIN10 0x0A00 // Windows 10
#define PHNT_VERSION PHNT_THRESHOLD // Windows 10


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
#include "krekens_overlay.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "ConfigFile.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <future>
#include <chrono>
#include <condition_variable>
#include "HotkeyManager.h"

#pragma comment( lib, "ntdll.lib" )

HINSTANCE hDLL, hDLL2;               
HHOOK hKeyboardHook;
HANDLE gDoneEvent;
HANDLE hHotkeyThread = NULL;
HANDLE handle = NULL;
COLORREF color_default, color_off, color_on;
bool can_trigger_any_hotkey = TRUE;
bool debug = FALSE;

void MessageLoop();
void SetPathToConfigFile(wchar_t* configFileName);
int __cdecl Overlay( LPTSTR );   
DWORD WINAPI HotkeyThread( LPVOID lpParm );
__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam );