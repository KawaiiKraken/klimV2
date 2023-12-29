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
#include "imgui.h"
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
#include "UserInterface.h"
#include "imgui.h"
#include "helperFunctions.h"
#include "Limit.h"
#include "ConfigFile.h"

#pragma comment( lib, "ntdll.lib" )


void MessageLoop();
void SetPathToConfigFile(wchar_t* configFileName);
int __cdecl Overlay( LPTSTR );   
DWORD WINAPI HotkeyThread( LPVOID lpParm );
__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam );

HHOOK hKeyboardHook;
bool debug = FALSE;

std::atomic<limit> lim_3074 = limit("3074", " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)");
std::atomic<limit> lim_3074_ul = limit("3074UL", " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)");
std::atomic<limit> lim_27k = limit("27k", " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)");
std::atomic<limit> lim_27k_ul = limit("27kUL", " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)");
std::atomic<limit> lim_30k = limit("30k", " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)");
std::atomic<limit> lim_7k = limit("7k", " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)");
std::atomic<limit> lim_game("game"); 
std::atomic<limit> suspend("suspend"); 
std::atomic<limit> exitapp("exitapp"); 
const std::vector<std::atomic<limit>*> limit_ptr_vector = { &lim_3074, &lim_3074_ul, &lim_27k, &lim_27k_ul, &lim_30k, &lim_7k, &lim_game, &suspend, &exitapp };

typedef void (*KeyboardEventCallback)(int, bool);
std::vector<int> currently_pressed_keys;

char combined_windivert_rules[1000];
wchar_t path_to_config_file[MAX_PATH];
Settings settings;

HotkeyManager hotkeyManager(limit_ptr_vector);
UserInterface userInterface(limit_ptr_vector, path_to_config_file, &settings);
UserInterface* UserInterface::instance = &userInterface;
HotkeyManager* UserInterface::hotkeyInstance = &hotkeyManager;



DWORD WINAPI run_gui_wrapper(LPVOID lpParam) {
    UserInterface* uiinstance = static_cast<UserInterface*>(lpParam);
    uiinstance->run_gui();
    return 0;
}


int __cdecl main( int argc, char** argv ){
    if ( argv[1] != NULL ){
        if ( ( strcmp( argv[1], "--debug" ) == 0 ) ){
            printf( "debug: true\n" );
            debug = true;
        } else {
            printf( "error: invalid argument...\n" );
            printf( "options:\n" );
            printf( "    --debug     prevents console hiding and enables hotkey trigger outside of destiny 2.\n" );
            return 0;
        }
    } else {
        ShowWindow( GetConsoleWindow(), SW_HIDE );
    }

    if ( !Helper::RunningAsAdmin() ){
        MessageBox( NULL, ( LPCWSTR )L"ERROR: not running as admin", ( LPCWSTR )L"ERROR", MB_ICONERROR | MB_DEFBUTTON2 );
        return 0;
    }
    
    ConfigFile::SetPathToConfigFile( ( wchar_t* )L"config.json", path_to_config_file);
    if (ConfigFile::FileExists(path_to_config_file)) {
        ConfigFile::LoadConfig( limit_ptr_vector, path_to_config_file, &settings);
    }



    userInterface.show_config = true;
    userInterface.show_overlay= false;
    DWORD dwThread;
    printf( "starting ui thread\n" );
    CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )run_gui_wrapper, &userInterface, NULL, &dwThread );


    printf( "starting hotkey thread\n" );

    HANDLE hHotkeyThread = CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )HotkeyThread, ( LPVOID )NULL, NULL, &dwThread );

    if ( hHotkeyThread ){
        return WaitForSingleObject( hHotkeyThread, INFINITE );
    }
    else {
        if (hHotkeyThread != 0) {
            CloseHandle( hHotkeyThread );
        }
        return 1;
    }
}


__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam ){
    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYUP ) || ( wParam == WM_KEYUP ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =  *( ( KBDLLHOOKSTRUCT* )lParam );

        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);


		// Check if the vector contains the target element
		auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it != currently_pressed_keys.end()) {
            currently_pressed_keys.erase(it);
		}
        HotkeyManager::UnTriggerHotkeys(limit_ptr_vector, currently_pressed_keys);
    }

    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYDOWN ) || ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *( ( KBDLLHOOKSTRUCT* ) lParam );

        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);

		auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it == currently_pressed_keys.end()) {
            currently_pressed_keys.push_back(key);
		}
        if (!userInterface.show_config) {
            HotkeyManager::TriggerHotkeys(limit_ptr_vector, currently_pressed_keys, debug, settings, combined_windivert_rules);
        }
    }
    return CallNextHookEx( hKeyboardHook, nCode, wParam, lParam );
}


void MessageLoop(){
    MSG message;
    while ( GetMessage( &message, NULL, 0, 0 ) ){
        TranslateMessage( &message );
        DispatchMessage( &message );
    }
}


DWORD WINAPI HotkeyThread( LPVOID lpParam ){
    HINSTANCE hInstance = GetModuleHandle( NULL );
    if ( !hInstance ){
        hInstance = LoadLibrary( ( LPCWSTR )lpParam );
    }
    if ( !hInstance ){ 
        return 1; 
    }
    hKeyboardHook = SetWindowsHookEx ( WH_KEYBOARD_LL, ( HOOKPROC ) KeyboardEvent, hInstance, NULL );
    MessageLoop();
    UnhookWindowsHookEx( hKeyboardHook );
    return 0;
}

