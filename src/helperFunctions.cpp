#include "helperFunctions.h"
#include "krekens_overlay.h"

const wchar_t* GetFileName( const wchar_t *path )
{
    const wchar_t *filename = wcsrchr( path, '\\' );
    if ( filename == NULL )
        filename = path;
    else
        filename++;
    return filename;
}



bool isD2Active()
{
    TCHAR buffer[MAX_PATH] = {0};
    DWORD dwProcId = 0; 

    GetWindowThreadProcessId( GetForegroundWindow(), &dwProcId );
    
    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, dwProcId );
    GetModuleFileNameEx( hProc, NULL, buffer, MAX_PATH );
    CloseHandle( hProc );

    const wchar_t* bufferFilename = GetFileName( buffer );
    printf( "active window filename: %ls\n", bufferFilename );

    if ( wcscmp( bufferFilename, L"destiny2.exe" ) == 0 ){
        return TRUE;
    } else {
        return FALSE;
    }
}


// TODO better name for this function
void formatHotkeyStatusWcString( wchar_t* wcString, int szWcString, limit* limit){ 
    char hotkeyBuffer[1] = { limit->hotkey };
    char modkeyBuffer[1] = { limit->modkey };
    
    wchar_t* wcHotkeyBuffer = new wchar_t[2];
    wchar_t* wcModkeyBuffer = new wchar_t[2];
    size_t outSize;

    mbstowcs_s( &outSize, wcHotkeyBuffer, 2, hotkeyBuffer, 1);
    mbstowcs_s( &outSize, wcModkeyBuffer, 2, modkeyBuffer, 1);

    wcscpy_s(wcString, static_cast<rsize_t>(szWcString) - 1, wcModkeyBuffer);
    if ( limit->modkey == VK_SHIFT ){
        wcscpy_s( wcString, sizeof(L"shift"), L"shift" );
    }
    if ( limit->modkey == VK_CONTROL ){
        wcscpy_s( wcString, sizeof(L"ctrl"), L"ctrl" );
    }
    if ( limit->modkey == VK_MENU ){
        wcscpy_s( wcString, sizeof(L"alt"), L"alt" );
    }
    wcscat_s(wcString, szWcString, L"+");
    wcscat_s(wcString, szWcString, wcHotkeyBuffer);
    wcscat_s(wcString, szWcString, L" to ");
    wcscat_s(wcString, szWcString, limit->name);
    wcscat_s(wcString, szWcString, limit->state_name);

    delete []wcHotkeyBuffer;
}



bool IsElevated(){
    bool fRet = false;
    HANDLE hToken = NULL;
    if( OpenProcessToken( GetCurrentProcess(),TOKEN_QUERY,&hToken ) ) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize ) ) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if( hToken ) {
        CloseHandle( hToken );
    }
    return fRet;
}



bool FileExists( LPCTSTR szPath )
{
  DWORD dwAttrib = GetFileAttributes( szPath );

  return ( dwAttrib != INVALID_FILE_ATTRIBUTES && !( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) );
}



void writeIniContents( wchar_t* filePath ){
    if ( !FileExists( filePath ) ){
        printf( "creating config file\n" );
        CreateFileW( (LPCTSTR)filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
        printf( "setting config file to default settings\n" );
        // TODO switch to json https://github.com/open-source-parsers/jsoncpp
        WritePrivateProfileString( L"", L"Modkey accepts any key that hotkey does or 'shift', 'alt', 'ctrl'. Capitalization matters.", L"", filePath );
        WritePrivateProfileString( L"", L"game lim only works if you have windows pro edition", L"", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_exitapp", L"k", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_exitapp", L"ctrl", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_3074", L"g", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_3074", L"ctrl", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_3074_UL", L"c", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_3074_UL", L"ctrl", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_27k", L"6", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_27k", L"ctrl", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_27k_UL", L"7", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_27k_UL", L"ctrl", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_30k", L"l", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_30k", L"ctrl", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_7k", L"j", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_7k", L"ctrl", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_game", L"o", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_game", L"ctrl", filePath );
        WritePrivateProfileString( L"hotkeys", L"hotkey_suspend", L"p", filePath );
        WritePrivateProfileString( L"hotkeys", L"modkey_suspend", L"ctrl", filePath );
        WritePrivateProfileString( L"other", L"useOverlay", L"true", filePath );
        WritePrivateProfileString( L"other", L"fontSize", L"30", filePath );
        WritePrivateProfileString( L"other", L"colorDefault", L"0x00FFFFFF", filePath );
        WritePrivateProfileString( L"other", L"colorOn", L"0x000000FF", filePath );
        WritePrivateProfileString( L"other", L"colorOff", L"0x00FFFFFF", filePath );
    }
}



void toggle3074( limit* lim3074, COLORREF colorOn, COLORREF colorOff )
{
    lim3074->toggleState();
    printf("3074 state: %ws\n", lim3074->state_name);

    COLORREF color = lim3074->state ? colorOn : colorOff;
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString(wcstring, 200, lim3074);
    updateOverlayLine( wcstring, 1, color);
    delete []wcstring;
}



void suspendProcess(DWORD pid) {
    if ( pid != 0 ){
        HANDLE procHandle = NULL;
        procHandle = OpenProcess(PROCESS_SUSPEND_RESUME, 0, pid); 
        if ( procHandle != NULL ){
            printf("suspending process\n");
            NtSuspendProcess(procHandle);
            CloseHandle(procHandle);
        }
    }
}



void resumeProcess(DWORD pid){
    if ( pid != 0 ){
        HANDLE procHandle = NULL;
        procHandle = OpenProcess(PROCESS_SUSPEND_RESUME, 0, pid); 
        if ( procHandle != NULL ){
            printf("resuming process\n");
            NtResumeProcess( procHandle );
            CloseHandle( procHandle );
        }
    }
}



void toggleSuspend( limit* suspend, COLORREF colorOn, COLORREF colorOff )
{
    DWORD pid = 0;
    GetWindowThreadProcessId( GetForegroundWindow(), &pid );
    if ( isD2Active() ){ // prevents from pausing random stuff if running with debug
        suspend->toggleState();
        printf( "suspend %s\n", suspend->state ? "true" : "false" );

        if ( suspend->state ){
            suspendProcess(pid);
        } else {
            resumeProcess(pid);
        }

        COLORREF color = suspend->state ? colorOn : colorOff;
        wchar_t* wcstring = new wchar_t[200];
        formatHotkeyStatusWcString(wcstring, 200, suspend);
        updateOverlayLine( wcstring, 8, color);
        delete []wcstring;
    }
}



void toggleGame( limit* lim_game, COLORREF colorOn, COLORREF colorOff )
{
    lim_game->toggleState();
    printf( "state_game %s\n", lim_game->state ? "true" : "false" );

    if ( lim_game->state ){
        ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -noe -c New-NetQosPolicy -Name 'Destiny2-Limit' -AppPathNameMatchCondition 'destiny2.exe' -ThrottleRateActionBitsPerSecond 0.801KB", NULL, SW_HIDE );
    } else {
        ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE );
    }

    COLORREF color = lim_game->state ? colorOn : colorOff;
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString(wcstring, 200, lim_game);
    updateOverlayLine( wcstring, 7, color);
    delete []wcstring;
}



void toggle7k( limit* lim7k, COLORREF colorOn, COLORREF colorOff )
{
    lim7k->toggleState();
    printf( "state7k %s\n", lim7k->state ? "true" : "false" );

    COLORREF color = lim7k->state ? colorOn : colorOff;
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString( wcstring, 200, lim7k);
    updateOverlayLine( wcstring, 6, color);
    delete []wcstring;
}



void toggle30k( limit* lim30k, COLORREF colorOn, COLORREF colorOff )
{
    lim30k->toggleState();
    printf( "state30k %s\n", lim30k->state ? "true" : "false" );

    COLORREF color = lim30k->state ? colorOn : colorOff;
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString( wcstring, 200, lim30k);
    updateOverlayLine( wcstring, 5, color);
    delete []wcstring;
}



void toggle27k_UL( limit* lim27kUL, COLORREF colorOn, COLORREF colorOff )
{
    lim27kUL->toggleState();
    printf( "state3074UL %s\n", lim27kUL->state ? "true" : "false" );

    COLORREF color = lim27kUL->state ? colorOn : colorOff;
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString( wcstring, 200, lim27kUL);
    updateOverlayLine( wcstring, 4, color);
    delete []wcstring;
}



void toggle27k( limit* lim27k, COLORREF colorOn, COLORREF colorOff )
{
    lim27k->toggleState();
    printf( "state3074UL %s\n", lim27k->state ? "true" : "false" );

    COLORREF color = lim27k->state ? colorOn : colorOff;
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString( wcstring, 200, lim27k);
    updateOverlayLine( wcstring, 3, color);
    delete []wcstring;
}



void toggle3074_UL( limit* lim3074UL, COLORREF colorOn, COLORREF colorOff )
{
    lim3074UL->toggleState();
    printf( "state3074UL %s\n", lim3074UL->state ? "true" : "false" );

    COLORREF color = lim3074UL->state ? colorOn : colorOff;
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString( wcstring, 200, lim3074UL);
    updateOverlayLine( wcstring, 2, color);
    delete []wcstring;
}



void setVarFromIni( wchar_t* hotkey_name, char* hotkey_var, wchar_t* pathToIni )
{
    wchar_t buffer[200];
    wchar_t* wcSingleChar = nullptr;

    GetPrivateProfileStringW( L"hotkeys", hotkey_name, NULL, buffer, sizeof(buffer-4), pathToIni );
    if ( GetLastError() == 0x2 ){
        printf( "GetPrivateProfileString failed (%lu)\n", GetLastError() );
    } else {
        // convert from key nane to virtual keycode
        if ( wcscmp( buffer, L"alt" ) == 0 ){
            *hotkey_var = VK_MENU;
            printf( "set %ls to: alt\n", hotkey_name );
        } 
        else if ( wcscmp( buffer, L"shift" ) == 0 ){
            *hotkey_var = VK_SHIFT;
            printf( "set %ls to: shift\n", hotkey_name );
        } 
        else if ( wcscmp( buffer, L"ctrl" ) == 0 ){
            *hotkey_var = VK_CONTROL;
            printf( "set %ls to: ctrl\n", hotkey_name );
        } else {
            wcSingleChar = &buffer[0];
            *hotkey_var = VkKeyScanW( *wcSingleChar );
            printf( "set %ls to: %s\n", hotkey_name, hotkey_var);
        }
    } 
}