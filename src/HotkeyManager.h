#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H
#include "Limit.h"
#include <Windows.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>

struct Settings;

class HotkeyManager {
public:
    HotkeyManager(std::vector<std::atomic<Limit>*> _limit_ptr_vector);
    static void TriggerHotkeys(std::vector<std::atomic<Limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys, bool debug, char combined_windivert_rules[1000]);
    static void UnTriggerHotkeys(std::vector<std::atomic<Limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys);
    static void OnTriggerHotkey(std::atomic<Limit>* limitarg, bool debug, std::vector<std::atomic<Limit>*> limit_ptr_vector, char* combined_windivert_rules);
    void asyncBindHotkey(int i);
    void KeyboardInputHandler(int key, bool isKeyDown);
    int done = false;

private:
    int _cur_line = -1;
    std::vector<int> _currentHotkeyList;
    std::vector<std::atomic<Limit>*> _limit_ptr_vector;
    bool cancelled = false;
};


#endif HOTKEYMANAGER_H