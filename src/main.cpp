#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "ConfigFile.h"
#include "HelperFunctions.h"
#include "HotkeyManager.h"
#include "Limit.h"
#include "UserInterface.h"
// #include "imgui.h"
// #include <chrono>
// #include <condition_variable>
#include <future>
#include <iostream>
#include <shellapi.h>
// #include <stdbool.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <cstring>
#include <tchar.h>
#include <vector>
// #include <cwchar>
#include <windows.h>

#pragma comment(lib, "ntdll.lib")

void MessageLoop();
void SetPathToConfigFile(wchar_t* config_file_name);
int __cdecl Overlay(LPTSTR);
DWORD WINAPI HotkeyThread(LPVOID lp_Param);
__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int n_code, WPARAM w_param, LPARAM l_param);

HHOOK keyboard_hook_handle;
bool debug = FALSE;

// this also isn't great because we're calling the wrong type of ctor
std::atomic<Klim::Limit> limit_3074_dl = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::limit_3074_dl), " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)");
std::atomic<Klim::Limit> limit_3074_ul = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::limit_3074_ul), " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)");
std::atomic<Klim::Limit> limit_27k_dl = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::limit_27k_dl), " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)");
std::atomic<Klim::Limit> limit_27k_ul = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::limit_27k_ul), " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)");
std::atomic<Klim::Limit> limit_30k = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::limit_30k_dl), " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)");
std::atomic<Klim::Limit> limit_7k = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::limit_7500_dl), " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)");
std::atomic<Klim::Limit> limit_full_game = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::limit_full_game));
std::atomic<Klim::Limit> suspend = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::suspend_game));
std::atomic<Klim::Limit> exit_app = Klim::Limit(Klim::Limit::TypeToString(Klim::LimitType::exit_app));

const std::vector<std::atomic<Klim::Limit>*> limit_ptr_vector = { &limit_3074_dl, &limit_3074_ul, &limit_27k_dl, &limit_27k_ul, &limit_30k, &limit_7k, &limit_full_game, &suspend, &exit_app };

typedef void (*KeyboardEventCallback)(int, bool);
std::vector<int> currently_pressed_keys;

char combined_windivert_rules[1000];
wchar_t path_to_config_file[MAX_PATH];
Klim::Settings settings;

Klim::UserInterface user_interface(limit_ptr_vector, path_to_config_file, &settings);
Klim::HotkeyManager hotkey_manager(limit_ptr_vector, &user_interface);
Klim::UserInterface* Klim::UserInterface::ui_instance = &user_interface;
Klim::HotkeyManager* Klim::UserInterface::hk_instance = &hotkey_manager;

DWORD WINAPI RunGuiWrapper(LPVOID lpParam)
{
    Klim::UserInterface* ui_instance = static_cast<Klim::UserInterface*>(lpParam);
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

    if (!Klim::Helper::RunningAsAdmin())
    {
        MessageBox(nullptr, L"ERROR: not running as admin", L"ERROR", MB_ICONERROR | MB_DEFBUTTON2);
        return 0;
    }

    Klim::ConfigFile::SetPathToConfigFile(L"config.json", path_to_config_file);
    if (Klim::ConfigFile::FileExists(path_to_config_file))
    {
        Klim::ConfigFile::LoadConfig(limit_ptr_vector, path_to_config_file, &settings);
    }

    user_interface.show_config = true;
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

__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int n_code, WPARAM w_param, LPARAM l_param)
{
    if (n_code == HC_ACTION && (w_param == WM_SYSKEYUP || w_param == WM_KEYUP))
    {
        const KBDLLHOOKSTRUCT hooked_key = *reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);

        const int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
        const std::vector<int>::iterator iterator = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (iterator != currently_pressed_keys.end())
        {
            currently_pressed_keys.erase(iterator);
        }

        hotkey_manager.UnTriggerHotkeys(limit_ptr_vector, currently_pressed_keys);
    }

    if (n_code == HC_ACTION && ((w_param == WM_SYSKEYDOWN) || (w_param == WM_KEYDOWN)))
    {
        const KBDLLHOOKSTRUCT hooked_key = *reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);

        const int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
        const std::vector<int>::iterator iterator = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (iterator == currently_pressed_keys.end())
        {
            currently_pressed_keys.push_back(key);
        }
        if (!user_interface.show_config)
        {
            hotkey_manager.TriggerHotkeys(limit_ptr_vector, currently_pressed_keys, debug, combined_windivert_rules);
        }
    }

    return CallNextHookEx(keyboard_hook_handle, n_code, w_param, l_param);
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

DWORD WINAPI HotkeyThread(LPVOID lp_Param)
{
    HINSTANCE h_instance = GetModuleHandle(nullptr);
    if (!h_instance)
    {
        h_instance = LoadLibrary(static_cast<LPCWSTR>(lp_Param));
    }
    if (!h_instance)
    {
        return 1;
    }

    keyboard_hook_handle = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardEvent, h_instance, NULL);
    MessageLoop();
    UnhookWindowsHookEx(keyboard_hook_handle);
    return 0;
}
