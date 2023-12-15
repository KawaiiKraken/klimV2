#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include "../phnt/phnt_windows.h"
#include "../phnt/phnt.h"
#include "windivertFunctions.h"
#include <vector>
#include <mutex>
#include "krekens_overlay.h"
#include <iostream>
#include "ConfigFile.h"

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

void FormatHotkeyStatusWcString(wchar_t* wcstring, int szWcstring, limit* limit);
bool D2Active();
bool RunningAsAdmin();

void ToggleBlockingLimit( limit* limit,     COLORREF colorOn, COLORREF colorOff);
void ToggleSuspend(       limit* suspend,   COLORREF colorOn, COLORREF colorOff);
void ToggleWholeGameLimit(limit* lim_game,  COLORREF colorOn, COLORREF colorOff);

#endif HELPERFUNCTIONS_H
