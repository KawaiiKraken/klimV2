#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "ConfigFile.h"
#include "HelperFunctions.h"
#include "HotkeyManager.h"
#include "Limit.h"
#include "UserInterface.h"
#include "windivertFunctions.h"
#include <cstring>
#include <future>
#include <iostream>
#include <shellapi.h>
#include <tchar.h>
#include <vector>
#include <windows.h>

#pragma comment(lib, "ntdll.lib")

// this also isn't great because we're calling the wrong type of ctor?
std::atomic<Klim::Limit> limit_3074_dl = Klim::Limit(Klim::LimitType::limit_3074_dl);
std::atomic<Klim::Limit> limit_3074_ul = Klim::Limit(Klim::LimitType::limit_3074_ul);
std::atomic<Klim::Limit> limit_27k_dl = Klim::Limit(Klim::LimitType::limit_27k_dl);
std::atomic<Klim::Limit> limit_27k_ul = Klim::Limit(Klim::LimitType::limit_27k_ul);
std::atomic<Klim::Limit> limit_30k_dl = Klim::Limit(Klim::LimitType::limit_30k_dl);
std::atomic<Klim::Limit> limit_7500_dl = Klim::Limit(Klim::LimitType::limit_7500_dl);
std::atomic<Klim::Limit> limit_full_game = Klim::Limit(Klim::LimitType::limit_full_game);
std::atomic<Klim::Limit> suspend_game = Klim::Limit(Klim::LimitType::suspend_game);
std::atomic<Klim::Limit> exit_app = Klim::Limit(Klim::LimitType::exit_app);

const std::vector<std::atomic<Klim::Limit>*> limit_ptr_vector = { &limit_3074_dl, &limit_3074_ul, &limit_27k_dl, &limit_27k_ul, &limit_30k_dl, &limit_7500_dl, &limit_full_game, &suspend_game, &exit_app };

wchar_t path_to_config_file[MAX_PATH];
Klim::Settings settings;

std::shared_ptr<spdlog::logger> logger = Klim::Helper::LoggerInit("log.txt");
Klim::HotkeyManager hotkey_manager(limit_ptr_vector, &settings, logger);
Klim::UserInterface user_interface(limit_ptr_vector, path_to_config_file, &settings, logger);
Klim::WinDivertShit windivert_shit(limit_ptr_vector, &user_interface, logger);
Klim::UserInterface* Klim::UserInterface::ui_instance = &user_interface;
Klim::HotkeyManager* Klim::UserInterface::hk_instance = &hotkey_manager;
Klim::WinDivertShit* Klim::HotkeyManager::windivert_instance = &windivert_shit;
Klim::HotkeyManager* Klim::HotkeyManager::hk_instance = &hotkey_manager;
HHOOK Klim::HotkeyManager::keyboard_hook_handle = SetWindowsHookEx(WH_KEYBOARD_LL, hotkey_manager.StaticKeyboardEvent, NULL, NULL);
HHOOK Klim::HotkeyManager::mouseHook = SetWindowsHookEx(WH_MOUSE_LL, hotkey_manager.MouseProc, NULL, 0);

DWORD WINAPI RunGuiWrapper(LPVOID lpParam)
{
    Klim::UserInterface* ui_instance = static_cast<Klim::UserInterface*>(lpParam);
    ui_instance->RunGui();
    return 0;
}

int main()
{
    // dumb way to do it but wtv
    hotkey_manager.ui_instance = &user_interface;
    logger->info("init");

    if (!Klim::Helper::RunningAsAdmin())
    {
        MessageBox(nullptr, L"ERROR: not running as admin", L"ERROR", MB_ICONERROR | MB_DEFBUTTON2);
        return 0;
    }

    Klim::ConfigFile::SetPathToFileInExeDir(L"config.json", path_to_config_file);
    if (Klim::ConfigFile::FileExists(path_to_config_file))
    {
        Klim::ConfigFile::LoadConfig(limit_ptr_vector, path_to_config_file, &settings);
    }

    if (!settings.debug)
    {
        // hide the console if not running with debug on
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    user_interface.show_config = true;
    user_interface.show_overlay = false;

    DWORD dw_thread;
    logger->info("starting ui thread");
    CreateThread(nullptr, NULL, RunGuiWrapper, &user_interface, NULL, &dw_thread);

    logger->info("starting hotkey thread");


    hotkey_manager.HotkeyThread();

    return 1; // this should never be reached
}
