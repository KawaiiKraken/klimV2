#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H
#include <vector>
#include <mutex>
#include <iostream>
#include <chrono>
#include <Windows.h>
#include "Limit.h"

struct Settings;

class HotkeyManager {
public:
    HotkeyManager(std::vector<std::atomic<limit>*> _limit_ptr_vector);
    static void TriggerHotkeys(std::vector<std::atomic<limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys, bool debug, Settings settings, char combined_windivert_rules[1000]);
    static void UnTriggerHotkeys(std::vector<std::atomic<limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys);
    static void OnTriggerHotkey(std::atomic<limit>* limitarg, bool debug, std::vector<std::atomic<limit>*> limit_ptr_vector, char* combined_windivert_rules);
    void asyncBindHotkey(int i);
	void KeyboardInputHandler(int key, bool isKeyDown);
    int done = false;

private:
    int _cur_line = -1;
    std::vector<int> _currentHotkeyList;
    std::vector<std::atomic<limit>*> _limit_ptr_vector;
    bool cancelled = false;
};


#endif HOTKEYMANAGER_H