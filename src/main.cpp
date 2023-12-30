#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

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
#include "imgui.h"
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <windows.h>

#include "ConfigFile.h"
#include "HotkeyManager.h"
#include "Limit.h"
#include "UserInterface.h"
#include "HelperFunctions.h"

#pragma comment(lib, "ntdll.lib")

void MessageLoop();
void SetPathToConfigFile(wchar_t* config_file_name);
int __cdecl Overlay(LPTSTR);
DWORD WINAPI HotkeyThread(LPVOID lpParm);
__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam);

HHOOK hKeyboardHook;
bool debug = FALSE;

std::atomic<Limit> limit_3074_dl = Limit("3074", " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)");
std::atomic<Limit> limit_3074_ul = Limit("3074UL", " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)");
std::atomic<Limit> limit_27k_dl = Limit("27k", " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)");
std::atomic<Limit> limit_27k_ul = Limit("27kUL", " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)");
std::atomic<Limit> limit_30k = Limit("30k", " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)");
std::atomic<Limit> limit_7k = Limit("7k", " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)");
std::atomic<Limit> lim_game("game");
std::atomic<Limit> suspend("suspend");
std::atomic<Limit> exit_app("exitapp");
const std::vector<std::atomic<Limit>*> limit_ptr_vector = { &limit_3074_dl, &limit_3074_ul, &limit_27k_dl, &limit_27k_ul, &limit_30k, &limit_7k, &lim_game, &suspend, &exit_app };

typedef void (*KeyboardEventCallback)(int, bool);
std::vector<int> currently_pressed_keys;

char combined_windivert_rules[1000];
wchar_t path_to_config_file[MAX_PATH];
Settings settings;

HotkeyManager hotkey_manager(limit_ptr_vector);
UserInterface user_interface(limit_ptr_vector, path_to_config_file, &settings);
UserInterface* UserInterface::ui_instance = &user_interface;
HotkeyManager* UserInterface::hk_instance = &hotkey_manager;

DWORD WINAPI RunGuiWrapper(LPVOID lpParam)
{
    UserInterface* ui_instance = static_cast<UserInterface*>(lpParam);
    ui_instance->RunGui();
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "--debug") == 0) 
        {
            std::cout << "debug: true\n";
            debug = true;
        }
        else 
        {
            std::cout << "error: invalid argument...\n"
                      << "options:\n"
                      << "    --debug     prevents console hiding and enables hotkey trigger outside of destiny 2.\n";
            return 0;
        }
    }
    else 
    {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    if (!Helper::RunningAsAdmin()) 
    {
        MessageBox(nullptr, L"ERROR: not running as admin", L"ERROR", MB_ICONERROR | MB_DEFBUTTON2);
        return 0;
    }

    ConfigFile::SetPathToConfigFile(L"config.json", path_to_config_file);
    if (ConfigFile::FileExists(path_to_config_file)) 
    {
        ConfigFile::LoadConfig(limit_ptr_vector, path_to_config_file, &settings);
    }

    user_interface.show_config  = true;
    user_interface.show_overlay = false;

    DWORD dw_thread;
    std::cout << "starting ui thread\n";
    CreateThread(nullptr, NULL, RunGuiWrapper, &user_interface, NULL, &dw_thread);

    std::cout << "starting hotkey thread\n";

    const HANDLE h_hotkey_thread = CreateThread(nullptr, NULL, HotkeyThread, nullptr, NULL, &dw_thread);

    if (h_hotkey_thread) 
    {
        return WaitForSingleObject(h_hotkey_thread, INFINITE);
    }

    if (h_hotkey_thread != nullptr) 
    {
        CloseHandle(h_hotkey_thread);
    }
    return 1;
}

__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_SYSKEYUP || wParam == WM_KEYUP))
    {
        const KBDLLHOOKSTRUCT hooked_key = *reinterpret_cast<KBDLLHOOKSTRUCT*>( lParam );

        const int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
        const std::vector<int>::iterator iterator = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (iterator != currently_pressed_keys.end()) 
        {
            currently_pressed_keys.erase(iterator);
        }

        HotkeyManager::UnTriggerHotkeys(limit_ptr_vector, currently_pressed_keys);
    }

    if (nCode == HC_ACTION && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN))) 
    {
        const KBDLLHOOKSTRUCT hooked_key = *reinterpret_cast<KBDLLHOOKSTRUCT*>( lParam );

        const int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
        const std::vector<int>::iterator iterator = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (iterator == currently_pressed_keys.end()) 
        {
            currently_pressed_keys.push_back(key);
        }
        if (!user_interface.show_config) 
        {
            HotkeyManager::TriggerHotkeys(limit_ptr_vector, currently_pressed_keys, debug, combined_windivert_rules);
        }
    }

    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

void MessageLoop()
{
    MSG message;
    while (GetMessage(&message, nullptr, 0, 0)) 
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

DWORD WINAPI HotkeyThread(LPVOID lpParam)
{
    HINSTANCE h_instance = GetModuleHandle(nullptr);
    if (!h_instance) 
    {
        h_instance = LoadLibrary(static_cast<LPCWSTR>( lpParam ));
    }
    if (!h_instance) 
    {
        return 1;
    }

    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardEvent, h_instance, NULL);
    MessageLoop();
    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}
