#include "Limit.h"


void Limit::ToggleBlockingLimit( limit* limit, COLORREF color_on, COLORREF color_off ){
    limit->ToggleState();

    COLORREF color = limit->state ? color_on : color_off;
    wchar_t* wcstring = new wchar_t[200];
    FormatHotkeyStatusWcString( wcstring, 200, limit);
    UpdateOverlayLine( wcstring, limit->overlay_line_number, color );
    delete []wcstring;
}



void Limit::ToggleWholeGameLimit( limit* lim_game, COLORREF color_on, COLORREF color_off ){
    lim_game->ToggleState();

    if ( lim_game->state ){
        ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -noe -c New-NetQosPolicy -Name 'Destiny2-Limit' -AppPathNameMatchCondition 'destiny2.exe' -ThrottleRateActionBitsPerSecond 0.801KB", NULL, SW_HIDE );
    } else {
        ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE );
    }

    COLORREF color = lim_game->state ? color_on : color_off;
    wchar_t* wcstring = new wchar_t[200];
    FormatHotkeyStatusWcString( wcstring, 200, lim_game );
    UpdateOverlayLine( wcstring, lim_game->overlay_line_number, color);
    delete []wcstring;
}



void Limit::ToggleSuspend( limit* suspend, COLORREF color_on, COLORREF color_off ){
    if ( !Helper::D2Active() ){ // prevents from pausing random stuff if running with debug
        MessageBox(NULL, L"failed to pause...\nd2 is not the active window", NULL, MB_OK | MB_ICONWARNING);
        return;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId( GetForegroundWindow(), &pid );
    suspend->state ? SuspendProcess( pid, true ) : SuspendProcess( pid, false );

    suspend->ToggleState();
    COLORREF color = suspend->state ? color_on : color_off;
    wchar_t* wcstring = new wchar_t[200];
    FormatHotkeyStatusWcString( wcstring, 200, suspend );
    UpdateOverlayLine( wcstring, suspend->overlay_line_number, color );
    delete []wcstring;
}


// TODO fix this function
void Limit::FormatHotkeyStatusWcString( wchar_t* wcString, int szWcString, limit* limit ){ 
    wcscpy_s(wcString, szWcString, L"");
    for (int i = 0; i < limit->key_list.size() - 1; i++) {

        int scan_code = MapVirtualKey(limit->key_list[i], 0);
        wchar_t nameBuffer[256];
        GetKeyNameText(scan_code << 16, nameBuffer, sizeof(nameBuffer) / sizeof(nameBuffer[0]));
        wcscat_s(wcString, szWcString, L"+");
        wcscat_s(wcString, szWcString, nameBuffer);
    }
    wcscat_s(wcString, szWcString, L" to ");
    wcscat_s(wcString, szWcString, limit->name);
    wcscat_s(wcString, szWcString, limit->state_name);
}

void Limit::SuspendProcess( DWORD pid, bool suspend){
    if ( pid == 0 ){
        return;
    }

    HANDLE hProc = OpenProcess( PROCESS_SUSPEND_RESUME, 0, pid ); 

    if (hProc == NULL) {
        return;
    }
    suspend ? printf("suspending process\n") : printf("resuming process\n");
    suspend ? NtSuspendProcess( hProc ) : NtResumeProcess( hProc );
    CloseHandle( hProc );
}

