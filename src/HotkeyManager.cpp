#include "HotkeyManager.h"
HotkeyManager::HotkeyManager(std::vector<std::atomic<limit>*> _limit_ptr_vector)
    : _cur_line(-1), _currentHotkeyList(), done(false), _limit_ptr_vector(_limit_ptr_vector) {
}
void HotkeyManager::asyncBindHotkey(int i) {
    _cur_line = i;
    if (_limit_ptr_vector[_cur_line]->load().bindingComplete == false) {
        MessageBox(NULL, (wchar_t*)L"error hotkey is already being bound...", NULL, MB_OK);
	    return;
    }
    limit temp_limit = _limit_ptr_vector[_cur_line]->load();
    temp_limit.bindingComplete = false;
    _limit_ptr_vector[_cur_line]->store(temp_limit);

    _currentHotkeyList.clear();

    done = false;
    while (done == false) {
        Sleep(10);
    }
    temp_limit = _limit_ptr_vector[_cur_line]->load();
    temp_limit.updateUI = true;
    std::cout << "current size: " << _currentHotkeyList.size() << std::endl;
    temp_limit.bindingComplete = true;
    _limit_ptr_vector[_cur_line]->store(temp_limit);
    _cur_line = -1;

    return;
}

void HotkeyManager::KeyboardInputHandler(int key, bool isKeyDown) {
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
		auto it = std::find(_currentHotkeyList.begin(), _currentHotkeyList.end(), key);
        limit temp_limit = _limit_ptr_vector[_cur_line]->load();
        for (int i = 0; i < _currentHotkeyList.size(); i++) {
            temp_limit.key_list[i] = _currentHotkeyList[i];
        }
        _limit_ptr_vector[_cur_line]->store(temp_limit);
        done = true;
	}
}