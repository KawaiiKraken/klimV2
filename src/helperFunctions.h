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

struct limit {
    wchar_t* name;
    char hotkey = 0x3A; // this vkcode is undefined so it won't trigger anything, its only here to please the compiler 
    char modkey = 0x3A;
    bool state = false;
    wchar_t state_name[20] = {'\0'};
    bool hotkey_down = false;
    DWORD modkey_state = 0;
    void toggleState() {
        state = !state;
        wcscpy_s(state_name, state ? (wchar_t*)L"(on)" : (wchar_t*)L"(off)");
    }
    limit(wchar_t* n) : name(n) {}
};

void triggerHotkeyString( wchar_t* wcstring, int szWcstring, limit* limit);
bool isD2Active();
bool IsElevated();

void setVarFromIni( wchar_t* hotkey_name, char* hotkey_var, wchar_t* pathToIni );
void writeIniContents( wchar_t* filepath );
bool FileExists( LPCTSTR szPath );
const wchar_t* GetFileName( const wchar_t *path );

void toggle3074(    limit* lim3074,   COLORREF colorOn, COLORREF colorOff );
void toggleSuspend( limit* suspend,   COLORREF colorOn, COLORREF colorOff );
void toggleGame(    limit* lim_game,  COLORREF colorOn, COLORREF colorOff );
void toggle7k(      limit* lim7k,     COLORREF colorOn, COLORREF colorOff );
void toggle30k(     limit* lim30k,    COLORREF colorOn, COLORREF colorOff );
void toggle27k_UL(  limit* lim27kUL,  COLORREF colorOn, COLORREF colorOff );
void toggle27k(     limit* lim27k,    COLORREF colorOn, COLORREF colorOff );
void toggle3074_UL( limit* lim3074UL, COLORREF colorOn, COLORREF colorOff );
