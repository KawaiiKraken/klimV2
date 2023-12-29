#pragma once
#include <windows.h>
#include <vector>
#include <atomic>


struct limit {
    char name[40];
    int key_list[20];
    int max_key_list_size = 20;
    char windivert_rule[250];
    bool bindingComplete = true;
    bool triggered = false;
    bool state = false;
    int overlay_line_number = -1;
    bool updateUI = false;

    limit() {
        std::fill(std::begin(name), std::end(name), '\0');
        std::fill(std::begin(windivert_rule), std::end(windivert_rule), '\0');
        std::fill(std::begin(key_list), std::end(key_list), 0);
    }

    limit(const char* n, const char* b = "") : limit() {
        strncpy_s(name, n, sizeof(name));
        strncpy_s(windivert_rule, b, sizeof(windivert_rule));
    }
};

struct Settings;

class Limit {
public:
	static void ToggleSuspend(       std::atomic<limit>* suspend,   Settings settings);
	static void ToggleWholeGameLimit(std::atomic<limit>* lim_game,  Settings settings);
private:
    static void SuspendProcess(DWORD pid, bool suspend);
};
