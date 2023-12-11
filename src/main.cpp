// TODO comments
#include "main.h"
#include "helperFunctions.h"
#include "krekens_overlay.h"

using namespace std;

char combined_windivert_rules[1000];
int OnTriggerHotkey( limit* limit );
wchar_t path_to_config_file[MAX_PATH];
void LoadConfig( bool* useOverlay, int* fontSize, limit* limit_array[], int array_size );
static void SetOverlayLineNumberOfHotkeys( limit* limit_array[], int array_size );
void SetFilterRuleString( limit* limit_array[], int array_size );
void InitializeOverlay( bool useOverlay, int fontSize, limit* limit_array[], int array_size );

limit lim_3074(    ( wchar_t* )L"3074" ); 
limit lim_3074_ul( ( wchar_t* )L"3074UL" );
limit lim_27k(     ( wchar_t* )L"27k" ); 
limit lim_27k_ul(  ( wchar_t* )L"27kUL" ); 
limit lim_30k(     ( wchar_t* )L"30k" ); 
limit lim_7k(      ( wchar_t* )L"7k" ); 
limit lim_game(    ( wchar_t* )L"game" ); 
limit suspend(     ( wchar_t* )L"suspend" ); 
limit exitapp(     ( wchar_t* )L"exitapp" ); 
limit* limit_ptr_array[] = { &lim_3074, &lim_3074_ul, &lim_27k, &lim_27k_ul, &lim_30k, &lim_7k, &lim_game, &suspend, &exitapp };
int size_of_limit_ptr_array = sizeof(limit_ptr_array) / sizeof(limit_ptr_array[0]);

// TODO make exitapp work when d2 is not the active window


int __cdecl main( int argc, char** argv ){
    strcpy_s( lim_3074.windivert_rule,    sizeof( lim_3074.windivert_rule ),    " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)" ); 
	strcpy_s( lim_3074_ul.windivert_rule, sizeof( lim_3074_ul.windivert_rule ), " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)" ); 
	strcpy_s( lim_27k.windivert_rule,     sizeof( lim_27k.windivert_rule ),     " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)" ); 
	strcpy_s( lim_27k_ul.windivert_rule,  sizeof( lim_27k_ul.windivert_rule ),  " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)" ); 
	strcpy_s( lim_30k.windivert_rule,     sizeof( lim_30k.windivert_rule ),     " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)" ); 
	strcpy_s( lim_7k.windivert_rule,      sizeof( lim_7k.windivert_rule ),      " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)" ); 

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

    if ( !RunningAsAdmin() ){
        MessageBox( NULL, ( LPCWSTR )L"ERROR: not running as admin", ( LPCWSTR )L"ERROR", MB_ICONERROR | MB_DEFBUTTON2 );
        return 0;
    }

    
    SetPathToConfigFile( ( wchar_t* )L"config.txt" );
    if ( !FileExists( path_to_config_file ) ){
        WriteDefaultJsonConfig( path_to_config_file );
    }

    bool use_overlay;
	int font_size;
    LoadConfig( &use_overlay, &font_size, limit_ptr_array, size_of_limit_ptr_array );
    SetOverlayLineNumberOfHotkeys( limit_ptr_array, size_of_limit_ptr_array );
    InitializeOverlay( use_overlay, font_size, limit_ptr_array, size_of_limit_ptr_array );


    printf( "starting hotkey thread\n" );

    // TODO remove dwThread
    DWORD dwThread;
    hHotkeyThread = CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )HotkeyThread, ( LPVOID )NULL, NULL, &dwThread );

    if ( hHotkeyThread ){
        return WaitForSingleObject( hHotkeyThread, INFINITE );
    }
    else {
        return 1;
    }
    CloseHandle( hHotkeyThread );
    FreeLibrary( hDLL );
    return 0;
}



static void SetOverlayLineNumberOfHotkeys( limit* limit_ptr_array[], int size_of_limit_ptr_array ){
    int current_overlay_line = 1;
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        bool valid_hotkey = ( limit_ptr_array[i]->hotkey != undefined_key );
        bool valid_modkey = ( limit_ptr_array[i]->modkey != undefined_key );
        if ( valid_hotkey || valid_modkey ){
            limit_ptr_array[i]->overlay_line_number = current_overlay_line;
            current_overlay_line++;
        }
    }
}



void LoadConfig( bool* use_overlay, int* font_size, limit* limit_ptr_array[], int size_of_limit_ptr_array ){
    // Load the config from the JSON file
    Json::Value loaded_config = LoadConfigFileFromJson( path_to_config_file );
    // this could definitely be done programmatically but i think that would be more effort than its worth
    SetVarFromJson( limit_ptr_array[0], loaded_config["hotkey_3074"].asString(),    loaded_config["modkey_3074"].asString() );
    SetVarFromJson( limit_ptr_array[1], loaded_config["hotkey_3074_ul"].asString(), loaded_config["modkey_3074_ul"].asString() );
    SetVarFromJson( limit_ptr_array[2], loaded_config["hotkey_27k"].asString(),     loaded_config["modkey_27k"].asString() );
    SetVarFromJson( limit_ptr_array[3], loaded_config["hotkey_27k_ul"].asString(),  loaded_config["modkey_27k_ul"].asString() );
    SetVarFromJson( limit_ptr_array[4], loaded_config["hotkey_30k"].asString(),     loaded_config["modkey_30k"].asString() );
    SetVarFromJson( limit_ptr_array[5], loaded_config["hotkey_7k"].asString(),      loaded_config["modkey_7k"].asString() );
    SetVarFromJson( limit_ptr_array[6], loaded_config["hotkey_game"].asString(),    loaded_config["modkey_game"].asString() );
    SetVarFromJson( limit_ptr_array[7], loaded_config["hotkey_suspend"].asString(), loaded_config["modkey_suspend"].asString() );
    SetVarFromJson( limit_ptr_array[8], loaded_config["hotkey_exitapp"].asString(), loaded_config["modkey_exitapp"].asString() );

    color_default = stol( loaded_config["color_default"].asString(), NULL, 16 );
    color_on      = stol( loaded_config["color_on"].asString(),      NULL, 16 );
    color_off     = stol( loaded_config["color_off"].asString(),     NULL, 16 );

    *use_overlay  = loaded_config["use_overlay"].asBool();
    *font_size    = loaded_config["font_size"].asInt();
}



void InitializeOverlay( bool use_overlay, int font_size, limit* limit_ptr_array[], int size_of_limit_ptr_array ){
    startOverlay( use_overlay, font_size );

    // set overlay to default state
    wchar_t* wc_string = new wchar_t[200];
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        FormatHotkeyStatusWcString( wc_string, 200, limit_ptr_array[i] );
        UpdateOverlayLine( wc_string, limit_ptr_array[i]->overlay_line_number, color_default );
    }
    delete []wc_string;
}



void HotkeyResetKeyDownState( int key ){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        if ( key == (int)limit_ptr_array[i]->hotkey ){
            limit_ptr_array[i]->hotkey_down= false;
        }
    }
}



void ModkeyResetKeyDownState(int key){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        if ( key == limit_ptr_array[i]->modkey ){
            limit_ptr_array[i]->modkey_down = false;
        }
    }
}



void ModkeySetDownState( int key ){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        if ( key == limit_ptr_array[i]->modkey || limit_ptr_array[i]->modkey == undefined_key ){
            limit_ptr_array[i]->modkey_down = true;
        }
    }
}



void TriggerHotkeys( int key ){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){ 
        if ( limit_ptr_array[i]->modkey_down == true && key == limit_ptr_array[i]->hotkey ){
            OnTriggerHotkey( limit_ptr_array[i] );
        }
    }
}



__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam ){
    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYUP ) || ( wParam == WM_KEYUP ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =  *( ( KBDLLHOOKSTRUCT* )lParam );

        int key = hooked_key.vkCode;

        ModkeyResetKeyDownState( key );
        HotkeyResetKeyDownState( key );
    }


    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYDOWN ) || ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *( ( KBDLLHOOKSTRUCT* ) lParam );

        int key = hooked_key.vkCode;

        ModkeySetDownState( key );
        TriggerHotkeys( key );
    }
    return CallNextHookEx( hKeyboardHook, nCode, wParam, lParam );
}



void Exitapp(){
    wcout << "shutting down\n";
    if ( !debug ){
		ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE );
		ShowWindow( GetConsoleWindow(), SW_RESTORE );
		}
	PostQuitMessage( 0 );
}



int OnTriggerHotkey( limit* limit ){
    if ( !( D2Active() || debug ) )
    {
        printf("hotkey ignored: d2 is not the active window and debug mode is not on");
        return 1;
    }
    if ( !limit->hotkey_down ) {
		limit->hotkey_down = true;
		if ( wcscmp( limit->name, L"exitapp" ) == 0 ){
            Exitapp();
		} 
        else if ( wcscmp( limit->name, L"game" ) == 0){
			ToggleWholeGameLimit( limit, color_on, color_off );
		} 
        else if ( wcscmp( limit->name, L"suspend") == 0 ){
			ToggleSuspend( limit, color_on, color_off );
		} 
        else {
			ToggleBlockingLimit( limit, color_on, color_off );
        }
        printf( "state of %ws: %s\n", limit->name, limit->state ? "true" : "false" );
        SetFilterRuleString( limit_ptr_array, size_of_limit_ptr_array );
        UpdateFilter( combined_windivert_rules );
    }
    return 0;
}



void MessageLoop(){
    MSG message;
    while ( GetMessage( &message, NULL, 0, 0 ) ){
        TranslateMessage( &message );
        DispatchMessage( &message );
    }
}



DWORD WINAPI HotkeyThread( LPVOID lpParam ){
    HINSTANCE hInstance = GetModuleHandle( NULL );
    if ( !hInstance ){
        hInstance = LoadLibrary( ( LPCWSTR )lpParam );
    }
    if ( !hInstance ){ 
        return 1; 
    }
    hKeyboardHook = SetWindowsHookEx ( WH_KEYBOARD_LL, ( HOOKPROC ) KeyboardEvent, hInstance, NULL );
    MessageLoop();
    UnhookWindowsHookEx( hKeyboardHook );
    return 0;
}



void SetPathToConfigFile( wchar_t* config_filename ){ 
    wchar_t file_path_self[MAX_PATH], folder_path_self[MAX_PATH];
    GetModuleFileName( NULL, file_path_self, MAX_PATH );
    wcsncpy_s( folder_path_self, MAX_PATH, file_path_self, ( wcslen( file_path_self ) - wcslen( GetFilename( file_path_self ) ) ) );
    wchar_t filename[MAX_PATH], file_path[MAX_PATH];
    wcscpy_s( filename, MAX_PATH, config_filename );
    wcscpy_s( file_path, MAX_PATH, folder_path_self );
    wcscat_s( file_path, MAX_PATH, filename );
    wcsncpy_s( path_to_config_file, MAX_PATH, file_path, MAX_PATH );
}



void SetFilterRuleString( limit* limit_array[], int array_size ){
    strcpy_s( combined_windivert_rules, sizeof( combined_windivert_rules ), "(udp.DstPort < 1 and udp.DstPort > 1)" ); // set to rule that wont match anything

    for ( int i = 0; i < array_size; i++ ){
        if ( strcmp( limit_array[i]->windivert_rule, "" ) != 0 ){
            if ( limit_array[i]->state ){
                strcat_s( combined_windivert_rules, sizeof( combined_windivert_rules ), limit_array[i]->windivert_rule );
            }
        }
    }
    printf( "filter: %s\n", combined_windivert_rules );
}
