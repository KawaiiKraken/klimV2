#include "HotkeyManager.h"
#include "ConfigFile.h"
#include "helperFunctions.h"
#include "windivertFunctions.h"
#include <algorithm>

HotkeyManager::HotkeyManager(std::vector<std::atomic<limit>*> _limit_ptr_vector)
    : _cur_line(-1)
    , _currentHotkeyList()
    , done(false)
    , _limit_ptr_vector(_limit_ptr_vector)
{
}
void HotkeyManager::asyncBindHotkey(int i)
{
    _cur_line = i;
    if (_limit_ptr_vector[_cur_line]->load().bindingComplete == false) {
        MessageBox(NULL, ( wchar_t* )L"error hotkey is already being bound...", NULL, MB_OK);
        return;
    }
    limit temp_limit           = _limit_ptr_vector[_cur_line]->load();
    temp_limit.bindingComplete = false;
    _limit_ptr_vector[_cur_line]->store(temp_limit);

    _currentHotkeyList.clear();

    done = false;
    while (done == false) {
        Sleep(10);
    }
    temp_limit          = _limit_ptr_vector[_cur_line]->load();
    temp_limit.updateUI = true;
    std::cout << "current size: " << _currentHotkeyList.size() << std::endl;
    temp_limit.bindingComplete = true;
    _limit_ptr_vector[_cur_line]->store(temp_limit);
    _cur_line = -1;

    return;
}

void HotkeyManager::KeyboardInputHandler(int key, bool isKeyDown)
{
    if (_cur_line == -1) {
        return;
    }
    if (isKeyDown) {
        std::cout << "Key Down: " << key << std::endl;
        auto it = std::find(_currentHotkeyList.begin(), _currentHotkeyList.end(), key);
        if (it == _currentHotkeyList.end()) {
            _currentHotkeyList.push_back(key);
        }
    } else {
        std::cout << "Key Up: " << key << std::endl;
        auto it          = std::find(_currentHotkeyList.begin(), _currentHotkeyList.end(), key);
        limit temp_limit = _limit_ptr_vector[_cur_line]->load();
        for (int i = 0; i < _currentHotkeyList.size(); i++) {
            temp_limit.key_list[i] = _currentHotkeyList[i];
        }
        _limit_ptr_vector[_cur_line]->store(temp_limit);
        done = true;
    }
}


void HotkeyManager::TriggerHotkeys(
    std::vector<std::atomic<limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys, bool debug, Settings settings, char combined_windivert_rules[1000])
{
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        if (limit_ptr_vector[i]->load().key_list[0] == 0) {
            continue;
        }
        // Sort both vectors
        limit temp_limit = limit_ptr_vector[i]->load();
        std::vector<int> key_list;
        for (int j = 0; j < temp_limit.max_key_list_size; j++) {
            if (temp_limit.key_list[j] != 0) {
                key_list.push_back(temp_limit.key_list[j]);
            }
        }
        std::sort(key_list.begin(), key_list.end());
        std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), key_list.begin(), key_list.end());

        if (containsAll) {
            HotkeyManager::OnTriggerHotkey(limit_ptr_vector[i], debug, limit_ptr_vector, combined_windivert_rules);
        }
    }
}


void HotkeyManager::UnTriggerHotkeys(std::vector<std::atomic<limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys)
{
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        // Sort both vectors
        limit temp_limit = limit_ptr_vector[i]->load();
        std::vector<int> key_list;
        for (int j = 0; j < temp_limit.max_key_list_size; j++) {
            if (temp_limit.key_list[j] != 0) {
                key_list.push_back(temp_limit.key_list[j]);
            }
        }
        std::sort(key_list.begin(), key_list.end());
        std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), key_list.begin(), key_list.end());

        if (!containsAll) {
            temp_limit.triggered = false;
            limit_ptr_vector[i]->store(temp_limit);
        }
    }
}


void HotkeyManager::OnTriggerHotkey(std::atomic<limit>* limitarg, bool debug, std::vector<std::atomic<limit>*> limit_ptr_vector, char* combined_windivert_rules)
{
    if (strcmp(limitarg->load().name, "exitapp") == 0) {
        Helper::Exitapp(debug);
    }
    if (!(Helper::D2Active() || debug)) {
        printf("hotkey ignored: d2 is not the active window and debug mode is not on");
        return;
    }
    if (!limitarg->load().triggered) {
        limit limit     = limitarg->load();
        limit.triggered = true;
        limitarg->store(limit);
        if (strcmp(limitarg->load().name, "game") == 0) {
            Limit::ToggleWholeGameLimit(limitarg);
        } else if (strcmp(limitarg->load().name, "suspend") == 0) {
            Limit::ToggleSuspend(limitarg);
        } else {
            limit       = limitarg->load();
            limit.state = !limit.state;
            limitarg->store(limit);
        }
        printf("state of %s: %s\n", limitarg->load().name, limitarg->load().state ? "true" : "false");
        SetFilterRuleString(limit_ptr_vector, combined_windivert_rules);
        UpdateFilter(combined_windivert_rules);
    }
}