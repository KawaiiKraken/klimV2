#pragma once

#include "Limit.h"
#include "helperFunctions.h"

namespace Klim
{
    class Settings;
    class UserInterface;
    class WinDivertShit;

    class HotkeyManager
    {
        public:
            HotkeyManager(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, Settings* settings, std::shared_ptr<spdlog::logger> logger);
            void TriggerHotkeys(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, std::vector<int> currently_pressed_keys, bool debug);
            void UnTriggerHotkeys(std::vector<std::atomic<Limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys);
            void OnTriggerHotkey(std::atomic<Limit>* limit_arg, bool debug, const std::vector<std::atomic<Limit>*>& limit_ptr_vector);
            void AsyncBindHotkey(int i);
            void KeyboardInputHandler(int key, bool is_key_down);
            int done = true;
            UserInterface* ui_instance;
            static WinDivertShit* windivert_instance;
            DWORD HotkeyThread();
            static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
            static __declspec(dllexport) LRESULT CALLBACK StaticKeyboardEvent(int n_code, WPARAM w_param, LPARAM l_param);
            static __declspec(dllexport) LRESULT CALLBACK StaticMouseEvent(int n_code, WPARAM w_param, LPARAM l_param);
            static HotkeyManager* hk_instance;
            static HHOOK keyboard_hook_handle;
            static HHOOK mouseHook;

        private:
            int _cur_line = -1;
            std::vector<int> _current_hotkey_list;
            std::vector<std::atomic<Limit>*> _limit_ptr_vector;
            bool _chatbox_open = false;
            typedef void (*KeyboardEventCallback)(int, bool);
            void KeyboardEvent(int n_code, WPARAM w_param, LPARAM l_param);
            void MouseEvent(int n_code, WPARAM w_param, LPARAM l_param);
            char combined_windivert_rules[1000] = "";
            std::vector<int> _currently_pressed_keys;
            void MessageLoop();
            Settings* _settings;
            std::shared_ptr<spdlog::logger> logger;
    };
}
