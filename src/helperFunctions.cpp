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
// Function to convert a vector to Json::Value
Json::Value vectorToJson(const std::vector<int>& vec) {
    Json::Value jsonVec;

    for (const auto& element : vec) {
        jsonVec.append(element);
    }

    return jsonVec;
}

// Function to convert Json::Value to a vector
std::vector<int> jsonToVector(const Json::Value& jsonVec) {
    std::vector<int> vec;

    for (const auto& element : jsonVec) {
        vec.push_back(element.asInt());
    }

    return vec;
}

void StoreConfigToJson( wchar_t* file_path, const Json::Value& config_data ){
    //if ( FileExists( file_path ) ){
        //return;
    //}
    printf( "creating new config file\n" );
    HANDLE hFile = CreateFileW( ( LPCTSTR )file_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE ){
        printf( "Error opening file for writing" );
        return;
    }

    std::string json_data = config_data.toStyledString();

    printf("setting config to default\n");
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
    wchar_t* wc_buffer[250];
    wcscpy_s(wcString, szWcString, L"");
    for (int i = 0; i < limit->key_list.size() - 1; i++) {

        int scan_code = MapVirtualKey(limit->key_list[i], 0);
        wchar_t nameBuffer[256];
        int length = GetKeyNameText(scan_code << 16, nameBuffer, sizeof(nameBuffer) / sizeof(nameBuffer[0]));
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



bool FileExists( LPCTSTR szPath ){
  DWORD dwAttrib = GetFileAttributes( szPath );

  return ( dwAttrib != INVALID_FILE_ATTRIBUTES && !( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) );
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


/*
int GetVKCodeFromName(const wchar_t* keyName) {
    std::wcout << "attempting to map key: " << keyName << std::endl;
    for (int vkCode = 1; vkCode < 256; vkCode++) {
        int scanCode = MapVirtualKey(vkCode, 0);

        wchar_t nameBuffer[256];
        int length = GetKeyNameText(scanCode << 16, nameBuffer, sizeof(nameBuffer) / sizeof(nameBuffer[0]));

        //printf("comparing %ws to %ws\n", nameBuffer, keyName);

        if (length && wcscmp(keyName, nameBuffer) == 0) {
            //std::cout << "scanCode: " << scanCode << std::endl;
            std::cout << "vkCode: " << vkCode << std::endl;
            return vkCode;
        }
    }
    return 0x0; 
}
*/


/*
void SetVarFromJson( limit* limit, std::string hotkey, std::string modkey ){
    const char* hotkey_char_ptr = hotkey.c_str();
    const char* modkey_char_ptr = modkey.c_str();
    wchar_t hotkey_buffer[20];
    wchar_t modkey_buffer[20];
    MultiByteToWideChar( CP_UTF8, 0, hotkey_char_ptr, -1, hotkey_buffer, 20 );
    MultiByteToWideChar( CP_UTF8, 0, modkey_char_ptr, -1, modkey_buffer, 20 );
    limit->hotkey = GetVKCodeFromName(hotkey_buffer);
    limit->modkey = GetVKCodeFromName(modkey_buffer);
}
*/