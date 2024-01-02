#pragma once
#include <atomic>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

namespace Klim
{
    enum LimitType
    {
        invalid,

        limit_3074_dl,
        limit_3074_ul,

        limit_27k_dl,
        limit_27k_ul,

        limit_30k_dl,

        limit_7500_dl,

        limit_full_game,
        suspend_game,

        exit_app
    };


    class Limit
    {
        public:
            char name[40];
            LimitType type;
            int key_list[20];
            int max_key_list_size = 20;
            char windivert_rule[250];
            bool binding_complete = true;
            bool triggered = false;
            bool state = false;
            int overlay_line_number = -1;
            bool update_ui = false;

            Limit()
            {
                std::fill(std::begin(name), std::end(name), '\0');
                std::fill(std::begin(windivert_rule), std::end(windivert_rule), '\0');
                std::fill(std::begin(key_list), std::end(key_list), 0);
                type = invalid;
            }

            explicit Limit(const char* n, const char* b = "")
                : Limit()
            {
                strncpy_s(name, n, sizeof(name));
                strncpy_s(windivert_rule, b, sizeof(windivert_rule));

                type = StringToType(name);
            }

            static void ToggleSuspend(std::atomic<Limit>* suspend);
            static void ToggleWholeGameLimit(std::atomic<Limit>* limit_ptr);

            static const char* TypeToString(LimitType type);
            static LimitType StringToType(const char* str);

        private:
            static void SuspendProcess(DWORD pid, bool suspend);
    };
}
