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
static void setOverlayLineNumberOfHotkeys(limit* limit_array[], int array_size);
void initializeOverlay(bool useOverlay, int fontSize, limit* limit_array[], int array_size);

limit lim3074(  (wchar_t*)L"3074"); 
limit lim3074UL((wchar_t*)L"3074UL");
limit lim27k(   (wchar_t*)L"27k"); 
limit lim27kUL( (wchar_t*)L"27kUL"); 
limit lim30k(   (wchar_t*)L"30k"); 
limit lim7k(    (wchar_t*)L"7k"); 
limit lim_game( (wchar_t*)L"game"); 
limit suspend(  (wchar_t*)L"suspend"); 
limit exitapp(  (wchar_t*)L"exitapp"); 
limit* limit_ptr_array[10] = { &lim3074, &lim3074UL, &lim27k, &lim27kUL, &lim30k, &lim7k, &lim_game, &suspend, &exitapp };
int size_of_limit_ptr_array = sizeof(limit_ptr_array) / sizeof(nullptr);



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
    setOverlayLineNumberOfHotkeys(limit_ptr_array, size_of_limit_ptr_array);
    initializeOverlay(useOverlay, fontSize, limit_ptr_array, size_of_limit_ptr_array);


    printf( "starting hotkey thread\n" );

    DWORD dwThread;
    hThread = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)hotkeyThread, (LPVOID)NULL, NULL, &dwThread );

    if ( hThread ) return WaitForSingleObject( hThread, INFINITE );
    else return 1;
    CloseHandle( hThread );
    FreeLibrary( hDLL );
    return 0;
}

static void setOverlayLineNumberOfHotkeys(limit* limit_array[], int array_size) {
    int currentOverlayLine = 1;
    for (int i = 0; i < (array_size-1); i++) {
        bool valid_hotkey = (limit_array[i]->hotkey != 0x0);
        bool valid_modkey = (limit_array[i]->modkey != 0x0);
        if (valid_hotkey && valid_modkey) {
            limit_array[i]->overlayLineNumber = currentOverlayLine;
            currentOverlayLine++;
        }
    }
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


void initializeOverlay(bool useOverlay, int fontSize, limit* limit_array[], int array_size) {
    startOverlay( useOverlay, fontSize );

    // set overlay to default state
    wchar_t* wcstring = new wchar_t[200];
    for (int i = 0; i < (array_size - 1); i++) {
        formatHotkeyStatusWcString( wcstring, 200, limit_array[i]);
        updateOverlayLine( wcstring, limit_array[i]->overlayLineNumber, colorDefault);
    }
    delete []wcstring;
}


void setKeyDownStateOfHotkeys(int key){
    for (int i = 0; i < (size_of_limit_ptr_array  - 1); i++) {
        if (key == limit_ptr_array[i]->hotkey) {
            limit_ptr_array[i]->hotkey_down= false;
        }
    }
}

void triggerHotkeys(int key){
    for (int i = 0; i < (size_of_limit_ptr_array - 2); i++) { // TODO make exitapp part of the global hotkey system
        if (limit_ptr_array[i]->modkey_state != 0 && key == limit_ptr_array[i]->hotkey) {
            onTriggerHotkey(limit_ptr_array[i]);
            limit_ptr_array[i]->modkey_state = 0;
        }
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
        // 2x cuz i don't want to bitshift and this gets state of key at the moment its called too
        for (int i = 0; i < 2; i++) {
            for (int i = 0; i < (size_of_limit_ptr_array - 1); i++) {
                limit_ptr_array[i]->modkey_state = GetAsyncKeyState(limit_ptr_array[i]->modkey);
            }
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