#include "main.h"
std::vector<int> currently_pressed_keys;

std::mutex mutex;
std::mutex* mutex_ptr = &mutex;
limit lim_3074(    ( wchar_t* )L"3074" ); 
limit lim_3074_ul( ( wchar_t* )L"3074UL" );
limit lim_27k(     ( wchar_t* )L"27k" ); 
limit lim_27k_ul(  ( wchar_t* )L"27kUL" ); 
limit lim_30k(     ( wchar_t* )L"30k" ); 
limit lim_7k(      ( wchar_t* )L"7k" ); 
limit lim_game(    ( wchar_t* )L"game" ); 
limit suspend(     ( wchar_t* )L"suspend" ); 
limit exitapp(     ( wchar_t* )L"exitapp" ); 
std::vector<limit*> limit_ptr_vector = { &lim_3074, &lim_3074_ul, &lim_27k, &lim_27k_ul, &lim_30k, &lim_7k, &lim_game, &suspend, &exitapp };

typedef void (*KeyboardEventCallback)(int, bool);

HotkeyManager hotkeyManager(limit_ptr_vector);
UserInterface userInterface(limit_ptr_vector);
UserInterface* UserInterface::instance = &userInterface;
HotkeyManager* UserInterface::hotkeyInstance = &hotkeyManager;


char combined_windivert_rules[1000];
int OnTriggerHotkey( limit* limit );
wchar_t path_to_config_file[MAX_PATH];
static void SetOverlayLineNumberOfLimits( std::vector<limit*> limit_ptr_vector);
void SetFilterRuleString(std::vector<limit*> limit_ptr_vector);
void InitializeOverlay( bool useOverlay, int fontSize, std::vector<limit*> limit_ptr_vector);





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

    if ( !Helper::RunningAsAdmin() ){
        MessageBox( NULL, ( LPCWSTR )L"ERROR: not running as admin", ( LPCWSTR )L"ERROR", MB_ICONERROR | MB_DEFBUTTON2 );
        return 0;
    }

    
    SetPathToConfigFile( ( wchar_t* )L"config.txt" );
    if ( !ConfigFile::FileExists( path_to_config_file ) ){
        userInterface.run_gui();
        ConfigFile::WriteConfig(limit_ptr_vector, path_to_config_file);
    }

    bool use_overlay;
	int font_size;
    ConfigFile::LoadConfig( &use_overlay, &font_size, &color_default, &color_on, &color_off, limit_ptr_vector, path_to_config_file);
    SetOverlayLineNumberOfLimits( limit_ptr_vector);
    InitializeOverlay( use_overlay, font_size, limit_ptr_vector);


    printf( "starting hotkey thread\n" );

    DWORD dwThread;
    hHotkeyThread = CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )HotkeyThread, ( LPVOID )NULL, NULL, &dwThread );

    if ( hHotkeyThread ){
        return WaitForSingleObject( hHotkeyThread, INFINITE );
    }
    else {
        if (hHotkeyThread != 0) {
            CloseHandle( hHotkeyThread );
        }
		FreeLibrary( hDLL );
        return 1;
    }
    return 0;
}






static void SetOverlayLineNumberOfLimits( std::vector<limit*> limit_ptr_vector){
    int current_overlay_line = 1;
    for ( int i = 0; i < limit_ptr_vector.size(); i++) {
        if (limit_ptr_vector[i]->key_list[0] != undefined_key) {
			limit_ptr_vector[i]->overlay_line_number = current_overlay_line;
			current_overlay_line++;
		}
    }
}




void InitializeOverlay( bool use_overlay, int font_size, std::vector<limit*> limit_ptr_vector){
    startOverlay( use_overlay, font_size );

    // set overlay to default state
    wchar_t* wc_string = new wchar_t[200];
    for ( int i = 0; i < limit_ptr_vector.size(); i++ ){
        if (limit_ptr_vector[i]->overlay_line_number != -1) {
            Limit::FormatHotkeyStatusWcString(wc_string, 200, limit_ptr_vector[i]);
            UpdateOverlayLine(wc_string, limit_ptr_vector[i]->overlay_line_number, color_default);
        }
    }
    delete []wc_string;
}


void UnTriggerHotkeys( std::vector<limit*> limit_ptr_vector) {
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        // Sort both vectors
        std::sort(limit_ptr_vector[i]->key_list.begin(), limit_ptr_vector[i]->key_list.end());
        std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), limit_ptr_vector[i]->key_list.begin(), limit_ptr_vector[i]->key_list.end());

        if (!containsAll) {
            limit_ptr_vector[i]->triggered = false;
        }
    }
}


void TriggerHotkeys( std::vector<limit*> limit_ptr_vector){
    for ( int i = 0; i < limit_ptr_vector.size(); i++ ){
          // Sort both vectors
        std::sort(limit_ptr_vector[i]->key_list.begin(), limit_ptr_vector[i]->key_list.end());
		std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), limit_ptr_vector[i]->key_list.begin(), limit_ptr_vector[i]->key_list.end());

        if (containsAll) {
            OnTriggerHotkey(limit_ptr_vector[i]);
        }
    }
}


__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam ){
    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYUP ) || ( wParam == WM_KEYUP ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =  *( ( KBDLLHOOKSTRUCT* )lParam );

        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
        std::cout << "key recv: " << key << std::endl;


		// Check if the vector contains the target element
		auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it != currently_pressed_keys.end()) {
            currently_pressed_keys.erase(it);
		}
        UnTriggerHotkeys(limit_ptr_vector);
    }


    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYDOWN ) || ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *( ( KBDLLHOOKSTRUCT* ) lParam );

        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);

		auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it == currently_pressed_keys.end()) {
            currently_pressed_keys.push_back(key);
		}
        TriggerHotkeys(limit_ptr_vector);
    }
    return CallNextHookEx( hKeyboardHook, nCode, wParam, lParam );
}




int OnTriggerHotkey( limit* limit ){
    if ( wcscmp( limit->name, L"exitapp" ) == 0 ){
        Helper::Exitapp(debug);
	} 
    if ( !( Helper::D2Active() || debug ) )
    {
        printf("hotkey ignored: d2 is not the active window and debug mode is not on");
        return 1;
    }
    if ( !limit->triggered ) {
		limit->triggered = true;
        if ( wcscmp( limit->name, L"game" ) == 0){
			Limit::ToggleWholeGameLimit( limit, color_on, color_off );
		} 
        else if ( wcscmp( limit->name, L"suspend") == 0 ){
			Limit::ToggleSuspend( limit, color_on, color_off );
		} 
        else {
			Limit::ToggleBlockingLimit( limit, color_on, color_off );
        }
        printf( "state of %ws: %s\n", limit->name, limit->state ? "true" : "false" );
        SetFilterRuleString( limit_ptr_vector );
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
    wcsncpy_s( folder_path_self, MAX_PATH, file_path_self, ( wcslen( file_path_self ) - wcslen( Helper::GetFilename( file_path_self ) ) ) );
    wchar_t filename[MAX_PATH], file_path[MAX_PATH];
    wcscpy_s( filename, MAX_PATH, config_filename );
    wcscpy_s( file_path, MAX_PATH, folder_path_self );
    wcscat_s( file_path, MAX_PATH, filename );
    wcsncpy_s( path_to_config_file, MAX_PATH, file_path, MAX_PATH );
}



void SetFilterRuleString( std::vector<limit*> limit_ptr_vector){
    strcpy_s( combined_windivert_rules, sizeof( combined_windivert_rules ), "(udp.DstPort < 1 and udp.DstPort > 1)" ); // set to rule that wont match anything

    for ( int i = 0; i < limit_ptr_vector.size(); i++ ){
        if ( strcmp( limit_ptr_vector[i]->windivert_rule, "" ) != 0 ){
            if ( limit_ptr_vector[i]->state ){
                strcat_s( combined_windivert_rules, sizeof( combined_windivert_rules ), limit_ptr_vector[i]->windivert_rule );
            }
        }
    }
    printf( "filter: %s\n", combined_windivert_rules );
}
