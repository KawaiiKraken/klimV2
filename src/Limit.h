#pragma once
#include <windows.h>
#include "helperFunctions.h"
#include <vector>


struct limit {
    char name[40];
    int key_list[20];
    int max_key_list_size = 20;
    bool bindingComplete = true;
    bool triggered = false;
    bool state = false;
    int overlay_line_number = -1;
    char windivert_rule[250];
    bool updateUI = false;

    limit() {
        std::fill(std::begin(windivert_rule), std::end(windivert_rule), '\0');
        std::fill(std::begin(key_list), std::end(key_list), 0);
    }

    limit(const char* n, const char* b = "") : limit() {
        strncpy_s(name, n, sizeof(name));
        strncpy_s(windivert_rule, b, sizeof(windivert_rule));
    }
};

class Limit {
public:
	static void ToggleSuspend(       std::atomic<limit>* suspend,   Settings settings);
	static void ToggleWholeGameLimit(std::atomic<limit>* lim_game,  Settings settings);
private:
    static void SuspendProcess(DWORD pid, bool suspend);
};
