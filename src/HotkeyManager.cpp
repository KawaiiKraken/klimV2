#include "HotkeyManager.h"
#include "ConfigFile.h"
#include "HelperFunctions.h"
#include "WinDivertFunctions.h"
#include <algorithm>
#include <iostream>

namespace Klim
{
    HotkeyManager::HotkeyManager(const std::vector<std::atomic<Limit>*>& limit_ptr_vector)
        : done(false)
        , _cur_line(-1)
        , _current_hotkey_list()
        , _limit_ptr_vector(limit_ptr_vector)
    {
    }

    void HotkeyManager::AsyncBindHotkey(const int i)
    {
        _cur_line = i;
        if (_limit_ptr_vector[_cur_line]->load().binding_complete == false)
        {
            MessageBox(nullptr, L"error hotkey is already being bound...", nullptr, MB_OK);
            return;
        }

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
        std::cout << "current size: " << _current_hotkey_list.size() << "\n";
        temp_limit.binding_complete = true;
        _limit_ptr_vector[_cur_line]->store(temp_limit);
        _cur_line = -1;
    }

    void HotkeyManager::KeyboardInputHandler(const int key, const bool is_key_down)
    {
        if (_cur_line == -1)
        {
            return;
        }

        if (is_key_down)
        {
            std::cout << "Key Down: " << key << "\n";
            const std::vector<int>::iterator iterator = std::find(_current_hotkey_list.begin(), _current_hotkey_list.end(), key);
            if (iterator == _current_hotkey_list.end())
            {
                _current_hotkey_list.push_back(key);
            }
        }
        else
        {
            std::cout << "Key Up: " << key << "\n";
            std::vector<int>::iterator it = std::find(_current_hotkey_list.begin(), _current_hotkey_list.end(), key);
            Limit temp_limit = _limit_ptr_vector[_cur_line]->load();
            for (int i = 0; i < _current_hotkey_list.size(); i++)
            {
                temp_limit.key_list[i] = _current_hotkey_list[i];
            }
            _limit_ptr_vector[_cur_line]->store(temp_limit);
            done = true;
        }
    }


    void HotkeyManager::TriggerHotkeys(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, std::vector<int> currently_pressed_keys, const bool debug, char combined_windivert_rules[1000])
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

            if (contains_all)
            {
                OnTriggerHotkey(limit_ptr_vector[i], debug, limit_ptr_vector, combined_windivert_rules);
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


    void HotkeyManager::OnTriggerHotkey(std::atomic<Limit>* limit_arg, const bool debug, const std::vector<std::atomic<Limit>*>& limit_ptr_vector, char* combined_windivert_rules)
    {
        if (limit_arg->load().type == exit_app)
        {
            Helper::ExitApp(debug);
        }

        if (!(Helper::D2Active() || debug))
        {
            std::cout << "hotkey ignored: d2 is not the active window and debug mode is not on\n";
            return;
        }

        if (!limit_arg->load().triggered)
        {
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

            std::cout << "state of " << limit_arg->load().name << ": " << limit_arg->load().state << "\n";
            SetFilterRuleString(limit_ptr_vector, combined_windivert_rules);
            UpdateFilter(combined_windivert_rules);
        }
    }
}
