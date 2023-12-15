#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H
#include <vector>
#include <mutex>
#include <iostream>
#include <chrono>
#include <Windows.h>
#include "helperFunctions.h"

class HotkeyManager {
public:
    HotkeyManager(std::vector<limit*> _limit_ptr_vector);
    void asyncBindHotkey(int i);
	void KeyboardInputHandler(int key, bool isKeyDown);

private:
    int _cur_line = -1;
    int _done = false;
    std::vector<int> _currentHotkeyList;
    std::vector<limit*> _limit_ptr_vector;
};


#endif HOTKEYMANAGER_H