#pragma once
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
// these 2 are for the phnt header files
//#pragma clang diagnostic ignored "-Wpragma-pack"
//#pragma clang diagnostic ignored "-Wmicrosoft-enum-forward-reference"
#include "../phnt/phnt_windows.h"
#include "../phnt/phnt.h"
#include "windivertFunctions.h"
#include "..\jsoncpp_x64-windows\include\json\json.h"

struct limit {
    wchar_t* name;
    char hotkey = 0x0;
    char modkey = 0x0;
    bool state = false;
    wchar_t state_name[20] = {'\0'};
    bool hotkey_down = false;
    int overlayLineNumber = -1;
    DWORD modkey_state = 0;
    void toggleState() {
        state = !state;
        wcscpy_s(state_name, state ? (wchar_t*)L"(on)" : (wchar_t*)L"(off)");
    }
    //limit(wchar_t* n, int i) : name(n), overlayLineNumber(i) {}
    limit(wchar_t* n) : name(n) {}
};

void formatHotkeyStatusWcString( wchar_t* wcstring, int szWcstring, limit* limit);
bool isD2Active();
bool IsElevated();

Json::Value loadConfigFileFromJson(wchar_t* filePath);
void setVarFromJson(limit* limit, std::string hotkey, std::string modkey);
void writeDefaultJsonConfig(wchar_t* filePath);
bool FileExists( LPCTSTR szPath );
const wchar_t* GetFileName( const wchar_t *path );

void toggleBlockingLimit( limit* limit,     COLORREF colorOn, COLORREF colorOff);
void toggleSuspend(       limit* suspend,   COLORREF colorOn, COLORREF colorOff );
void toggleWholeGameLimit(limit* lim_game,  COLORREF colorOn, COLORREF colorOff );
