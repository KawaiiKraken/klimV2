#ifndef LIMIT_H
#define LIMIT_H
#include <windows.h>
#include "helperFunctions.h"

#define undefined_key 0x0

struct limit {
    wchar_t* name;
    std::vector<int> key_list = { undefined_key };
    bool bindingComplete = true;
    bool triggered = false;
    bool state = false;
    wchar_t state_name[20] = {'\0'};
    int overlay_line_number = -1;
    char windivert_rule[250];
    bool updateUI = false;
    void ToggleState() {
        state = !state;
        wcscpy_s(state_name, state ? (wchar_t*)L"(on)" : (wchar_t*)L"(off)");
    }
    limit(wchar_t* n) : name(n) {}
};

class Limit {
public:
    static void ToggleBlockingLimit( limit* limit,     COLORREF colorOn, COLORREF colorOff);
	static void ToggleSuspend(       limit* suspend,   COLORREF colorOn, COLORREF colorOff);
	static void ToggleWholeGameLimit(limit* lim_game,  COLORREF colorOn, COLORREF colorOff);
    static void FormatHotkeyStatusWcString(wchar_t* wcString, int szWcString, limit* limit);

private:
    static void SuspendProcess(DWORD pid, bool suspend);
};

#endif LIMIT_H