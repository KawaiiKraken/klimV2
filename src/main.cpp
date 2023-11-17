// TODO comments
#include "main.h"
#include "helperFunctions.h"
#include "krekens_overlay.h"

using namespace std;

char myNetRules[1000];
int onTriggerHotkey(limit* limit);
wchar_t pathToIni[MAX_PATH];

limit lim3074((wchar_t*)L"3074"); 
limit lim3074UL((wchar_t*)L"3074UL");
limit lim27k((wchar_t*)L"27k"); 
limit lim27kUL((wchar_t*)L"27kUL"); 
limit lim30k((wchar_t*)L"30k"); 
limit lim7k((wchar_t*)L"7k"); 
limit lim_game((wchar_t*)L"game"); 
limit suspend((wchar_t*)L"suspend"); 
limit exitapp((wchar_t*)L"exitapp"); 

int __cdecl main( int argc, char** argv ){
    if ( argv[1] != NULL ){
        if ( ( strcmp( argv[1], "--debug" ) == 0 ) ){
            printf( "debug: true\n" );
            debug = true;
        } else {
            printf( "error: invalid argument...\n" );
            printf( "options:\n" );
            printf( "    --debug     prevents console hiding and enables hotkey trigger outside of destiny 2.\n" );
            return 0;
        }
    } else {
        ShowWindow( GetConsoleWindow(), SW_HIDE );
    }

    // check if running as admin
    if ( !IsElevated() ){
        MessageBox( NULL, (LPCWSTR)L"ERROR: not running as admin", (LPCWSTR)L"ERROR", MB_ICONERROR | MB_DEFBUTTON2 );
        return 0;
    }

    DWORD dwThread;
    
    // config file stuff
    setPathToIni(); 
    if ( !FileExists( pathToIni ) ){
        writeIniContents( pathToIni );
    }
    wchar_t wc_buffer[50];
    bool useOverlay;
    // TODO fix memory leak
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

    setVarFromIni( (wchar_t*)L"hotkey_exitapp", &exitapp.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_exitapp", &exitapp.modkey, pathToIni );
    setVarFromIni( (wchar_t*)L"hotkey_3074", &lim3074.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_3074", &lim3074.modkey, pathToIni );
    setVarFromIni( (wchar_t*)L"hotkey_3074_UL", &lim3074UL.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_3074_UL", &lim3074UL.modkey, pathToIni );
    setVarFromIni( (wchar_t*)L"hotkey_27k", &lim27k.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_27k", &lim27k.modkey, pathToIni );
    setVarFromIni( (wchar_t*)L"hotkey_27k_UL", &lim27kUL.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_27k_UL", &lim27kUL.modkey, pathToIni );
    setVarFromIni( (wchar_t*)L"hotkey_30k", &lim30k.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_30k", &lim30k.modkey, pathToIni );
    setVarFromIni( (wchar_t*)L"hotkey_7k", &lim7k.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_7k", &lim7k.modkey, pathToIni );
    setVarFromIni( (wchar_t*)L"hotkey_game", &lim_game.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_game", &lim_game.modkey, pathToIni );
    setVarFromIni( (wchar_t*)L"hotkey_suspend", &suspend.hotkey, pathToIni );
    setVarFromIni( (wchar_t*)L"modkey_suspend", &suspend.modkey, pathToIni );


    startOverlay( useOverlay, fontSize );

    // set overlay to default state
    wchar_t* wcstring = new wchar_t[200];
    triggerHotkeyString( wcstring, 200, &lim3074, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 1, colorDefault);

    triggerHotkeyString( wcstring, 200, &lim3074UL, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 2, colorDefault);

    triggerHotkeyString( wcstring, 200, &lim27k, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 3, colorDefault);

    triggerHotkeyString( wcstring, 200, &lim27kUL, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 4, colorDefault);

    triggerHotkeyString( wcstring, 200, &lim30k, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 5, colorDefault);

    triggerHotkeyString( wcstring, 200, &lim7k, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 6, colorDefault);

    triggerHotkeyString( wcstring, 200, &lim_game, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 7, colorDefault);

    triggerHotkeyString( wcstring, 200, &suspend, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 8, colorDefault);

    triggerHotkeyString( wcstring, 200, &exitapp, (wchar_t*)L"" );
    updateOverlayLine( wcstring, 9, colorDefault);
    
    delete []wcstring;


    printf( "starting hotkey thread\n" );
    hThread = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)my_HotKey, (LPVOID)NULL, NULL, &dwThread );

    if ( hThread ) return WaitForSingleObject( hThread, INFINITE );
    else return 1;
    CloseHandle( hThread );
    FreeLibrary( hDLL );
    return 0;
}



__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam )
{
    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYUP ) || ( wParam == WM_KEYUP ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =  *( ( KBDLLHOOKSTRUCT* )lParam );
        int key = hooked_key.vkCode;

        if ( key == lim3074.hotkey ){
            lim3074.hotkey_down = false;
        }
        if ( key == lim3074UL.hotkey ){
            lim3074UL.hotkey_down =  false;
        }
        if ( key == lim27k.hotkey ){
            lim27k.hotkey_down = false;
        }
        if ( key == lim27kUL.hotkey ){
            lim27kUL.hotkey_down = false;
        }
        if ( key == lim30k.hotkey ){
            lim30k.hotkey_down = false;
        }
        if ( key == lim7k.hotkey ){
            lim7k.hotkey_down = false;
        }
        if ( key == lim_game.hotkey ){
            lim_game.hotkey_down = false;
        }
        if ( key == suspend.hotkey ){
            suspend.hotkey_down = false;
        }
    }


    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYDOWN ) || ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *( ( KBDLLHOOKSTRUCT* ) lParam );

        int key = hooked_key.vkCode;

        // TODO use the hotkey system properly instead of using GetAsyncState
        lim3074.modkey_state = GetAsyncKeyState( lim3074.modkey );
        lim3074UL.modkey_state = GetAsyncKeyState( lim3074UL.modkey );
        lim27k.modkey_state = GetAsyncKeyState( lim27k.modkey );
        lim27kUL.modkey_state = GetAsyncKeyState( lim27kUL.modkey );
        lim30k.modkey_state = GetAsyncKeyState( lim30k.modkey );
        lim7k.modkey_state = GetAsyncKeyState( lim7k.modkey );
        lim_game.modkey_state = GetAsyncKeyState( lim_game.modkey );
        suspend.modkey_state = GetAsyncKeyState( suspend.modkey );
        exitapp.modkey_state = GetAsyncKeyState( exitapp.modkey );

        // double cuz im lazy enough to not bitshift
        lim3074.modkey_state = GetAsyncKeyState( lim3074.modkey );
        lim3074UL.modkey_state = GetAsyncKeyState( lim3074UL.modkey );
        lim27k.modkey_state = GetAsyncKeyState( lim27k.modkey );
        lim27kUL.modkey_state = GetAsyncKeyState( lim27kUL.modkey );
        lim30k.modkey_state = GetAsyncKeyState( lim30k.modkey );
        lim7k.modkey_state = GetAsyncKeyState( lim7k.modkey );
        lim_game.modkey_state = GetAsyncKeyState( lim_game.modkey );
        suspend.modkey_state = GetAsyncKeyState( suspend.modkey );
        exitapp.modkey_state = GetAsyncKeyState( exitapp.modkey );


            
        if ( lim3074.modkey_state != 0 && key == lim3074.hotkey ) 
        {
            onTriggerHotkey(&lim3074);
            lim3074.modkey_state = 0;
        }

        if ( lim3074UL.modkey_state != 0 && key == lim3074UL.hotkey ) 
        {
            onTriggerHotkey(&lim3074UL);
            lim3074UL.modkey_state = 0;
        }

        if ( lim27k.modkey_state != 0 && key == lim27k.hotkey ) 
        {
            onTriggerHotkey(&lim27k);
            lim27k.modkey_state = 0;
        }

        if ( lim27kUL.modkey_state != 0 && key == lim27kUL.hotkey ) 
        {
            onTriggerHotkey(&lim27kUL);
            lim27kUL.modkey_state = 0;
        }

        if ( lim3074.modkey_state != 0 && key == lim30k.hotkey ) 
        {
            onTriggerHotkey(&lim30k);
            lim30k.modkey_state = 0;
        }

        if ( lim7k.modkey_state != 0 && key == lim7k.hotkey ) 
        {
            onTriggerHotkey(&lim7k);
            lim7k.modkey_state = 0;
        } 
            
        if ( lim_game.modkey_state != 0 && key == lim_game.hotkey ) 
        {
            onTriggerHotkey(&lim_game);
            lim_game.modkey_state = 0;
        }

        if ( suspend.modkey_state != 0 && key == suspend.hotkey ) 
        {
            onTriggerHotkey(&suspend);
            suspend.modkey_state = 0;
        }

        if ( exitapp.modkey_state != 0 && key == exitapp.hotkey )
        {
            wcout << "shutting down\n";
            if ( !debug ){
                ShellExecute(NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE);
                ShowWindow( GetConsoleWindow(), SW_RESTORE );
            }
            PostQuitMessage(0);
        }
    }
    return CallNextHookEx( hKeyboardHook, nCode, wParam, lParam );
}



int onTriggerHotkey(limit* limit)
{
    if (!(isD2Active() || debug))
    {
        printf("d2 not active and debug not on");
        return 1;
    }
    printf("limit.name: %ws\n", limit->name);
    limit->hotkey_down = true;
    // TODO, make it use std::unordered_map
    if (wcscmp(limit->name, L"3074") == 0){
        toggle3074(limit, colorOn, colorOff);
    } else 
    if (wcscmp(limit->name, L"3074UL") == 0){
        toggle3074_UL(limit, colorOn, colorOff);
    } else 
    if (wcscmp(limit->name, L"3074UL") == 0){
        toggle3074_UL(limit, colorOn, colorOff);
    } else 
    if (wcscmp(limit->name, L"27k") == 0){
        toggle27k(limit, colorOn, colorOff);
    } else 
    if (wcscmp(limit->name, L"27kUL") == 0){
        toggle27k_UL(limit, colorOn, colorOff);
    } else 
    if (wcscmp(limit->name, L"30k") == 0){
        toggle30k(limit, colorOn, colorOff);
    } else 
    if (wcscmp(limit->name, L"7k") == 0){
        toggle7k(limit, colorOn, colorOff);
    } else 
    if (wcscmp(limit->name, L"game") == 0){
        toggleGame(limit, colorOn, colorOff);
    } else 
    if (wcscmp(limit->name, L"suspend") == 0){
        toggleSuspend(limit, colorOn, colorOff);
    } 
    return 0;
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



void setPathToIni()
{ 
    wchar_t szFilePathSelf[MAX_PATH], szFolderPathSelf[MAX_PATH];
    GetModuleFileName(NULL, szFilePathSelf, MAX_PATH);
    wcsncpy_s( szFolderPathSelf, MAX_PATH, szFilePathSelf, (wcslen(szFilePathSelf) - wcslen(GetFileName(szFilePathSelf))));
    wchar_t filename[MAX_PATH], filePath[MAX_PATH];
    wcscpy_s( filename, MAX_PATH, L"config.ini" );
    wcscpy_s( filePath, MAX_PATH, szFolderPathSelf );
    wcscat_s( filePath, MAX_PATH, filename );
    wcsncpy_s(pathToIni, MAX_PATH, filePath, MAX_PATH);
}



void setFilterRuleString()
{
    strcpy_s( myNetRules, sizeof( myNetRules ), "(udp.DstPort < 1 and udp.DstPort > 1)" ); // set to rule that wont match anything
    if ( lim3074.state ){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)" );
    }
    if ( lim3074UL.state ){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)" ); 
    }
    if ( lim27k.state ){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)" );
    }
    if ( lim27kUL.state ){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)" );
    }
    if ( lim30k.state ){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)" );
    }
    if ( lim7k.state ){
        strcat_s( myNetRules, sizeof( myNetRules ), " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)" );
    }
    printf( "filter: %s\n", myNetRules );
}