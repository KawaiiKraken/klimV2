#include "main.h"
#include "ConfigFile.h"

HINSTANCE hDLL, hDLL2;               
HANDLE gDoneEvent;
HHOOK hKeyboardHook;
HANDLE handle = NULL;
bool can_trigger_any_hotkey = TRUE;
bool debug = FALSE;

std::mutex mutex;
std::mutex* mutex_ptr = &mutex;

std::atomic<limit> lim_3074("3074"); 
std::atomic<limit> lim_3074_ul("3074UL");
std::atomic<limit> lim_27k("27k"); 
std::atomic<limit> lim_27k_ul("27kUL"); 
std::atomic<limit> lim_30k("30k"); 
std::atomic<limit> lim_7k("7k"); 
std::atomic<limit> lim_game("game"); 
std::atomic<limit> suspend("suspend"); 
std::atomic<limit> exitapp("exitapp"); 
const std::vector<std::atomic<limit>*> limit_ptr_vector = { &lim_3074, &lim_3074_ul, &lim_27k, &lim_27k_ul, &lim_30k, &lim_7k, &lim_game, &suspend, &exitapp };

typedef void (*KeyboardEventCallback)(int, bool);
std::vector<int> currently_pressed_keys;

char combined_windivert_rules[1000];
wchar_t path_to_config_file[MAX_PATH];
Settings settings;

HotkeyManager hotkeyManager(limit_ptr_vector);
UserInterface userInterface(limit_ptr_vector, path_to_config_file, &settings);
UserInterface* UserInterface::instance = &userInterface;
HotkeyManager* UserInterface::hotkeyInstance = &hotkeyManager;



DWORD WINAPI run_gui_wrapper(LPVOID lpParam) {
    UserInterface* uiinstance = static_cast<UserInterface*>(lpParam);
    uiinstance->run_gui();
    return 0;
}


int __cdecl main( int argc, char** argv ){
    std::cout << std::is_trivially_copyable<limit>::value << std::is_copy_constructible<limit>::value << std::is_move_constructible<limit>::value << std::is_copy_assignable<limit>::value << std::is_move_assignable<limit>::value << std::endl;
    limit temp_limit = lim_3074.load();
    strcpy_s(temp_limit.windivert_rule, sizeof(temp_limit.windivert_rule), " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)");
    lim_3074.store(temp_limit);
    temp_limit = lim_3074_ul.load();
    strcpy_s(temp_limit.windivert_rule, sizeof(temp_limit.windivert_rule), " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)");
    lim_3074_ul.store(temp_limit);
    temp_limit = lim_27k.load();
    strcpy_s(temp_limit.windivert_rule, sizeof(temp_limit.windivert_rule), " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)");
    lim_27k.store(temp_limit);
    temp_limit = lim_27k_ul.load();
    strcpy_s(temp_limit.windivert_rule, sizeof(temp_limit.windivert_rule), " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)");
    lim_27k_ul.store(temp_limit);
    temp_limit = lim_30k.load();
    strcpy_s(temp_limit.windivert_rule, sizeof(temp_limit.windivert_rule), " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)");
    lim_30k.store(temp_limit);
    temp_limit = lim_7k.load();
    strcpy_s(temp_limit.windivert_rule, sizeof(temp_limit.windivert_rule), " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)");
    lim_7k.store(temp_limit);

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
    
    ConfigFile::SetPathToConfigFile( ( wchar_t* )L"config.json", path_to_config_file);
    if (ConfigFile::FileExists(path_to_config_file)) {
        ConfigFile::LoadConfig( limit_ptr_vector, path_to_config_file, &settings);
    }



    userInterface.show_config = true;
    userInterface.show_overlay= false;
    DWORD dwThread;
    printf( "starting ui thread\n" );
    HANDLE hGuiThread = CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )run_gui_wrapper, &userInterface, NULL, &dwThread );


    printf( "starting hotkey thread\n" );

    HANDLE hHotkeyThread = CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )HotkeyThread, ( LPVOID )NULL, NULL, &dwThread );

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
        if (!userInterface.show_config) {
            Helper::TriggerHotkeys(limit_ptr_vector, currently_pressed_keys, debug, settings, combined_windivert_rules);
        }
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
    HHOOK hKeyboardHook = SetWindowsHookEx ( WH_KEYBOARD_LL, ( HOOKPROC ) KeyboardEvent, hInstance, NULL );
    MessageLoop();
    UnhookWindowsHookEx( hKeyboardHook );
    return 0;
}

