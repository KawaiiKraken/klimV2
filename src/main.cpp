#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "ConfigFile.h"
#include "HelperFunctions.h"
#include "imgui.h"
#include <iostream>
#include <shellapi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <wchar.h>
#include <windows.h>
#include "ConfigFile.h"
#include "HotkeyManager.h"
#include "Limit.h"
#include "UserInterface.h"
#include "imgui.h"
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <windows.h>

#pragma comment(lib, "ntdll.lib")

void MessageLoop();
void SetPathToConfigFile(wchar_t* configFileName);
int __cdecl Overlay(LPTSTR);
DWORD WINAPI HotkeyThread(LPVOID lpParm);
__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam);

HHOOK hKeyboardHook;
bool debug = FALSE;

std::atomic<Limit> lim_3074    = Limit("3074", " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)");
std::atomic<Limit> lim_3074_ul = Limit("3074UL", " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)");
std::atomic<Limit> lim_27k     = Limit("27k", " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)");
std::atomic<Limit> lim_27k_ul  = Limit("27kUL", " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)");
std::atomic<Limit> lim_30k     = Limit("30k", " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)");
std::atomic<Limit> lim_7k      = Limit("7k", " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)");
std::atomic<Limit> lim_game("game");
std::atomic<Limit> suspend("suspend");
std::atomic<Limit> exitapp("exitapp");
const std::vector<std::atomic<Limit>*> limit_ptr_vector = { &lim_3074, &lim_3074_ul, &lim_27k, &lim_27k_ul, &lim_30k, &lim_7k, &lim_game, &suspend, &exitapp };

typedef void (*KeyboardEventCallback)(int, bool);
std::vector<int> currently_pressed_keys;

char combined_windivert_rules[1000];
wchar_t path_to_config_file[MAX_PATH];
Settings settings;

HotkeyManager hotkeyManager(limit_ptr_vector);
UserInterface userInterface(limit_ptr_vector, path_to_config_file, &settings);
UserInterface* UserInterface::ui_instance       = &userInterface;
HotkeyManager* UserInterface::hk_instance = &hotkeyManager;


DWORD WINAPI run_gui_wrapper(LPVOID lpParam)
{
    UserInterface* uiinstance = static_cast<UserInterface*>(lpParam);
    uiinstance->RunGui();
    return 0;
}


int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        if ((strcmp(argv[1], "--debug") == 0)) 
        {
            std::cout << "debug: true" << std::endl;
            debug = true;
        } else 
        {
            std::cout << "error: invalid argument..." << std::endl
                      << "options:" << std::endl
                      << "    --debug     prevents console hiding and enables hotkey trigger outside of destiny 2." << std::endl;
            return 0;
        }
    }
    else 
    {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    if (!Helper::RunningAsAdmin()) 
    {
        MessageBox(NULL, ( LPCWSTR )L"ERROR: not running as admin", ( LPCWSTR )L"ERROR", MB_ICONERROR | MB_DEFBUTTON2);
        return 0;
    }

    ConfigFile::SetPathToConfigFile(( wchar_t* )L"config.json", path_to_config_file);
    if (ConfigFile::FileExists(path_to_config_file)) 
    {
        ConfigFile::LoadConfig(limit_ptr_vector, path_to_config_file, &settings);
    }

    userInterface.show_config  = true;
    userInterface.show_overlay = false;
    DWORD dwThread;
    std::cout << "starting ui thread" << std::endl;
    CreateThread(NULL, NULL, ( LPTHREAD_START_ROUTINE )run_gui_wrapper, &userInterface, NULL, &dwThread);

    std::cout << "starting hotkey thread" << std::endl;

    HANDLE hHotkeyThread = CreateThread(NULL, NULL, ( LPTHREAD_START_ROUTINE )HotkeyThread, ( LPVOID )NULL, NULL, &dwThread);

    if (hHotkeyThread) 
    {
        return WaitForSingleObject(hHotkeyThread, INFINITE);
    }

    if (hHotkeyThread != 0) 
    {
        CloseHandle(hHotkeyThread);
    }
    return 1;
}


__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
    if ((nCode == HC_ACTION) && ((wParam == WM_SYSKEYUP) || (wParam == WM_KEYUP)))
    {
        KBDLLHOOKSTRUCT hooked_key = *(( KBDLLHOOKSTRUCT* )lParam);

        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);


        // Check if the vector contains the target element
        auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it != currently_pressed_keys.end()) 
        {
            currently_pressed_keys.erase(it);
        }
        HotkeyManager::UnTriggerHotkeys(limit_ptr_vector, currently_pressed_keys);
    }

    if ((nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN))) 
    {
        KBDLLHOOKSTRUCT hooked_key = *(( KBDLLHOOKSTRUCT* )lParam);

        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);

        auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it == currently_pressed_keys.end()) 
        {
            currently_pressed_keys.push_back(key);
        }
        if (!userInterface.show_config) 
        {
            HotkeyManager::TriggerHotkeys(limit_ptr_vector, currently_pressed_keys, debug, combined_windivert_rules);
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}


void MessageLoop()
{
    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) 
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}


DWORD WINAPI HotkeyThread(LPVOID lpParam)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!hInstance) 
    {
        hInstance = LoadLibrary(( LPCWSTR )lpParam);
    }
    if (!hInstance) 
    {
        return 1;
    }
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, ( HOOKPROC )KeyboardEvent, hInstance, NULL);
    MessageLoop();
    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}
