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
    printf( "buffer: %ls\n", buffer );
    const wchar_t* bufferFilename = GetFileName( buffer );
    printf( "filename: %ls\n", bufferFilename );
    CloseHandle( hProc );
    if ( wcscmp( bufferFilename, L"destiny2.exe" ) == 0 ){
        return TRUE;
    } else {
        return FALSE;
    }
}



void triggerHotkeyString( wchar_t* wcstring, int szWcstring, limit* limit){ // TODO better name for this
    char charbuf[2];
    char charbuf2[2];
    charbuf[0] = limit->hotkey;
    charbuf[1] = '\0'; // has to be null terminated for strlen to work 
    charbuf2[0] = limit->modkey;
    charbuf2[1] = '\0'; // has to be null terminated for strlen to work 
    
    int szCharbuf = strlen( charbuf ) + 1;
    int szCharbuf2 = strlen( charbuf2 ) + 1;
    wchar_t* wcstringbuf = new wchar_t[szCharbuf];
    wchar_t* wcstringbuf2 = new wchar_t[szCharbuf2];
    size_t outSize;

    mbstowcs_s( &outSize, wcstringbuf, szCharbuf, charbuf, szCharbuf-1 );
    mbstowcs_s( &outSize, wcstringbuf2, szCharbuf2, charbuf2, szCharbuf2-1 );
    wcscpy_s( wcstring, szWcstring-1, wcstringbuf2);
    if ( limit->modkey == VK_SHIFT ){
        wcscpy_s( wcstring, sizeof(L"shift"), L"shift" );
    }
    if ( limit->modkey == VK_CONTROL ){
        wcscpy_s( wcstring, sizeof(L"ctrl"), L"ctrl" );
    }
    if ( limit->modkey == VK_MENU ){
        wcscpy_s( wcstring, sizeof(L"alt"), L"alt" );
    }
    wcscat_s( wcstring, szWcstring, L"+");
    wcscat_s( wcstring, szWcstring, wcstringbuf );
    wcscat_s( wcstring, szWcstring, L" to " );
    wcscat_s( wcstring, szWcstring, limit->name);
    wcscat_s( wcstring, szWcstring, limit->state_name);

    delete []wcstringbuf;
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
        // TODO switch to json
        WritePrivateProfileString( L"", L"Modkey accepts any key that hotkey does or 'shift', 'alt', 'ctrl'. Capitilization matters.", L"", filePath );
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
    COLORREF color;
    lim3074->toggleState();
    printf( "state3074 %s\n", lim3074->state ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( lim3074->state ){
        triggerHotkeyString(wcstring, 200, lim3074);
        color = colorOn;
    } else {
        triggerHotkeyString(wcstring, 200, lim3074);
        color = colorOff;
    }
    updateOverlayLine( wcstring, 1, color);
    delete []wcstring;
}



void toggleSuspend( limit* suspend, COLORREF colorOn, COLORREF colorOff )
{
    COLORREF color;
    if ( isD2Active() ){
        DWORD pid = 0;
        // shitty way to get pid but eh
        GetWindowThreadProcessId( GetForegroundWindow(), &pid );
        suspend->toggleState();
        HANDLE procHandle = NULL;
        printf( "suspend %s\n", suspend->state ? "true" : "false" );
        wchar_t* wcstring = new wchar_t[200];

        if ( suspend->state ){
            triggerHotkeyString(wcstring, 200, suspend);
            color = colorOn;
            if ( pid != 0 ){
                printf( "pid: %lu\n", pid );
                procHandle = OpenProcess( 0x1F0FFF, 0, pid ); // TODO remove magic numbers
                if ( procHandle != NULL ){
                    printf( "suspending match\n" );
                    NtSuspendProcess( procHandle );
                }
            }
        
        } else {
            triggerHotkeyString(wcstring, 200, suspend);
            color = colorOff;
            if ( pid != 0 ){
                procHandle = OpenProcess( 0x1F0FFF, 0, pid ); // TODO remove magic numbers
                if ( procHandle != NULL ){
                    printf( "resuming match\n" );
                    NtResumeProcess( procHandle );
                }
            }
        }
        if ( procHandle != NULL ){
            CloseHandle( procHandle );
        }
        updateOverlayLine( wcstring, 8, color);
        delete []wcstring;
    }
}



void toggleGame( limit* lim_game, COLORREF colorOn, COLORREF colorOff )
{
    COLORREF color;
    lim_game->toggleState();
    printf( "state_game %s\n", lim_game->state ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( lim_game->state ){
        ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -noe -c New-NetQosPolicy -Name 'Destiny2-Limit' -AppPathNameMatchCondition 'destiny2.exe' -ThrottleRateActionBitsPerSecond 0.801KB", NULL, SW_HIDE );
        triggerHotkeyString(wcstring, 200, lim_game);
        color = colorOn;
    } else {
        ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE );
        triggerHotkeyString(wcstring, 200, lim_game);
        color = colorOff;
    }
    updateOverlayLine( wcstring, 7, color);
    delete []wcstring;
}



void toggle7k( limit* lim7k, COLORREF colorOn, COLORREF colorOff )
{
    COLORREF color;
    lim7k->toggleState();
    printf( "state7k %s\n", lim7k->state ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( lim7k->state ){
        triggerHotkeyString( wcstring, 200, lim7k);
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, lim7k);
        color = colorOff;
    }
    updateOverlayLine( wcstring, 6, color);
    delete []wcstring;
}



void toggle30k( limit* lim30k, COLORREF colorOn, COLORREF colorOff )
{
    COLORREF color;
    lim30k->toggleState();
    printf( "state30k %s\n", lim30k->state ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( lim30k->state ){
        triggerHotkeyString( wcstring, 200, lim30k);
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, lim30k);
        color = colorOff;
    }
    updateOverlayLine( wcstring, 5, color);
    delete []wcstring;
}



void toggle27k_UL( limit* lim27kUL, COLORREF colorOn, COLORREF colorOff )
{
    COLORREF color;
    lim27kUL->toggleState();
    printf( "state3074UL %s\n", lim27kUL->state ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( lim27kUL->state ){
        triggerHotkeyString( wcstring, 200, lim27kUL);
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, lim27kUL);
        color = colorOff;
    }
    updateOverlayLine( wcstring, 4, color);
    delete []wcstring;
}



void toggle27k( limit* lim27k, COLORREF colorOn, COLORREF colorOff )
{
    COLORREF color;
    lim27k->toggleState();
    printf( "state3074UL %s\n", lim27k->state ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( lim27k->state ){
        triggerHotkeyString( wcstring, 200, lim27k);
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, lim27k);
        color = colorOff;
    }
    updateOverlayLine( wcstring, 3, color);
    delete []wcstring;
}



void toggle3074_UL( limit* lim3074UL, COLORREF colorOn, COLORREF colorOff )
{
    COLORREF color;
    lim3074UL->toggleState();
    printf( "state3074UL %s\n", lim3074UL->state ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( lim3074UL->state ){
        triggerHotkeyString( wcstring, 200, lim3074UL);
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, lim3074UL);
        color = colorOff;
    }
    updateOverlayLine( wcstring, 2, color);
    delete []wcstring;
}



void setVarFromIni( wchar_t* hotkey_name, char* hotkey_var, wchar_t* pathToIni )
{
    wchar_t buffer[50];
    wchar_t* wcSingleChar = nullptr;

    GetPrivateProfileStringW( L"hotkeys", hotkey_name, NULL, buffer, sizeof(buffer), pathToIni );
    if ( GetLastError() == 0x2 ){
        printf( "GetPrivateProfileString failed (%lu)\n", GetLastError() );
    } else {
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