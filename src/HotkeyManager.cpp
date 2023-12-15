#include "HotkeyManager.h"
HotkeyManager::HotkeyManager(std::vector<limit*> _limit_ptr_vector)
    : _cur_line(-1), _currentHotkeyList(), _done(false), _limit_ptr_vector(_limit_ptr_vector) {
}
void HotkeyManager::asyncBindHotkey(int i) {
    _cur_line = i;
    if (_limit_ptr_vector[_cur_line]->bindingComplete == false) {
        MessageBox(NULL, (wchar_t*)L"error hotkey is already being bound...", NULL, MB_OK);
	    return;
    }
    _limit_ptr_vector[_cur_line]->bindingComplete = false;

    _currentHotkeyList.clear();

    _done = false;
    while (_done == false) {
        Sleep(10);
    }
	_limit_ptr_vector[_cur_line]->updateUI = true;
    std::cout << "current size: " << _currentHotkeyList.size() << std::endl;
	_limit_ptr_vector[_cur_line]->bindingComplete = true;
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
		_limit_ptr_vector[_cur_line]->key_list = _currentHotkeyList;
        _done = true;
	}
}