#pragma once

#include "Limit.h"
// #include <mutex>
// #include <vector>

namespace Klim
{
    class Settings;
    class UserInterface;

    class HotkeyManager
    {
        public:
            HotkeyManager(const std::vector<std::atomic<Limit>*>& limit_ptr_vector);
            void TriggerHotkeys(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, std::vector<int> currently_pressed_keys, bool debug, char combined_windivert_rules[1000]);
            void UnTriggerHotkeys(std::vector<std::atomic<Limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys);
            void OnTriggerHotkey(std::atomic<Limit>* limit_arg, bool debug, const std::vector<std::atomic<Limit>*>& limit_ptr_vector, char* combined_windivert_rules);
            void AsyncBindHotkey(int i);
            void KeyboardInputHandler(int key, bool is_key_down);
            int done = false;
            UserInterface* ui_instance;

        private:
            int _cur_line = -1;
            std::vector<int> _current_hotkey_list;
            std::vector<std::atomic<Limit>*> _limit_ptr_vector;
            // bool _cancelled = false;
    };
}
