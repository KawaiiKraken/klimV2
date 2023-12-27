#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H
#include <vector>
#include <mutex>
#include <iostream>
#include <chrono>
#include <Windows.h>
#include "helperFunctions.h"
#include "Limit.h"

class HotkeyManager {
public:
    HotkeyManager(std::vector<std::atomic<limit>*> _limit_ptr_vector);
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