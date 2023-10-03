// TODO comments
#include "main.h"

using namespace std;

struct limit {
    char hotkey;
    char modkey;
    bool state = FALSE;
    bool hotkey_keydown = FALSE;
    DWORD modkey_state = 0;
};

struct limit lim3074; 
struct limit lim3074UL; 
struct limit lim27k; 
struct limit lim27kUL; 
struct limit lim30k; 
struct limit lim7k; 
struct limit lim_game; 
struct limit suspend; 

void startFilter(){
    hThread = CreateThread( NULL, 0, block_traffic, handle, 0, NULL );
    can_trigger_any_hotkey = TRUE;
    printf( "hotkeys re-enabled\n" );
}

__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam )
{
    if  ( ( nCode == HC_ACTION ) &&   ( ( wParam == WM_SYSKEYUP ) ||  ( wParam == WM_KEYUP ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =    *( ( KBDLLHOOKSTRUCT* )lParam );
        DWORD dwMsg = 1;
        dwMsg += hooked_key.scanCode << 16;
        dwMsg += hooked_key.flags << 24;
        wchar_t lpszKeyName[1024] = {0};
        lpszKeyName[0] = L'[';

        int i = GetKeyNameText( dwMsg,   ( lpszKeyName+1 ), 0xFF ) + 1;
        lpszKeyName[i] = L']';
        int key = hooked_key.vkCode;

        if ( key != ( VK_SHIFT | VK_CONTROL | VK_MENU ) )   // this might be a bit broken
        {
            if ( key == hotkey_3074 ){
                lim3074.hotkey_keydown = FALSE;
            }
            if ( key == hotkey_3074_UL ){
                lim3074UL.hotkey_keydown =  FALSE;
            }
            if ( key == hotkey_27k){
                lim27k.hotkey_keydown = FALSE;
            }
            if ( key == hotkey_27k_UL){
                lim27kUL.hotkey_keydown = FALSE;
            }
            if ( key == hotkey_30k){
                lim30k.hotkey_keydown = FALSE;
            }
            if ( key == hotkey_7k){
                lim7k.hotkey_keydown = FALSE;
            }
            if ( key == hotkey_game){
                lim_game.hotkey_keydown = FALSE;
            }
            if ( key == hotkey_suspend){
                suspend.hotkey_keydown = FALSE;
            }
        }
    }


    if  ( ( nCode == HC_ACTION ) &&   ( ( wParam == WM_SYSKEYDOWN ) ||  ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =    *( ( KBDLLHOOKSTRUCT* ) lParam );
        DWORD dwMsg = 1;
        dwMsg += hooked_key.scanCode << 16;
        dwMsg += hooked_key.flags << 24;
        wchar_t lpszKeyName[1024] = {0};
        lpszKeyName[0] = L'[';

        int i = GetKeyNameText( dwMsg, ( lpszKeyName+1 ), 0xFF ) + 1;
        lpszKeyName[i] = L']';

        int key = hooked_key.vkCode;

        //if (key >= 'A' && key <= 'Z')   
        //if ( key != ( modkey_3074 | modkey_3074_UL | modkey_27k | modkey_30k | modkey_7k | modkey_suspend | modkey_exitapp ) )   // this might be a bit broken or unneeded
        if ( TRUE )   // this might be a bit broken or unneeded
        {
            // TODO use the hotkey system properly instead of using GetAsyncState
            lim3074.modkey_state = GetAsyncKeyState( modkey_3074 );
            lim3074UL.modkey_state = GetAsyncKeyState( modkey_3074_UL );
            lim27k.modkey_state = GetAsyncKeyState( modkey_27k );
            lim27kUL.modkey_state = GetAsyncKeyState( modkey_27k_UL );
            lim30k.modkey_state = GetAsyncKeyState( modkey_30k );
            lim7k.modkey_state = GetAsyncKeyState( modkey_7k );
            lim_game.modkey_state = GetAsyncKeyState( modkey_game );
            suspend.modkey_state = GetAsyncKeyState( modkey_suspend );
            DWORD modkey_exitapp_state = GetAsyncKeyState( modkey_exitapp );

            // double cuz im lazy enough to not bitshift
            lim3074.modkey_state = GetAsyncKeyState( modkey_3074 );
            lim3074UL.modkey_state = GetAsyncKeyState( modkey_3074_UL );
            lim27k.modkey_state = GetAsyncKeyState( modkey_27k );
            lim27kUL.modkey_state = GetAsyncKeyState( modkey_27k_UL );
            lim30k.modkey_state = GetAsyncKeyState( modkey_30k );
            lim7k.modkey_state = GetAsyncKeyState( modkey_7k );
            lim_game.modkey_state = GetAsyncKeyState( modkey_game );
            suspend.modkey_state = GetAsyncKeyState( modkey_suspend );
            modkey_exitapp_state = GetAsyncKeyState( modkey_exitapp );


            
            // ============= 3074 ================
            if ( lim3074.modkey_state !=0 && key == hotkey_3074 ) 
            {
                wcout << L"hotkey_3074 detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !lim3074.hotkey_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        lim3074.hotkey_keydown = TRUE;
                        toggle3074();
                        combinerules();
                        startFilter();
                    }
                }
                lim3074.modkey_state = 0;
            }

            // ============= 3074UL ================
            if ( lim3074UL.modkey_state !=0 && key == hotkey_3074_UL ) 
            {
                wcout << L"hotkey_3074 detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !lim3074UL.hotkey_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        lim3074UL.hotkey_keydown= TRUE;
                        toggle3074_UL();
                        combinerules();
                        startFilter();
                    }
                }
                lim3074UL.modkey_state = 0;
            }

            // ============= 27k ================
            if ( lim27k.modkey_state !=0 && key == hotkey_27k ) 
            {
                wcout << L"hotkey_27k detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !lim27k.hotkey_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        lim27k.hotkey_keydown = TRUE;
                        toggle27k();
                        combinerules();
                        startFilter();
                    }
                }
                lim27k.modkey_state = 0;
            }

            // ============= 27k_UL ================
            if ( lim27kUL.modkey_state !=0 && key == hotkey_27k_UL ) 
            {
                wcout << L"hotkey_27k detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !lim27kUL.hotkey_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        lim27kUL.hotkey_keydown = TRUE;
                        toggle27k_UL();
                        combinerules();
                        startFilter();
                    }
                }
                lim27kUL.modkey_state = 0;
            }


            // ============= 30k ================
            if ( lim3074.modkey_state !=0 && key == hotkey_30k ) 
            {
                wcout << L"hotkey_30k detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !lim30k.hotkey_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        lim30k.hotkey_keydown = TRUE;
                        toggle30k();
                        combinerules();
                        startFilter();
                    }
                }
                lim30k.modkey_state = 0;
            }

            // ============= 7k ================
            if ( lim7k.modkey_state !=0 && key == hotkey_7k ) 
            {
                wcout << L"hotkey_7k detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !lim7k.hotkey_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        lim7k.hotkey_keydown = TRUE;
                        toggle7k();
                        combinerules();
                        startFilter();
                    }
                }
                lim7k.modkey_state = 0;
            } 
            
            // ============= game ================
            if ( lim_game.modkey_state !=0 && key == hotkey_game ) 
            {
                wcout << L"hotkey_game detected\n";
                if ( isD2Active() | debug ){
                    if ( !lim_game.hotkey_keydown ){ 
                        lim_game.hotkey_keydown = TRUE;
                        toggleGame();
                    }
                }
                lim_game.modkey_state = 0;
            }


            // ============= suspend ================
            if ( suspend.modkey_state !=0 && key == hotkey_suspend ) 
            {
                wcout << L"hotkey_suspend detected\n";
                if ( isD2Active() | debug ){
                    if ( !suspend.hotkey_keydown ){
                        suspend.hotkey_keydown = TRUE;
                        toggleSuspend();
                    }
                }
                suspend.modkey_state = 0;
            }


            // ============= exitapp ================
            if ( modkey_exitapp_state !=0 && key == hotkey_exitapp )
            {
                wcout << "shutting down\n";
                if ( !debug ){
                    ShellExecute(NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE);
                    ShowWindow( GetConsoleWindow(), SW_RESTORE );
                }
                PostQuitMessage(0);
            }
        }
    }
    return CallNextHookEx( hKeyboardHook, nCode, wParam, lParam );
}

void MessageLoop()
{
    MSG message;
    while ( GetMessage( &message, NULL, 0, 0 ) ) 
    {
        TranslateMessage( &message );
        DispatchMessage( &message );
    }
}

DWORD WINAPI my_HotKey( LPVOID lpParm )
{
    HINSTANCE hInstance = GetModuleHandle( NULL);
    if ( !hInstance ) hInstance = LoadLibrary( ( LPCWSTR ) lpParm ); 
    if ( !hInstance ) return 1;

    hKeyboardHook = SetWindowsHookEx ( WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent, hInstance, NULL );
    MessageLoop();
    UnhookWindowsHookEx( hKeyboardHook );
    return 0;
}

BOOL FileExists( LPCTSTR szPath )
{
  DWORD dwAttrib = GetFileAttributes( szPath );

  return ( dwAttrib != INVALID_FILE_ATTRIBUTES && !( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) );
}



void setGlobalPathToIni(){ // this function does a bit too much, should prob split it up
    wchar_t szFilePathSelf[MAX_PATH], szFolderPathSelf[MAX_PATH];
    GetModuleFileName(NULL, szFilePathSelf, MAX_PATH);
    wcsncpy_s( szFolderPathSelf, sizeof( szFolderPathSelf ), szFilePathSelf, ( wcslen( szFilePathSelf ) - wcslen( GetFileName( szFilePathSelf ) ) ) );
    wchar_t filename[MAX_PATH], filePath[MAX_PATH];
    wcscpy_s( filename, MAX_PATH, L"config.ini" );
    wcscpy_s( filePath, MAX_PATH, szFolderPathSelf );
    wcscat_s( filePath, MAX_PATH, filename );
    if ( !FileExists( filePath ) ){
        printf( "creating config file\n" );
        CreateFileW( (LPCTSTR)filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
        printf( "setting config file to default settings\n" );
        // TODO make the description thingy better
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
    wcsncpy_s( pathToIni, sizeof( pathToIni ), filePath, sizeof( pathToIni ) );
}

void setVarFromIni(wchar_t* hotkey_name, char* hotkey_var){
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
            // TODO fix weird printf memory leak from hotkey_var
            printf( "set %ls to: %s\n", hotkey_name, hotkey_var);
        }
    } 
}

void setVarsFromIni(){ 
    wchar_t buffer[50];
    wchar_t* wcSingleChar = nullptr;
    setVarFromIni( (wchar_t*)L"hotkey_exitapp", &hotkey_exitapp );
    setVarFromIni( (wchar_t*)L"modkey_exitapp", &modkey_exitapp );
    setVarFromIni( (wchar_t*)L"hotkey_3074", &hotkey_3074 );
    setVarFromIni( (wchar_t*)L"modkey_3074", &modkey_3074 );
    setVarFromIni( (wchar_t*)L"hotkey_3074_UL", &hotkey_3074_UL );
    setVarFromIni( (wchar_t*)L"modkey_3074_UL", &modkey_3074_UL );
    setVarFromIni( (wchar_t*)L"hotkey_27k", &hotkey_27k );
    setVarFromIni( (wchar_t*)L"modkey_27k", &modkey_27k );
    setVarFromIni( (wchar_t*)L"hotkey_27k_UL", &hotkey_27k_UL );
    setVarFromIni( (wchar_t*)L"modkey_27k_UL", &modkey_27k_UL );
    setVarFromIni( (wchar_t*)L"hotkey_30k", &hotkey_30k );
    setVarFromIni( (wchar_t*)L"modkey_30k", &modkey_30k );
    setVarFromIni( (wchar_t*)L"hotkey_7k", &hotkey_7k );
    setVarFromIni( (wchar_t*)L"modkey_7k", &modkey_7k );
    setVarFromIni( (wchar_t*)L"hotkey_game", &hotkey_game );
    setVarFromIni( (wchar_t*)L"modkey_game", &modkey_game );
    setVarFromIni( (wchar_t*)L"hotkey_suspend", &hotkey_suspend );
    setVarFromIni( (wchar_t*)L"modkey_suspend", &modkey_suspend );
}




int __cdecl main( int argc, char** argv ){
    if ( argv[1] != NULL ){
        if ( strcmp( argv[1], "--help" ) == 0 ){
            printf( "options:\n" );
            printf( "    --help      prints this message.\n" );
            printf( "    --debug     prevents console hiding and enables hotkey trigger outside of destiny 2.\n" );
            return 0;
        }
    }

    // check if exe as admin
    if ( !IsElevated() ){
        MessageBox( NULL, (LPCWSTR)L"ERROR: not running as admin", (LPCWSTR)L"ERROR", MB_ICONERROR | MB_DEFBUTTON2 );
        return 0;
    }
    
    // check if running with debug
    char* arg1 = (char *)"nothing";
    if ( argv[1] != NULL ){
        arg1 = argv[1];
    }

    if ( ( strcmp( arg1, "--debug" ) == 0 ) ){
        printf( "debug: TRUE\n" );
        debug = TRUE;
    } else {
        ShowWindow( GetConsoleWindow(), SW_HIDE );
    }
    
    
    
    // load dll function

    hDLL = LoadLibrary( L"krekens_overlay" );
    if ( hDLL != NULL )
    {
        lpfnDllStartOverlay = (LPFNDLLSTARTOVERLAY)GetProcAddress( hDLL, "startOverlay" );
        if ( !lpfnDllStartOverlay )
        {
            // handle the error
            FreeLibrary( hDLL );
            printf( "handle the error");
            return -3;
        }
        lpfnDllUpdateOverlayLine = (LPFNDLLUPDATEOVERLAYLINE)GetProcAddress( hDLL, "updateOverlayLine" );
        if ( !lpfnDllUpdateOverlayLine )
        {
            // handle the error
            FreeLibrary( hDLL );
            printf( "handle the error");
            return -3;
        }
    }


    DWORD dwThread;
    
    // ini file stuff
    setGlobalPathToIni(); 
    wchar_t wc_buffer[50];
    bool useOverlay;
    GetPrivateProfileStringW( L"other", L"useOverlay", NULL, wc_buffer, sizeof(wc_buffer), pathToIni );
    if ( wcscmp(wc_buffer, L"true") == 0 ){
        useOverlay = true;
    } else if ( wcscmp(wc_buffer, L"false") == 0 ){
        useOverlay = false;
    } else {
        MessageBox(NULL, L"useOverlay config option is set to an incorrect value.. exiting.", NULL, MB_OK);
        return 0;
    }
    printf( "pathToIni %ls\n", pathToIni );

    int fontSize;
    GetPrivateProfileStringW( L"other", L"fontSize", NULL, wc_buffer, sizeof(wc_buffer), pathToIni );
    fontSize = wcstol(wc_buffer, NULL, 10);
    GetPrivateProfileStringW( L"other", L"colorDefault", NULL, wc_buffer, sizeof(wc_buffer), pathToIni );
    colorDefault = wcstol(wc_buffer, NULL, 16);
    GetPrivateProfileStringW( L"other", L"colorOn", NULL, wc_buffer, sizeof(wc_buffer), pathToIni );
    colorOn = wcstol(wc_buffer, NULL, 16);
    GetPrivateProfileStringW( L"other", L"colorOff", NULL, wc_buffer, sizeof(wc_buffer), pathToIni );
    colorOff = wcstol(wc_buffer, NULL, 16);

    setVarsFromIni();
    

    lpfnDllStartOverlay( useOverlay, fontSize );
    // TODO make this into a function
    wchar_t* wcstring = new wchar_t[200];
    triggerHotkeyString( wcstring, 200, hotkey_3074, modkey_3074, (wchar_t *)L"3074", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 1, colorDefault);

    triggerHotkeyString( wcstring, 200, hotkey_3074_UL, modkey_3074_UL, (wchar_t *)L"3074UL", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 2, colorDefault);

    triggerHotkeyString( wcstring, 200, hotkey_27k, modkey_27k, (wchar_t *)L"27k", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 3, colorDefault);

    triggerHotkeyString( wcstring, 200, hotkey_27k_UL, modkey_27k_UL, (wchar_t *)L"27kUL", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 4, colorDefault);

    triggerHotkeyString( wcstring, 200, hotkey_30k, modkey_30k, (wchar_t *)L"30k", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 5, colorDefault);

    triggerHotkeyString( wcstring, 200, hotkey_7k, modkey_7k, (wchar_t *)L"7k", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 6, colorDefault);

    triggerHotkeyString( wcstring, 200, hotkey_game, modkey_game, (wchar_t *)L"game", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 7, colorDefault);

    triggerHotkeyString( wcstring, 200, hotkey_suspend, modkey_suspend, (wchar_t *)L"suspend", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 8, colorDefault);

    triggerHotkeyString( wcstring, 200, hotkey_exitapp, modkey_exitapp, (wchar_t *)L"close", (wchar_t*)L"" );
    lpfnDllUpdateOverlayLine( wcstring, 9, colorDefault);
    
    delete []wcstring;


    printf( "starting hotkey thread\n" );
    hThread = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)my_HotKey, (LPVOID)NULL, NULL, &dwThread );

    if ( hThread ) return WaitForSingleObject( hThread, INFINITE );
    else return 1;
    CloseHandle( hThread );
    FreeLibrary( hDLL );
    return 0;
}


void combinerules(){
    strcpy_s( myNetRules, sizeof( myNetRules ), "(udp.DstPort < 1 and udp.DstPort > 1)" ); // set to rule that wont match anything
    if (state3074){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)" );
    }
    if (state3074_UL){
        strcat_s( myNetRules, sizeof( myNetRules ), "or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)" ); 
    }
    if (state27k){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)" );
    }
    if (state27k_UL){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)" );
    }
    if (state30k){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)" );
    }
    if (state7k){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)" );
    }
    printf( "filter: %s\n", myNetRules );
    if ( handle != NULL ){
        printf( "deleting old filter\n" );
        if( !WinDivertClose( handle ) ){
            fprintf( stderr, "error: failed to open the WinDivert device (%lu)\n", GetLastError() );
        }
    }


    printf( "creating new filter\n" );
    handle = WinDivertOpen( myNetRules, WINDIVERT_LAYER_NETWORK, priority, 0 );
    if ( handle == INVALID_HANDLE_VALUE )
    {
        if ( GetLastError() == ERROR_INVALID_PARAMETER && !WinDivertHelperCompileFilter( myNetRules, WINDIVERT_LAYER_NETWORK, NULL, 0, &err_str, NULL ) )
        {
            fprintf( stderr, "error: invalid filter \"%s\"\n", err_str );
            exit( EXIT_FAILURE );
        }
        fprintf( stderr, "error: failed to open the WinDivert device (%lu)\n", GetLastError() );
        exit( EXIT_FAILURE );
    }
}

void toggle3074(){
    COLORREF color;
    state3074 = !state3074;
    printf( "state3074 %s\n", state3074 ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( state3074 ){
        triggerHotkeyString( wcstring, 200, hotkey_3074, modkey_3074, (wchar_t *)L"3074", (wchar_t*)L" on" );
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, hotkey_3074, modkey_3074, (wchar_t *)L"3074", (wchar_t*)L" off" );
        color = colorOff;
    }
    lpfnDllUpdateOverlayLine( wcstring, 1, color);
    delete []wcstring;
}

void toggle3074_UL(){
    COLORREF color;
    state3074_UL = !state3074_UL;
    printf( "state3074UL %s\n", state3074_UL ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( state3074_UL ){
        triggerHotkeyString( wcstring, 200, hotkey_3074_UL, modkey_3074_UL, (wchar_t *)L"3074UL", (wchar_t*)L" on" );
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, hotkey_3074_UL, modkey_3074_UL, (wchar_t *)L"3074UL", (wchar_t*)L" off" );
        color = colorOff;
    }
    lpfnDllUpdateOverlayLine( wcstring, 2, color);
    delete []wcstring;
}

void toggle27k(){
    COLORREF color;
    state27k = !state27k;
    printf( "state3074UL %s\n", state27k ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( state27k ){
        triggerHotkeyString( wcstring, 200, hotkey_27k, modkey_27k, (wchar_t *)L"27k", (wchar_t*)L" on" );
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, hotkey_27k, modkey_27k, (wchar_t *)L"27k", (wchar_t*)L" off" );
        color = colorOff;
    }
    lpfnDllUpdateOverlayLine( wcstring, 3, color);
    delete []wcstring;
}

void toggle27k_UL(){
    COLORREF color;
    state27k_UL = !state27k_UL;
    printf( "state3074UL %s\n", state27k_UL ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( state27k_UL ){
        triggerHotkeyString( wcstring, 200, hotkey_27k_UL, modkey_27k_UL, (wchar_t *)L"27kUL", (wchar_t*)L" on" );
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, hotkey_27k_UL, modkey_27k_UL, (wchar_t *)L"27kUL", (wchar_t*)L" off" );
        color = colorOff;
    }
    lpfnDllUpdateOverlayLine( wcstring, 4, color);
    delete []wcstring;
}


void toggle30k(){
    COLORREF color;
    state30k = !state30k;
    printf( "state30k %s\n", state30k ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( state30k ){
        triggerHotkeyString( wcstring, 200, hotkey_30k, modkey_30k, (wchar_t *)L"30k", (wchar_t*)L" on" );
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, hotkey_30k, modkey_30k, (wchar_t *)L"30k", (wchar_t*)L" off" );
        color = colorOff;
    }
    lpfnDllUpdateOverlayLine( wcstring, 5, color);
    delete []wcstring;
}

void toggle7k(){
    COLORREF color;
    state7k = !state7k;
    printf( "state7k %s\n", state7k ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( state7k ){
        triggerHotkeyString( wcstring, 200, hotkey_7k, modkey_7k, (wchar_t *)L"7k", (wchar_t*)L" on" );
        color = colorOn;
    } else {
        triggerHotkeyString( wcstring, 200, hotkey_7k, modkey_7k, (wchar_t *)L"7k", (wchar_t*)L" off" );
        color = colorOff;
    }
    lpfnDllUpdateOverlayLine( wcstring, 6, color);
    delete []wcstring;
}

void toggleGame(){
    COLORREF color;
    state_game = !state_game;
    printf( "state_game %s\n", state_game ? "true" : "false" );
    wchar_t* wcstring = new wchar_t[200];
    if ( state_game ){
        ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -noe -c New-NetQosPolicy -Name 'Destiny2-Limit' -AppPathNameMatchCondition 'destiny2.exe' -ThrottleRateActionBitsPerSecond 800KB", NULL, SW_HIDE );
        triggerHotkeyString( wcstring, 200, hotkey_game, modkey_game, (wchar_t *)L"game", (wchar_t*)L" on" );
        color = colorOn;
    } else {
        ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE );
        triggerHotkeyString( wcstring, 200, hotkey_game, modkey_game, (wchar_t *)L"game", (wchar_t*)L" off" );
        color = colorOff;
    }
    lpfnDllUpdateOverlayLine( wcstring, 7, color);
    delete []wcstring;
}


void toggleSuspend(){
    COLORREF color;
    if ( isD2Active() ){
        DWORD pid = 0;
        // shitty way to get pid but eh
        GetWindowThreadProcessId( GetForegroundWindow(), &pid );
        state_suspend = !state_suspend;
        HANDLE procHandle = NULL;
        printf( "suspend %s\n", state_suspend ? "true" : "false" );
        wchar_t* wcstring = new wchar_t[200];

        if ( state_suspend ){
            triggerHotkeyString( wcstring, 200, hotkey_suspend, modkey_suspend, (wchar_t *)L"suspend", (wchar_t*)L" on" );
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
            triggerHotkeyString( wcstring, 200, hotkey_suspend, modkey_suspend, (wchar_t *)L"suspend", (wchar_t*)L" off" );
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
        lpfnDllUpdateOverlayLine( wcstring, 8, color);
        delete []wcstring;
    }
}
