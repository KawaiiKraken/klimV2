#include "helperFunctions.h"
#include "krekens_overlay.h"
#include <iostream>
#include <fstream>
#include "..\jsoncpp_x64-windows\include\json\json.h"

const wchar_t* GetFileName( const wchar_t *path )
{
    const wchar_t *filename = wcsrchr( path, '\\' );
    if ( filename == NULL )
        filename = path;
    else
        filename++;
    return filename;
}

void StoreConfigToJson(wchar_t* filePath, const Json::Value& configData) {
    // Open the file for writing
    if (FileExists(filePath)) {
        return;
    }
    printf("creating new config file\n");
    HANDLE hFile = CreateFileW( (LPCTSTR)filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );

    if (hFile == INVALID_HANDLE_VALUE) {
        // Handle error (you may want to add proper error handling)
        printf("Error opening file for writing");
        return;
    }

    // Convert Json::Value to string
    std::string jsonData = configData.toStyledString();

    // Write the JSON data to the file
    printf("settings config to default\n");
    DWORD bytesWritten;
    WriteFile(hFile, jsonData.c_str(), static_cast<DWORD>(jsonData.length()), &bytesWritten, NULL);

    // Close the file handle
    CloseHandle(hFile);
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



void writeDefaultJsonConfig(wchar_t* filePath) {
    Json::Value configData;
    configData["hotkey_exitapp"] = "k";
    configData["modkey_exitapp"] = "ctrl";
    configData["hotkey_3074"] = "g";
    configData["modkey_3074"] = "ctrl";
    configData["hotkey_3074UL"] = "c";
    configData["modkey_3074UL"] = "ctrl";
    configData["hotkey_27k"] = "6";
    configData["modkey_27k"] = "ctrl";
    configData["hotkey_27kUL"] = "7";
    configData["modkey_27kUL"] = "ctrl";
    configData["hotkey_30k"] = "l";
    configData["modkey_30k"] = "ctrl";
    configData["hotkey_7k"] = "j";
    configData["modkey_7k"] = "ctrl";
    configData["hotkey_game"] = "o";
    configData["modkey_game"] = "ctrl";
    configData["hotkey_suspend"] = "p";
    configData["modkey_suspend"] = "ctrl";
    configData["useOverlay"] = true;
    configData["fontSize"] = 30;
    configData["colorDefault"] = "0x00FFFFFF";
    configData["colorOn"] = "0x000000FF";
    configData["colorOff"] = "0x00FFFFFF";

    StoreConfigToJson(filePath, configData);
}



// Function to load configuration data from a JSON file
Json::Value loadConfigFileFromJson(wchar_t* filePath) {
    Json::Value jsonData;

    // Open the file for reading
    std::ifstream configFile(filePath, std::ifstream::binary);

    if (configFile.is_open()) {
        Json::Reader reader;

        // Read the JSON data from the file
        bool parsingSuccessful = reader.parse(configFile, jsonData);

        // Check if parsing was successful
        if (!parsingSuccessful) {
            // Handle error (you may want to add proper error handling)
            std::cerr << "Error parsing JSON: " << reader.getFormattedErrorMessages() << std::endl;
        }

        configFile.close();
    } else {
        // Handle error (you may want to add proper error handling)
        std::cerr << "Error opening file for reading" << std::endl;
    }

    return jsonData;
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
        updateOverlayLine( wcstring, suspend->overlayLineNumber, color);
        delete []wcstring;
    }
}



void toggleWholeGameLimit( limit* lim_game, COLORREF colorOn, COLORREF colorOff )
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
    updateOverlayLine( wcstring, lim_game->overlayLineNumber, color);
    delete []wcstring;
}



void toggleBlockingLimit(limit* limit, COLORREF colorOn, COLORREF colorOff)
{
    limit->toggleState();
    printf( "state of %ws: %s\n", limit->name, limit->state ? "true" : "false" );

    COLORREF color = limit->state ? colorOn : colorOff;
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString( wcstring, 200, limit);
    updateOverlayLine( wcstring, limit->overlayLineNumber, color);
    delete []wcstring;
}

void setVarByKeyName(limit* limit, char* key, wchar_t* buffer) {
    wchar_t* wcSingleChar = nullptr;
    // convert from key name to virtual keycode
    if ( wcscmp( buffer, L"alt" ) == 0 ){
        *key = VK_MENU;
        printf( "set %ls to: alt\n", limit->name);
    } else 
    if ( wcscmp( buffer, L"shift" ) == 0 ){
        *key = VK_SHIFT;
        printf( "set %ls to: shift\n", limit->name);
    } else 
    if ( wcscmp( buffer, L"ctrl" ) == 0 ){
        *key = VK_CONTROL;
        printf( "set %ls to: ctrl\n", limit->name);
    } else {
        wcSingleChar = &buffer[0];
        *key = VkKeyScanW( *wcSingleChar );
        printf( "set %ls to: %wc+%wc\n", limit->name, limit->hotkey, limit->modkey);
    }
}


void setVarFromJson(limit* limit, std::string hotkey, std::string modkey)
{
    const char* charPointer = hotkey.c_str();
    const char* charPointer2 = modkey.c_str();
    wchar_t buffer[200];
    wchar_t buffer2[200];
    MultiByteToWideChar(CP_UTF8, 0, charPointer, -1, buffer, 200);
    MultiByteToWideChar(CP_UTF8, 0, charPointer2, -1, buffer2, 200);
    setVarByKeyName(limit, &limit->hotkey, buffer);
    setVarByKeyName(limit, &limit->modkey, buffer2);
}