#include "helperFunctions.h"


bool D2Active(){
    TCHAR buffer[MAX_PATH] = {0};
    DWORD dw_proc_id = 0; 

    GetWindowThreadProcessId( GetForegroundWindow(), &dw_proc_id );
    
    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, dw_proc_id );
    GetModuleFileNameEx( hProc, NULL, buffer, MAX_PATH );
    CloseHandle( hProc );

    const wchar_t* filename = ConfigFile::GetFilename( buffer );
    printf( "active window filename: %ls\n", filename );

    if ( wcscmp( filename, L"destiny2.exe" ) == 0 ){
        return true;
    } else {
        return false;
    }
}


void FormatHotkeyStatusWcString( wchar_t* wcString, int szWcString, limit* limit ){ 
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



bool RunningAsAdmin(){
    bool fRet = false;
    HANDLE hToken = NULL;
    if( OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ) ){
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &elevation, sizeof( elevation ), &cbSize ) ){
            fRet = elevation.TokenIsElevated;
        }
    }
    if( hToken ){
        CloseHandle( hToken );
    }
    return fRet;
}





void SuspendProcess( DWORD pid, bool suspend){
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



void ToggleSuspend( limit* suspend, COLORREF color_on, COLORREF color_off ){
    if ( !D2Active() ){ // prevents from pausing random stuff if running with debug
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



void ToggleWholeGameLimit( limit* lim_game, COLORREF color_on, COLORREF color_off ){
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



void ToggleBlockingLimit( limit* limit, COLORREF color_on, COLORREF color_off ){
    limit->ToggleState();

    COLORREF color = limit->state ? color_on : color_off;
    wchar_t* wcstring = new wchar_t[200];
    FormatHotkeyStatusWcString( wcstring, 200, limit);
    UpdateOverlayLine( wcstring, limit->overlay_line_number, color );
    delete []wcstring;
}
