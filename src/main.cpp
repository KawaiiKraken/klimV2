#include "main.h"

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
std::vector<int> currently_pressed_keys;

char combined_windivert_rules[1000];
wchar_t path_to_config_file[MAX_PATH];

HotkeyManager hotkeyManager(limit_ptr_vector);
UserInterface userInterface(limit_ptr_vector, path_to_config_file);
UserInterface* UserInterface::instance = &userInterface;
HotkeyManager* UserInterface::hotkeyInstance = &hotkeyManager;



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
    
    ConfigFile::SetPathToConfigFile( ( wchar_t* )L"config.txt", path_to_config_file);
    bool use_overlay;
	int font_size;
    if (ConfigFile::FileExists(path_to_config_file)) {
        ConfigFile::LoadConfig( &use_overlay, &font_size, &color_default, &color_on, &color_off, limit_ptr_vector, path_to_config_file);
    }
    userInterface.run_gui();

    ConfigFile::LoadConfig( &use_overlay, &font_size, &color_default, &color_on, &color_off, limit_ptr_vector, path_to_config_file);
    Helper::SetOverlayLineNumberOfLimits( limit_ptr_vector);
    Helper::InitializeOverlay( use_overlay, font_size, color_default, limit_ptr_vector);

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
        Helper::UnTriggerHotkeys(limit_ptr_vector, currently_pressed_keys);
    }

    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYDOWN ) || ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *( ( KBDLLHOOKSTRUCT* ) lParam );

        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);

		auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it == currently_pressed_keys.end()) {
            currently_pressed_keys.push_back(key);
		}
        Helper::TriggerHotkeys(limit_ptr_vector, currently_pressed_keys, debug, color_on, color_off, combined_windivert_rules);
    }
    return CallNextHookEx( hKeyboardHook, nCode, wParam, lParam );
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

