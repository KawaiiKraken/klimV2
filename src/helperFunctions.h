#pragma once
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
// these 2 are for the phnt header files
#pragma clang diagnostic ignored "-Wpragma-pack"
#pragma clang diagnostic ignored "-Wmicrosoft-enum-forward-reference"
#include "../phnt/phnt_windows.h"
#include "../phnt/phnt.h"
#include "windivertFunctions.h"

struct limit {
    char hotkey;
    char modkey;
    bool state = FALSE;
    bool hotkey_keydown = FALSE;
    DWORD modkey_state = 0;
};

void triggerHotkeyString( wchar_t* wcstring, int szWcstring, char hotkey, char modkey, wchar_t* action, wchar_t* state );
bool isD2Active();
BOOL IsElevated();

void setVarFromIni( wchar_t* hotkey_name, char* hotkey_var, wchar_t* pathToIni );
void writeIniContents( wchar_t* filepath );
BOOL FileExists( LPCTSTR szPath );
const wchar_t* GetFileName( const wchar_t *path );

void toggle3074( struct limit* lim3074, COLORREF colorOn, COLORREF colorOff );
void toggleSuspend( struct limit* suspend, COLORREF colorOn, COLORREF colorOff );
void toggleGame( struct limit* lim_game, COLORREF colorOn, COLORREF colorOff );
void toggle7k( struct limit* lim7k, COLORREF colorOn, COLORREF colorOff );
void toggle30k( struct limit* lim30k, COLORREF colorOn, COLORREF colorOff );
void toggle27k_UL( struct limit* lim27kUL, COLORREF colorOn, COLORREF colorOff );
void toggle27k( struct limit* lim27k, COLORREF colorOn, COLORREF colorOff );
void toggle3074_UL( struct limit* lim3074UL, COLORREF colorOn, COLORREF colorOff );
