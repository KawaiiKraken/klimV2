#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include "../phnt/phnt_windows.h"
#include "../phnt/phnt.h"
#include "windivertFunctions.h"
#include "jsoncpp_header\json.h"

#define undefined_key 0x0

struct limit {
    wchar_t* name;
    int hotkey = undefined_key;
    int modkey = undefined_key;
    bool state = false;
    wchar_t state_name[20] = {'\0'};
    bool hotkey_down = false;
    bool modkey_down = false;
    int overlay_line_number = -1;
    char windivert_rule[250];
    void ToggleState() {
        state = !state;
        wcscpy_s(state_name, state ? (wchar_t*)L"(on)" : (wchar_t*)L"(off)");
    }
    limit(wchar_t* n) : name(n) {}
};

void FormatHotkeyStatusWcString(wchar_t* wcstring, int szWcstring, limit* limit);
bool D2Active();
bool RunningAsAdmin();

Json::Value LoadConfigFileFromJson(wchar_t* filePath);
void SetVarFromJson(limit* limit, std::string hotkey, std::string modkey);
void WriteDefaultJsonConfig(wchar_t* filePath);
bool FileExists(LPCTSTR szPath);
const wchar_t* GetFilename(const wchar_t *path);

void ToggleBlockingLimit( limit* limit,     COLORREF colorOn, COLORREF colorOff);
void ToggleSuspend(       limit* suspend,   COLORREF colorOn, COLORREF colorOff);
void ToggleWholeGameLimit(limit* lim_game,  COLORREF colorOn, COLORREF colorOff);
