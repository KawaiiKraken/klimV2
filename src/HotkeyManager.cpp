#include "HotkeyManager.h"
#include "ConfigFile.h"
#include "UserInterface.h"
#include "WinDivertFunctions.h"
#include <algorithm>
#include <iostream>

namespace Klim
{
    HotkeyManager::HotkeyManager(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, Settings* settings, std::shared_ptr<spdlog::logger> logger)
        : _cur_line(-1)
        , _current_hotkey_list()
        , _limit_ptr_vector(limit_ptr_vector)
        , ui_instance(nullptr)
        , _settings(settings)
        , logger(logger)
    {
    }


    void HotkeyManager::AsyncBindHotkey(int i)
    {
        if (!done)
        {
            MessageBox(nullptr, L"error hotkey is already being bound...", nullptr, MB_OK);
            return;
        }

        _cur_line = i;

        Limit temp_limit = _limit_ptr_vector[_cur_line]->load();
        temp_limit.binding_complete = false;
        for (int j = 0; j < temp_limit.max_key_list_size; j++)
        {
            temp_limit.key_list[j] = 0;
        }
        _limit_ptr_vector[_cur_line]->store(temp_limit);

        _current_hotkey_list.clear();

        done = false;
        while (done == false)
        {
            Sleep(10);
        }

        temp_limit = _limit_ptr_vector[_cur_line]->load();
        temp_limit.update_ui = true;
        logger->info("_current_hotkey_list.size = {}", _current_hotkey_list.size());
        temp_limit.binding_complete = true;
        _limit_ptr_vector[_cur_line]->store(temp_limit);
        _cur_line = -1;
    }

    // this function is part of the hotkey binding, is not related to anything else
    void HotkeyManager::KeyboardInputHandler(const int key, const bool is_key_down)
    {
        if (_cur_line == -1)
        {
            return;
        }

        if (is_key_down)
        {
            const std::vector<int>::iterator iterator = std::find(_current_hotkey_list.begin(), _current_hotkey_list.end(), key);
            if (iterator == _current_hotkey_list.end())
            {
                logger->info("input handler: key down: {}", key);
                _current_hotkey_list.push_back(key);
            }
        }
        else
        {
            std::vector<int>::iterator it = std::find(_current_hotkey_list.begin(), _current_hotkey_list.end(), key);
            Limit temp_limit = _limit_ptr_vector[_cur_line]->load();
            for (int i = 0; i < _current_hotkey_list.size(); i++)
            {
                logger->info("input handler: key up: {}", key);
                temp_limit.key_list[i] = _current_hotkey_list[i];
            }
            _limit_ptr_vector[_cur_line]->store(temp_limit);
            done = true;
        }
    }


    void HotkeyManager::TriggerHotkeys(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, std::vector<int> currently_pressed_keys, const bool debug)
    {
        for (size_t i = 0; i < limit_ptr_vector.size(); i++)
        {
            if (limit_ptr_vector[i]->load().key_list[0] == 0)
            {
                continue;
            }

            // Sort both vectors
            const Limit temp_limit = limit_ptr_vector[i]->load();
            std::vector<int> key_list;
            for (int j = 0; j < temp_limit.max_key_list_size; j++)
            {
                if (temp_limit.key_list[j] != 0)
                {
                    key_list.push_back(temp_limit.key_list[j]);
                }
            }
            std::sort(key_list.begin(), key_list.end());
            std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
            const bool contains_all = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), key_list.begin(), key_list.end());

            if ((contains_all && !_chatbox_open) || (contains_all && limit_ptr_vector[i]->load().type == Klim::exit_app))
            {
                OnTriggerHotkey(limit_ptr_vector[i], debug, limit_ptr_vector);
            }
        }
    }


    void HotkeyManager::UnTriggerHotkeys(std::vector<std::atomic<Limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys)
    {
        for (std::atomic<Limit>*& limit_ptr : limit_ptr_vector)
        {
            // Sort both vectors
            Limit temp_limit = limit_ptr->load();
            std::vector<int> key_list;
            for (int j = 0; j < temp_limit.max_key_list_size; j++)
            {
                if (temp_limit.key_list[j] != 0)
                {
                    key_list.push_back(temp_limit.key_list[j]);
                }
            }
            std::sort(key_list.begin(), key_list.end());
            std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
            const bool contains_all = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), key_list.begin(), key_list.end());

            if (!contains_all)
            {
                temp_limit.triggered = false;
                limit_ptr->store(temp_limit);
            }
        }
    }


    void HotkeyManager::OnTriggerHotkey(std::atomic<Limit>* limit_arg, const bool debug, const std::vector<std::atomic<Limit>*>& limit_ptr_vector)
    {
        if (limit_arg->load().type == exit_app)
        {
            Helper::ExitApp(debug, logger);
        }

        if (!debug && !Helper::IsDestinyTheActiveWindow(logger))
        {
            logger->info("hotkey ignored: d2 is not the active window and debug mode is not on");
            return;
        }

        if (!limit_arg->load().triggered)
        {
            for (int i = 0; i < limit_ptr_vector.size(); i++)
            {
                // bug somewhere in the for loop, only the first timer works
                if (strcmp(limit_arg->load().name, limit_ptr_vector[i]->load().name) == 0)
                {
                    if (ui_instance->timer_vector[i].running)
                    {

                        ui_instance->timer_vector[i].reset();
                        logger->info("timer stop {}", limit_arg->load().name);
                    }
                    else
                    {
                        ui_instance->timer_vector[i].start();
                        logger->info("timer start {}", limit_arg->load().name);
                    }
                }
            }
            Limit limit = limit_arg->load();
            limit.triggered = true;
            limit_arg->store(limit);
            if (limit_arg->load().type == limit_full_game)
            {
                Limit::ToggleWholeGameLimit(limit_arg);
            }
            else if (limit_arg->load().type == suspend_game)
            {
                Limit::ToggleSuspend(limit_arg);
            }
            else
            {
                limit = limit_arg->load();
                limit.state = !limit.state;
                limit_arg->store(limit);
            }

            logger->info("state of {}: {}", limit_arg->load().name, limit_arg->load().state);
            windivert_instance->SetFilterRuleString(limit_ptr_vector, combined_windivert_rules);
            windivert_instance->UpdateFilter(combined_windivert_rules);
        }
    }


    void HotkeyManager::KeyboardEvent(int n_code, WPARAM w_param, LPARAM l_param)
    {
        if (n_code != HC_ACTION)
        {
            return;
        }

        if (w_param == WM_SYSKEYUP || w_param == WM_KEYUP)
        {
            const KBDLLHOOKSTRUCT hooked_key = *reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);

            const int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
            const std::vector<int>::iterator iterator = std::find(_currently_pressed_keys.begin(), _currently_pressed_keys.end(), key);
            if (iterator != _currently_pressed_keys.end())
            {
                _currently_pressed_keys.erase(iterator);
                this->UnTriggerHotkeys(_limit_ptr_vector, _currently_pressed_keys);
            }
        }

        if (w_param == WM_SYSKEYDOWN || w_param == WM_KEYDOWN)
        {
            const KBDLLHOOKSTRUCT hooked_key = *reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);

            const int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
            const std::vector<int>::iterator iterator = std::find(_currently_pressed_keys.begin(), _currently_pressed_keys.end(), key);
            if (iterator == _currently_pressed_keys.end())
            {
                _currently_pressed_keys.push_back(key);
                if (key == VK_RETURN)
                {
                    if (Helper::IsDestinyTheActiveWindow(logger))
                    {
                        _chatbox_open = !_chatbox_open;
                        logger->info("chatbox {}", _chatbox_open ? "open" : "closed");
                    }
                }
                // chatbox can be closed with both escape too
                if (key == VK_ESCAPE)
                {
                    if (_chatbox_open)
                    {
                        if (Helper::IsDestinyTheActiveWindow(logger))
                        {
                            _chatbox_open = false;
                            logger->info("chatbox {}", _chatbox_open ? "open" : "closed");
                        }
                    }
                }
            }
            if (!ui_instance->show_config)
            {
                this->TriggerHotkeys(_limit_ptr_vector, _currently_pressed_keys, _settings->debug);
            }
        }
    }


    LRESULT HotkeyManager::StaticKeyboardEvent(int n_code, WPARAM w_param, LPARAM l_param)
    {
        HotkeyManager* hk_instance_ptr = HotkeyManager::hk_instance;
        hk_instance_ptr->KeyboardEvent(n_code, w_param, l_param);
        return CallNextHookEx(HotkeyManager::keyboard_hook_handle, n_code, w_param, l_param);
    }

    LRESULT HotkeyManager::StaticMouseEvent(int n_code, WPARAM w_param, LPARAM l_param)
    {
        HotkeyManager* hk_instance_ptr = HotkeyManager::hk_instance;
        hk_instance_ptr->MouseEvent(n_code, w_param, l_param);
        return CallNextHookEx(HotkeyManager::keyboard_hook_handle, n_code, w_param, l_param);
    }


    LRESULT CALLBACK HotkeyManager::MouseProc(int n_code, WPARAM w_param, LPARAM l_param)
    {
        HotkeyManager::StaticMouseEvent(n_code, w_param, l_param);

        return CallNextHookEx(HotkeyManager::mouseHook, n_code, w_param, l_param);
    }

    void HotkeyManager::MouseEvent(int n_code, WPARAM w_param, LPARAM l_param)
    {
        int vk_code = 0;
        switch (w_param)
        {
            // determine direction
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
                vk_code = VK_LBUTTON;
                break;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
                vk_code = VK_RBUTTON;
                break;

            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
                vk_code = VK_MBUTTON;
                break;

            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
                // xbutton has 2 possible values
                switch (GET_XBUTTON_WPARAM(((MSLLHOOKSTRUCT*)l_param)->mouseData))
                {
                    case XBUTTON1:
                        vk_code = VK_XBUTTON1;
                        break;
                    case XBUTTON2:
                        vk_code = VK_XBUTTON2;
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;
        }

        // get direction
        bool down = (w_param == WM_LBUTTONDOWN || w_param == WM_RBUTTONDOWN || w_param == WM_MBUTTONDOWN || w_param == WM_XBUTTONDOWN);
        if (vk_code != 0)
        {
            logger->info("mouse button {} {}", vk_code, down ? "down" : "up");

            // this should make binding work
            KeyboardInputHandler(vk_code, down); // yes ik "keyboard"

            if (down)
            {
                _currently_pressed_keys.push_back(vk_code);
                // TODO make this use bool allow_hotkeys instead
                if (!ui_instance->show_config)
                {
                    this->TriggerHotkeys(_limit_ptr_vector, _currently_pressed_keys, _settings->debug);
                }
            }

            if (!down)
            {
                const std::vector<int>::iterator iterator = std::find(_currently_pressed_keys.begin(), _currently_pressed_keys.end(), vk_code);
                if (iterator != _currently_pressed_keys.end())
                {
                    _currently_pressed_keys.erase(iterator);
                    this->UnTriggerHotkeys(_limit_ptr_vector, _currently_pressed_keys);
                }
            }
        }
    }


    DWORD HotkeyManager::HotkeyThread()
    {
        HotkeyManager::keyboard_hook_handle = SetWindowsHookEx(WH_KEYBOARD_LL, HotkeyManager::StaticKeyboardEvent, NULL, NULL);
        MessageLoop();
        UnhookWindowsHookEx(HotkeyManager::keyboard_hook_handle);
        UnhookWindowsHookEx(HotkeyManager::mouseHook);
        return 0;
    }

    void HotkeyManager::MessageLoop()
    {
        MSG message;
        while (GetMessage(&message, nullptr, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

}
