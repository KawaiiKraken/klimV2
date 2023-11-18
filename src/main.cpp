// TODO comments
#include "main.h"
#include "helperFunctions.h"
#include "krekens_overlay.h"
#include "..\jsoncpp_x64-windows\include\json\json.h"

using namespace std;

char myNetRules[1000];
int onTriggerHotkey(limit* limit);
wchar_t pathToConfigFile[MAX_PATH];
void loadConfig(bool* useOverlay, int* fontSize, limit* exitapp, limit* lim3074, limit* lim3074UL, limit* lim27k, limit* lim27kUL, limit* lim30k, limit* lim7k, limit* lim_game, limit* suspend);

limit lim3074(  (wchar_t*)L"3074",    1); 
limit lim3074UL((wchar_t*)L"3074UL",  2);
limit lim27k(   (wchar_t*)L"27k",     3); 
limit lim27kUL( (wchar_t*)L"27kUL",   4); 
limit lim30k(   (wchar_t*)L"30k",     5); 
limit lim7k(    (wchar_t*)L"7k",      6); 
limit lim_game( (wchar_t*)L"game",    7); 
limit suspend(  (wchar_t*)L"suspend", 8); 
limit exitapp(  (wchar_t*)L"exitapp", 9); 



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

    
    // config file stuff
    setPathToConfigFile(); 
    if ( !FileExists( pathToConfigFile ) ){
        writeDefaultJsonConfig( pathToConfigFile );
    }

    bool useOverlay;
	int fontSize;
    loadConfig(&useOverlay, &fontSize, &exitapp, &lim3074, &lim3074UL, &lim27k, &lim27kUL, &lim30k, &lim7k, &lim_game, &suspend);
    initializeOverlay(useOverlay, fontSize);


    printf( "starting hotkey thread\n" );

    DWORD dwThread;
    hThread = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)hotkeyThread, (LPVOID)NULL, NULL, &dwThread );

    if ( hThread ) return WaitForSingleObject( hThread, INFINITE );
    else return 1;
    CloseHandle( hThread );
    FreeLibrary( hDLL );
    return 0;
}



void loadConfig(bool* useOverlay, int* fontSize, limit* exitapp, limit* lim3074, limit* lim3074UL, limit* lim27k, limit* lim27kUL, limit* lim30k, limit* lim7k, limit* lim_game, limit* suspend){
    // Load the config from the JSON file
    Json::Value loadedConfig = loadConfigFileFromJson(pathToConfigFile);
    setVarFromJson(exitapp,   loadedConfig["hotkey_exitapp"].asString(),   loadedConfig["modkey_exitapp"].asString());
    setVarFromJson(lim3074,   loadedConfig["hotkey_3074"].asString(),      loadedConfig["modkey_3074"].asString());
    setVarFromJson(lim3074UL, loadedConfig["hotkey_3074UL"].asString(),    loadedConfig["modkey_3074UL"].asString());
    setVarFromJson(lim27k,    loadedConfig["hotkey_27k"].asString(),       loadedConfig["modkey_27k"].asString());
    setVarFromJson(lim27kUL,  loadedConfig["hotkey_27kUL"].asString(),     loadedConfig["modkey_27kUL"].asString());
    setVarFromJson(lim30k,    loadedConfig["hotkey_30k"].asString(),       loadedConfig["modkey_30k"].asString());
    setVarFromJson(lim7k,     loadedConfig["hotkey_7k"].asString(),        loadedConfig["modkey_7k"].asString());
    setVarFromJson(lim_game,  loadedConfig["hotkey_game"].asString(),      loadedConfig["modkey_game"].asString());
    setVarFromJson(suspend,   loadedConfig["hotkey_suspend"].asString(),   loadedConfig["modkey_suspend"].asString());

    colorDefault = stol(loadedConfig["colorDefault"].asString(), NULL, 16);
    colorOn      = stol(loadedConfig["colorOn"].asString(), NULL, 16);
    colorOff     = stol(loadedConfig["colorOff"].asString(), NULL, 16);

    *useOverlay = loadedConfig["useOverlay"].asBool();
    *fontSize = loadedConfig["fontSize"].asInt();
}


void initializeOverlay(bool useOverlay, int fontSize){
    startOverlay( useOverlay, fontSize );

    // set overlay to default state
    wchar_t* wcstring = new wchar_t[200];
    formatHotkeyStatusWcString( wcstring, 200, &lim3074);
    updateOverlayLine( wcstring, lim3074.overlayLineNumber, colorDefault);

    formatHotkeyStatusWcString( wcstring, 200, &lim3074UL);
    updateOverlayLine( wcstring, lim3074UL.overlayLineNumber, colorDefault);

    formatHotkeyStatusWcString( wcstring, 200, &lim27k);
    updateOverlayLine( wcstring, lim27k.overlayLineNumber, colorDefault);

    formatHotkeyStatusWcString( wcstring, 200, &lim27kUL);
    updateOverlayLine( wcstring, lim27kUL.overlayLineNumber, colorDefault);

    formatHotkeyStatusWcString( wcstring, 200, &lim30k);
    updateOverlayLine( wcstring, lim30k.overlayLineNumber, colorDefault);

    formatHotkeyStatusWcString( wcstring, 200, &lim7k);
    updateOverlayLine( wcstring, lim7k.overlayLineNumber, colorDefault);

    formatHotkeyStatusWcString( wcstring, 200, &lim_game);
    updateOverlayLine( wcstring, lim_game.overlayLineNumber, colorDefault);

    formatHotkeyStatusWcString( wcstring, 200, &suspend);
    updateOverlayLine( wcstring, suspend.overlayLineNumber, colorDefault);

    formatHotkeyStatusWcString( wcstring, 200, &exitapp);
    updateOverlayLine( wcstring, exitapp.overlayLineNumber, colorDefault);
    
    delete []wcstring;
}


void setKeyDownStateOfHotkeys(int key){
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

void triggerHotkeys(int key){
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


__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam )
{
    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYUP ) || ( wParam == WM_KEYUP ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =  *( ( KBDLLHOOKSTRUCT* )lParam );
        int key = hooked_key.vkCode;
        setKeyDownStateOfHotkeys(key);
    }


    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYDOWN ) || ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *( ( KBDLLHOOKSTRUCT* ) lParam );

        int key = hooked_key.vkCode;

        // TODO use the hotkey system properly instead of using GetAsyncState
        // 2x cuz i dont want to bitshift and this gets state of key at the moment its called too
        for (int i = 0; i < 2; i++) {
            lim3074.modkey_state = GetAsyncKeyState( lim3074.modkey );
			lim3074UL.modkey_state = GetAsyncKeyState( lim3074UL.modkey );
			lim27k.modkey_state = GetAsyncKeyState( lim27k.modkey );
			lim27kUL.modkey_state = GetAsyncKeyState( lim27kUL.modkey );
			lim30k.modkey_state = GetAsyncKeyState( lim30k.modkey );
			lim7k.modkey_state = GetAsyncKeyState( lim7k.modkey );
			lim_game.modkey_state = GetAsyncKeyState( lim_game.modkey );
			suspend.modkey_state = GetAsyncKeyState( suspend.modkey );
			exitapp.modkey_state = GetAsyncKeyState( exitapp.modkey );
        }

        triggerHotkeys(key);
    }
    return CallNextHookEx( hKeyboardHook, nCode, wParam, lParam );
}



int onTriggerHotkey(limit* limit)
{
    if (!(isD2Active() || debug))
    {
        printf("hotkey ignored: d2 is not the active window and debug mode is not on");
        return 1;
    }
    if (!limit->hotkey_down) {
		limit->hotkey_down = true;
		if (wcscmp(limit->name, L"game"   ) == 0){
			toggleWholeGameLimit(limit, colorOn, colorOff);
		} else 
		if (wcscmp(limit->name, L"suspend") == 0){
			toggleSuspend(limit, colorOn, colorOff);
		}
		else {
			toggleBlockingLimit(limit, colorOn, colorOff);
        }
        printf( "state of %ws: %s\n", limit->name, limit->state ? "true" : "false" );
        setFilterRuleString();
        updateFilter(myNetRules);
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



DWORD WINAPI hotkeyThread( LPVOID lpParm )
{
    HINSTANCE hInstance = GetModuleHandle( NULL);
    if ( !hInstance ) hInstance = LoadLibrary( ( LPCWSTR ) lpParm ); 
    if ( !hInstance ) return 1;

    hKeyboardHook = SetWindowsHookEx ( WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent, hInstance, NULL );
    MessageLoop();
    UnhookWindowsHookEx( hKeyboardHook );
    return 0;
}



void setPathToConfigFile()
{ 
    wchar_t szFilePathSelf[MAX_PATH], szFolderPathSelf[MAX_PATH];
    GetModuleFileName(NULL, szFilePathSelf, MAX_PATH);
    wcsncpy_s( szFolderPathSelf, MAX_PATH, szFilePathSelf, (wcslen(szFilePathSelf) - wcslen(GetFileName(szFilePathSelf))));
    wchar_t filename[MAX_PATH], filePath[MAX_PATH];
    wcscpy_s( filename, MAX_PATH, L"config.txt" );
    wcscpy_s( filePath, MAX_PATH, szFolderPathSelf );
    wcscat_s( filePath, MAX_PATH, filename );
    wcsncpy_s(pathToConfigFile, MAX_PATH, filePath, MAX_PATH);
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