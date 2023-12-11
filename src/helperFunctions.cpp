#include "helperFunctions.h"
#include "krekens_overlay.h"
#include <iostream>
#include <fstream>

const wchar_t* GetFilename( const wchar_t *path ){
    const wchar_t *filename = wcsrchr( path, '\\' );
    if ( filename == NULL )
        filename = path;
    else
        filename++;
    return filename;
}

void StoreConfigToJson( wchar_t* file_path, const Json::Value& config_data ){
    if ( FileExists( file_path ) ){
        return;
    }
    printf( "creating new config file\n" );
    HANDLE hFile = CreateFileW( ( LPCTSTR )file_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE ){
        printf( "Error opening file for writing" );
        return;
    }

    std::string json_data = config_data.toStyledString();

    printf("settings config to default\n");
    DWORD bytes_written;
    WriteFile( hFile, json_data.c_str(), static_cast<DWORD>( json_data.length() ), &bytes_written, NULL );

    CloseHandle( hFile );
}


bool D2Active(){
    TCHAR buffer[MAX_PATH] = {0};
    DWORD dw_proc_id = 0; 

    GetWindowThreadProcessId( GetForegroundWindow(), &dw_proc_id );
    
    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, dw_proc_id );
    GetModuleFileNameEx( hProc, NULL, buffer, MAX_PATH );
    CloseHandle( hProc );

    const wchar_t* filename = GetFilename( buffer );
    printf( "active window filename: %ls\n", filename );

    if ( wcscmp( filename, L"destiny2.exe" ) == 0 ){
        return true;
    } else {
        return false;
    }
}


// random TODO make hotkey system work with right and left keys
void FormatHotkeyStatusWcString( wchar_t* wcString, int szWcString, limit* limit ){ 
    char hotkey_buffer[1] = { limit->hotkey };
    char modkey_buffer[1] = { limit->modkey };
    
    wchar_t* wc_hotkey_buffer = new wchar_t[2];
    wchar_t* wc_modkey_buffer = new wchar_t[2];
    size_t out_size;

    mbstowcs_s( &out_size, wc_hotkey_buffer, 2, hotkey_buffer, 1 );
    mbstowcs_s( &out_size, wc_modkey_buffer, 2, modkey_buffer, 1 );

    wcscpy_s( wcString, static_cast<rsize_t>( szWcString ) - 1, wc_modkey_buffer );
    if ( limit->modkey == VK_LSHIFT ){
        wcscpy_s( wcString, sizeof( L"shift" ), L"shift" );
    }
    if ( limit->modkey == VK_LCONTROL ){
        wcscpy_s( wcString, sizeof( L"ctrl" ), L"ctrl" );
    }
    if ( limit->modkey == VK_LMENU ){
        wcscpy_s( wcString, sizeof( L"alt" ), L"alt" );
    }
    if (limit->modkey != undefined_key) {
        wcscat_s( wcString, szWcString, L"+" );
    }
    wcscat_s( wcString, szWcString, wc_hotkey_buffer );
    wcscat_s( wcString, szWcString, L" to " );
    wcscat_s( wcString, szWcString, limit->name );
    wcscat_s( wcString, szWcString, limit->state_name );

    delete []wc_hotkey_buffer;
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



bool FileExists( LPCTSTR szPath ){
  DWORD dwAttrib = GetFileAttributes( szPath );

  return ( dwAttrib != INVALID_FILE_ATTRIBUTES && !( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) );
}



void WriteDefaultJsonConfig( wchar_t* filepath ){
    Json::Value config;
    config["hotkey_exitapp"] = "k";
    config["modkey_exitapp"] = "ctrl";
    config["hotkey_3074"] = "g";
    config["modkey_3074"] = "ctrl";
    config["hotkey_3074_ul"] = "c";
    config["modkey_3074_ul"] = "ctrl";
    config["hotkey_27k"] = "6";
    config["modkey_27k"] = "ctrl";
    config["hotkey_27k_ul"] = "7";
    config["modkey_27k_ul"] = "ctrl";
    config["hotkey_30k"] = "l";
    config["modkey_30k"] = "ctrl";
    config["hotkey_7k"] = "j";
    config["modkey_7k"] = "ctrl";
    config["hotkey_game"] = "o";
    config["modkey_game"] = "ctrl";
    config["hotkey_suspend"] = "p";
    config["modkey_suspend"] = "ctrl";
    config["use_overlay"] = true;
    config["font_size"] = 30;
    config["color_default"] = "0x00FFFFFF";
    config["color_on"] = "0x000000FF";
    config["color_off"] = "0x00FFFFFF";

    StoreConfigToJson(filepath, config);
}



Json::Value LoadConfigFileFromJson( wchar_t* filepath ){
    Json::Value json_data;
    std::ifstream config_file( filepath, std::ifstream::binary );

    if ( config_file.is_open() ){
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(config_file, json_data);

        if ( !parsingSuccessful ){
            std::cerr << "Error parsing JSON: " << reader.getFormattedErrorMessages() << std::endl;
        }
        config_file.close();
    } else {
        std::cerr << "Error opening file for reading" << std::endl;
    }

    return json_data;
}



void SuspendProcess( DWORD pid ){
    if ( pid == 0 ){
        return;
    }
    HANDLE hProc = NULL;
    hProc = OpenProcess( PROCESS_SUSPEND_RESUME, 0, pid ); 
    if ( hProc != NULL ){
        printf( "suspending process\n" );
        NtSuspendProcess( hProc );
        CloseHandle( hProc );
    }
}



void ResumeProcess(DWORD pid){
    if ( pid == 0 ){
        return;
    }
    HANDLE hProc = NULL;
    hProc = OpenProcess( PROCESS_SUSPEND_RESUME, 0, pid ); 
    if ( hProc != NULL ){
        printf( "resuming process\n" );
        NtResumeProcess( hProc );
        CloseHandle( hProc );
    }
}



void ToggleSuspend( limit* suspend, COLORREF color_on, COLORREF color_off ){
    DWORD pid = 0;
    GetWindowThreadProcessId( GetForegroundWindow(), &pid );
    if ( D2Active() ){ // prevents from pausing random stuff if running with debug
        suspend->ToggleState();

        if ( suspend->state ){
            SuspendProcess( pid );
        } else {
            ResumeProcess( pid );
        }

        COLORREF color = suspend->state ? color_on : color_off;
        wchar_t* wcstring = new wchar_t[200];
        FormatHotkeyStatusWcString( wcstring, 200, suspend );
        UpdateOverlayLine( wcstring, suspend->overlay_line_number, color );
        delete []wcstring;
    }
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



void SetVarByKeyName( limit* limit, int* key, wchar_t* buffer ){
    // convert from key name to virtual keycode
    if ( wcscmp( buffer, L"") == 0 ){
        *key = 0x0;
        printf( "set %ls to: 0x0\n", limit->name );
    } 
    else if  ( wcscmp( buffer, L"alt" ) == 0 ){
        *key = VK_LMENU;
        printf( "set %ls to: alt\n", limit->name );
    } 
    else if  ( wcscmp( buffer, L"shift" ) == 0 ){
        //*key = VK_SHIFT;
        *key = VK_LSHIFT;
        printf( "set %ls to: shift\n", limit->name );
    } 
    else if ( wcscmp( buffer, L"ctrl" ) == 0 ){
        //*key = VK_CONTROL;
        *key = VK_LCONTROL;
        printf ("set %ls to: ctrl\n", limit->name );
    } else {
        wchar_t* single_wc = nullptr;
        single_wc = &buffer[0];
        *key = VkKeyScanW( *single_wc );
        printf( "set %ls to: %wc+%wc\n", limit->name, limit->hotkey, limit->modkey );
    }
}


void SetVarFromJson( limit* limit, std::string hotkey, std::string modkey ){
    const char* hotkey_char_ptr = hotkey.c_str();
    const char* modkey_char_ptr = modkey.c_str();
    wchar_t hotkey_buffer[20];
    wchar_t modkey_buffer[20];
    MultiByteToWideChar( CP_UTF8, 0, hotkey_char_ptr, -1, hotkey_buffer, 20 );
    MultiByteToWideChar( CP_UTF8, 0, modkey_char_ptr, -1, modkey_buffer, 20 );
    SetVarByKeyName( limit, &limit->hotkey, hotkey_buffer );
    SetVarByKeyName( limit, &limit->modkey, modkey_buffer );
}