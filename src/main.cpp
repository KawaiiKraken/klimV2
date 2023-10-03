// TODO comments
#define _WIN32_WINNT_WIN10 0x0A00 // Windows 10
#define PHNT_VERSION PHNT_THRESHOLD // Windows 10
                                    //
#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "advapi32.lib" )
#pragma comment( lib, "Psapi.lib" )
#pragma comment( lib, "ntdll.lib" )

// these 2 are for the phnt header files
#pragma clang diagnostic ignored "-Wpragma-pack"
#pragma clang diagnostic ignored "-Wmicrosoft-enum-forward-reference"

#pragma clang diagnostic ignored "-Wunused-variable"
//#pragma clang diagnostic ignored "-Wwritable-strings"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <stdbool.h>
#include <iostream>
#include <wchar.h>
#include <shellapi.h>
#include <psapi.h>

#include "../phnt/phnt_windows.h"
#include "../phnt/phnt.h"
#include "../WinDivert/windivert.h"
#include "helperFunctions.h"
#include "main.h"



using namespace std;




#define ntohs(x)            WinDivertHelperNtohs(x)
#define ntohl(x)            WinDivertHelperNtohl(x)
#define htons(x)            WinDivertHelperHtons(x)
#define htonl(x)            WinDivertHelperHtonl(x)

#define MAXBUF              0xFFFF
#define INET6_ADDRSTRLEN    45
#define IPPROTO_ICMPV6      58

/*
 * Pre-fabricated packets.
 */
typedef struct
{
    WINDIVERT_IPHDR ip;
    WINDIVERT_TCPHDR tcp;
} TCPPACKET, *PTCPPACKET;

typedef struct
{
    WINDIVERT_IPV6HDR ipv6;
    WINDIVERT_TCPHDR tcp;
} TCPV6PACKET, *PTCPV6PACKET;

typedef struct
{
    WINDIVERT_IPHDR ip;
    WINDIVERT_ICMPHDR icmp;
    UINT8 data[];
} ICMPPACKET, *PICMPPACKET;

typedef struct
{
    WINDIVERT_IPV6HDR ipv6;
    WINDIVERT_ICMPV6HDR icmpv6;
    UINT8 data[];
} ICMPV6PACKET, *PICMPV6PACKET;

/*
 * Prototypes.
 */
static void PacketIpInit( PWINDIVERT_IPHDR packet );
static void PacketIpTcpInit( PTCPPACKET packet );
static void PacketIpIcmpInit( PICMPPACKET packet );
static void PacketIpv6Init( PWINDIVERT_IPV6HDR packet );
static void PacketIpv6TcpInit( PTCPV6PACKET packet );
static void PacketIpv6Icmpv6Init( PICMPV6PACKET packet );




void startFilter(){
    hThread = CreateThread( NULL, 0, block_traffic, NULL, 0, NULL );
    can_trigger_any_hotkey = TRUE;
    printf( "hotkeys re-enabled\n" );
}

bool hotkey_3074_keydown = FALSE;
bool hotkey_3074_UL_keydown = FALSE;
bool hotkey_27k_keydown = FALSE;
bool hotkey_27k_UL_keydown = FALSE;
bool hotkey_30k_keydown = FALSE;
bool hotkey_7k_keydown = FALSE;
bool hotkey_game_keydown = FALSE;
bool hotkey_suspend_keydown = FALSE;

__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam )
{
    DWORD modkey_3074_state=0;
    DWORD modkey_3074_UL_state=0;
    DWORD modkey_27k_state=0;
    DWORD modkey_27k_UL_state=0;
    DWORD modkey_30k_state=0;
    DWORD modkey_7k_state=0;
    DWORD modkey_game =0;
    DWORD modkey_suspend_state=0;
    DWORD modkey_exitapp_state=0;

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
                hotkey_3074_keydown = FALSE;
            }
            if ( key == hotkey_3074_UL ){
                hotkey_3074_UL_keydown = FALSE;
            }
            if ( key == hotkey_27k){
                hotkey_27k_keydown = FALSE;
            }
            if ( key == hotkey_27k_UL){
                hotkey_27k_UL_keydown = FALSE;
            }
            if ( key == hotkey_30k){
                hotkey_30k_keydown = FALSE;
            }
            if ( key == hotkey_7k){
                hotkey_7k_keydown = FALSE;
            }
            if ( key == hotkey_game){
                hotkey_game_keydown = FALSE;
            }
            if ( key == hotkey_suspend){
                hotkey_suspend_keydown = FALSE;
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

        DWORD modkey_3074_state = 0;
        DWORD modkey_3074_UL_state = 0;
        DWORD modkey_27k_state = 0;
        DWORD modkey_27k_UL_state = 0;
        DWORD modkey_30k_state = 0;
        DWORD modkey_7k_state = 0;
        DWORD modkey_game_state = 0;
        DWORD modkey_suspend_state = 0;
        DWORD modkey_exitapp_state = 0;

        //if (key >= 'A' && key <= 'Z')   
        //if ( key != ( modkey_3074 | modkey_3074_UL | modkey_27k | modkey_30k | modkey_7k | modkey_suspend | modkey_exitapp ) )   // this might be a bit broken or unneeded
        if ( TRUE )   // this might be a bit broken or unneeded
        {
            // TODO use the hotkey system properly instead of using GetAsyncState
            modkey_3074_state = GetAsyncKeyState( modkey_3074 );
            modkey_3074_UL_state = GetAsyncKeyState( modkey_3074_UL );
            modkey_27k_state = GetAsyncKeyState( modkey_27k );
            modkey_27k_UL_state = GetAsyncKeyState( modkey_27k_UL );
            modkey_30k_state = GetAsyncKeyState( modkey_30k );
            modkey_7k_state = GetAsyncKeyState( modkey_7k );
            modkey_game_state = GetAsyncKeyState( modkey_suspend );
            modkey_suspend_state = GetAsyncKeyState( modkey_suspend );
            modkey_exitapp_state = GetAsyncKeyState( modkey_exitapp );

            // double cuz im lazy enough to not bitshift
            modkey_3074_state = GetAsyncKeyState( modkey_3074 );
            modkey_3074_UL_state = GetAsyncKeyState( modkey_3074_UL );
            modkey_27k_UL_state = GetAsyncKeyState( modkey_27k_UL );
            modkey_30k_state = GetAsyncKeyState( modkey_30k );
            modkey_7k_state = GetAsyncKeyState( modkey_7k );
            modkey_game_state = GetAsyncKeyState( modkey_suspend );
            modkey_suspend_state = GetAsyncKeyState( modkey_suspend );
            modkey_exitapp_state = GetAsyncKeyState( modkey_exitapp );


            
            // ============= 3074 ================
            if ( modkey_3074_state !=0 && key == hotkey_3074 ) 
            {
                wcout << L"hotkey_3074 detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !hotkey_3074_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        hotkey_3074_keydown = TRUE;
                        toggle3074();
                        combinerules();
                        startFilter();
                    }
                }
                modkey_3074_state=0;
            }

            // ============= 3074UL ================
            if ( modkey_3074_UL_state !=0 && key == hotkey_3074_UL ) 
            {
                wcout << L"hotkey_3074 detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !hotkey_3074_UL_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        hotkey_3074_UL_keydown = TRUE;
                        toggle3074_UL();
                        combinerules();
                        startFilter();
                    }
                }
                modkey_3074_UL_state=0;
            }

            // ============= 27k ================
            if ( modkey_27k_state !=0 && key == hotkey_27k ) 
            {
                wcout << L"hotkey_27k detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !hotkey_27k_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        hotkey_27k_keydown = TRUE;
                        toggle27k();
                        combinerules();
                        startFilter();
                    }
                }
                modkey_27k_state=0;
            }

            // ============= 27k_UL ================
            if ( modkey_27k_UL_state !=0 && key == hotkey_27k_UL ) 
            {
                wcout << L"hotkey_27k detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !hotkey_27k_UL_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        hotkey_27k_UL_keydown = TRUE;
                        toggle27k_UL();
                        combinerules();
                        startFilter();
                    }
                }
                modkey_27k_UL_state=0;
            }


            // ============= 30k ================
            if ( modkey_30k_state !=0 && key == hotkey_30k ) 
            {
                wcout << L"hotkey_30k detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !hotkey_30k_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        hotkey_30k_keydown = TRUE;
                        toggle30k();
                        combinerules();
                        startFilter();
                    }
                }
                modkey_30k_state=0;
            }

            // ============= 7k ================
            if ( modkey_7k_state !=0 && key == hotkey_7k ) 
            {
                wcout << L"hotkey_7k detected\n";
                if ( isD2Active() | debug ){
                    if ( can_trigger_any_hotkey && !hotkey_7k_keydown ){ 
                        can_trigger_any_hotkey = FALSE;
                        hotkey_7k_keydown = TRUE;
                        toggle7k();
                        combinerules();
                        startFilter();
                    }
                }
                modkey_7k_state=0;
            } 
            
            // ============= game ================
            if ( modkey_game_state !=0 && key == hotkey_game ) 
            {
                wcout << L"hotkey_game detected\n";
                if ( isD2Active() | debug ){
                    if ( !hotkey_game_keydown ){ 
                        hotkey_game_keydown = TRUE;
                        toggleGame();
                    }
                }
                modkey_game_state=0;
            }


            // ============= suspend ================
            if ( modkey_suspend_state !=0 && key == hotkey_suspend ) 
            {
                wcout << L"hotkey_suspend detected\n";
                if ( isD2Active() | debug ){
                    if ( !hotkey_suspend_keydown ){
                        hotkey_suspend_keydown = TRUE;
                        toggleSuspend();
                    }
                }
                modkey_suspend_state=0;
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

void triggerHotkeyString( wchar_t* wcstring, int szWcstring, char hotkey, char modkey, wchar_t* action, wchar_t* state ){ // TODO better name for this
    char charbuf[2];
    char charbuf2[2];
    charbuf[0] = hotkey;
    charbuf[1] = '\0'; // has to be null terminated for strlen to work 
    charbuf2[0] = modkey;
    charbuf2[1] = '\0'; // has to be null terminated for strlen to work 
    
    int szCharbuf = strlen( charbuf ) + 1;
    int szCharbuf2 = strlen( charbuf2 ) + 1;
    wchar_t* wcstringbuf = new wchar_t[szCharbuf];
    wchar_t* wcstringbuf2 = new wchar_t[szCharbuf2];
    size_t outSize;

    mbstowcs_s( &outSize, wcstringbuf, szCharbuf, charbuf, szCharbuf-1 );
    mbstowcs_s( &outSize, wcstringbuf2, szCharbuf2, charbuf2, szCharbuf2-1 );
    wcscpy_s( wcstring, szWcstring-1, wcstringbuf2);
    if ( modkey == VK_SHIFT ){
        wcscpy_s( wcstring, sizeof(L"shift"), L"shift" );
    }
    if ( modkey == VK_CONTROL ){
        wcscpy_s( wcstring, sizeof(L"ctrl"), L"ctrl" );
    }
    if ( modkey == VK_MENU ){
        wcscpy_s( wcstring, sizeof(L"alt"), L"alt" );
    }
    wcscat_s( wcstring, szWcstring, L"+");
    wcscat_s( wcstring, szWcstring, wcstringbuf );
    wcscat_s( wcstring, szWcstring, L" to " );
    wcscat_s( wcstring, szWcstring, action );
    wcscat_s( wcstring, szWcstring, state );

    delete []wcstringbuf;
}

BOOL IsElevated(){
    BOOL fRet = FALSE;
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
    bool elevated = IsElevated();
    if ( !elevated ){
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

unsigned long block_traffic( LPVOID lpParam )
{
    HANDLE console;
    unsigned char packet[MAXBUF];
    UINT packet_len;
    WINDIVERT_ADDRESS recv_addr, send_addr;
    PWINDIVERT_IPHDR ip_header;
    PWINDIVERT_IPV6HDR ipv6_header;
    PWINDIVERT_ICMPHDR icmp_header;
    PWINDIVERT_ICMPV6HDR icmpv6_header;
    PWINDIVERT_TCPHDR tcp_header;
    PWINDIVERT_UDPHDR udp_header;
    UINT32 src_addr[4], dst_addr[4];
    char src_str[INET6_ADDRSTRLEN+1], dst_str[INET6_ADDRSTRLEN+1];
    UINT payload_len;
    
    TCPPACKET reset0;
    PTCPPACKET reset = &reset0;
    UINT8 dnr0[sizeof(ICMPPACKET) + 0x0F*sizeof(UINT32) + 8 + 1];
    PICMPPACKET dnr = (PICMPPACKET)dnr0;

    TCPV6PACKET resetv6_0;
    PTCPV6PACKET resetv6 = &resetv6_0;
    UINT8 dnrv6_0[sizeof(ICMPV6PACKET) + sizeof(WINDIVERT_IPV6HDR) +
        sizeof(WINDIVERT_TCPHDR)];
    PICMPV6PACKET dnrv6 = (PICMPV6PACKET)dnrv6_0;


    // Initialize all packets.
    PacketIpTcpInit( reset );
    reset->tcp.Rst = 1;
    reset->tcp.Ack = 1;
    PacketIpIcmpInit( dnr );
    dnr->icmp.Type = 3;         // Destination not reachable.
    dnr->icmp.Code = 3;         // Port not reachable.
    PacketIpv6TcpInit( resetv6 );
    resetv6->tcp.Rst = 1;
    resetv6->tcp.Ack = 1;
    PacketIpv6Icmpv6Init( dnrv6 );
    dnrv6->ipv6.Length = htons( sizeof( WINDIVERT_ICMPV6HDR ) + 4 +
        sizeof( WINDIVERT_IPV6HDR ) + sizeof( WINDIVERT_TCPHDR ) );
    dnrv6->icmpv6.Type = 1;     // Destination not reachable.
    dnrv6->icmpv6.Code = 4;     // Port not reachable.

    // Get console for pretty colors.
    console = GetStdHandle( STD_OUTPUT_HANDLE );


    // Main loop:
    while ( TRUE ){ 
    // Read a matching packet.
        if ( !WinDivertRecv( handle, packet, sizeof( packet ), &packet_len, &recv_addr ) )
        {
            fprintf( stderr, "warning: failed to read packet (if you just switched filters its fine)\n" );
            continue;
        }
       
        // Print info about the matching packet.
        WinDivertHelperParsePacket( packet, packet_len, &ip_header, &ipv6_header,
            NULL, &icmp_header, &icmpv6_header, &tcp_header, &udp_header, NULL,
            &payload_len, NULL, NULL );
        if ( ip_header == NULL && ipv6_header == NULL )
        {
            continue;
        }

        // Dump packet info: 
        SetConsoleTextAttribute( console, FOREGROUND_RED );
        fputs( "BLOCK ", stdout );
        SetConsoleTextAttribute( console,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
        if ( ip_header != NULL )
        {
            WinDivertHelperFormatIPv4Address( ntohl( ip_header->SrcAddr ),
                src_str, sizeof( src_str ) );
            WinDivertHelperFormatIPv4Address( ntohl( ip_header->DstAddr ),
                dst_str, sizeof(dst_str ) );
        }
        if ( ipv6_header != NULL )
        {
            WinDivertHelperNtohIpv6Address( ipv6_header->SrcAddr, src_addr );
            WinDivertHelperNtohIpv6Address( ipv6_header->DstAddr, dst_addr );
            WinDivertHelperFormatIPv6Address( src_addr, src_str, sizeof( src_str ) );
            WinDivertHelperFormatIPv6Address( dst_addr, dst_str, sizeof( dst_str ) );
        }
        //printf("ip.SrcAddr=%s ip.DstAddr=%s ", src_str, dst_str);
        if ( icmp_header != NULL )
        {
            printf( "icmp.Type=%u icmp.Code=%u ",
                icmp_header->Type, icmp_header->Code );
            // Simply drop ICMP
        }
        if ( icmpv6_header != NULL )
        {
            printf( "icmpv6.Type=%u icmpv6.Code=%u ",
                icmpv6_header->Type, icmpv6_header->Code );
            // Simply drop ICMPv6
        }
        if ( tcp_header != NULL )
        {
            printf( "tcp.SrcPort=%u tcp.DstPort=%u tcp.Flags=",
                ntohs( tcp_header->SrcPort ), ntohs( tcp_header->DstPort ) );
            if ( tcp_header->Fin )
            {
                fputs( "[FIN]", stdout );
            }
            if ( tcp_header->Rst )
            {
                fputs( "[RST]", stdout );
            }
            if ( tcp_header->Urg )
            {
                fputs( "[URG]", stdout );
            }
            if ( tcp_header->Syn )
            {
                fputs( "[SYN]", stdout );
            }
            if ( tcp_header->Psh )
            {
                fputs( "[PSH]", stdout );
            }
            if ( tcp_header->Ack )
            {
                fputs( "[ACK]", stdout );
            }
            putchar( ' ' );


            if ( ip_header != NULL && !tcp_header->Rst && !tcp_header->Fin )
            {
                // do nothing we just silently drop the packet
                
                // this code is for sending an RST packet
                //reset->ip.SrcAddr = ip_header->DstAddr;
                //reset->ip.DstAddr = ip_header->SrcAddr;
                //reset->tcp.SrcPort = tcp_header->DstPort;
                //reset->tcp.DstPort = tcp_header->SrcPort;
                //reset->tcp.SeqNum = 
                    //(tcp_header->Ack? tcp_header->AckNum: 0);
                //reset->tcp.AckNum =
                    //(tcp_header->Syn?
                        //htonl(ntohl(tcp_header->SeqNum) + 1):
                        //htonl(ntohl(tcp_header->SeqNum) + payload_len));

                //memcpy(&send_addr, &recv_addr, sizeof(send_addr));
                //send_addr.Outbound = !recv_addr.Outbound;
                //WinDivertHelperCalcChecksums((PVOID)reset, sizeof(TCPPACKET),
                    //&send_addr, 0);
                //if (!WinDivertSend(handle, (PVOID)reset, sizeof(TCPPACKET),
                        //NULL, &send_addr))
                //{
                    //fprintf(stderr, "warning: failed to send TCP reset (%lu)\n",
                        //GetLastError());
                //}
            }

            if ( ipv6_header != NULL && !tcp_header->Rst && !tcp_header->Fin )
            {
                memcpy( resetv6->ipv6.SrcAddr, ipv6_header->DstAddr,
                    sizeof( resetv6->ipv6.SrcAddr ) );
                memcpy( resetv6->ipv6.DstAddr, ipv6_header->SrcAddr,
                    sizeof( resetv6->ipv6.DstAddr ) );
                resetv6->tcp.SrcPort = tcp_header->DstPort;
                resetv6->tcp.DstPort = tcp_header->SrcPort;
                resetv6->tcp.SeqNum = ( tcp_header->Ack? tcp_header->AckNum: 0 );
                resetv6->tcp.AckNum =
                    ( tcp_header->Syn?
                        htonl( ntohl( tcp_header->SeqNum ) + 1 ):
                        htonl( ntohl( tcp_header->SeqNum ) + payload_len ) );

                memcpy( &send_addr, &recv_addr, sizeof( send_addr ) );
                send_addr.Outbound = !recv_addr.Outbound;
                WinDivertHelperCalcChecksums( (PVOID)resetv6,
                    sizeof( TCPV6PACKET ), &send_addr, 0 );
                if ( !WinDivertSend(handle, (PVOID)resetv6, sizeof(TCPV6PACKET),
                        NULL, &send_addr ) )
                {
                    fprintf( stderr, "warning: failed to send TCP (IPV6) "
                        "reset (%lu)\n", GetLastError() );
                }
            }
        }
        if ( udp_header != NULL )
        {
            printf( "udp.SrcPort=%u udp.DstPort=%u ",
                ntohs( udp_header->SrcPort ), ntohs( udp_header->DstPort ) );
        
            if ( ip_header != NULL )
            {
                UINT icmp_length = ip_header->HdrLength*sizeof(UINT32) + 8;
                memcpy( dnr->data, ip_header, icmp_length );
                icmp_length += sizeof(ICMPPACKET);
                dnr->ip.Length = htons( (UINT16)icmp_length );
                dnr->ip.SrcAddr = ip_header->DstAddr;
                dnr->ip.DstAddr = ip_header->SrcAddr;
                
                memcpy( &send_addr, &recv_addr, sizeof( send_addr ) );
                send_addr.Outbound = !recv_addr.Outbound;
                WinDivertHelperCalcChecksums( (PVOID)dnr, icmp_length,
                    &send_addr, 0 );
                if ( !WinDivertSend( handle, (PVOID)dnr, icmp_length, NULL,
                        &send_addr ) )
                {
                    fprintf( stderr, "warning: failed to send ICMP message "
                        "(%lu)\n", GetLastError() );
                }
            }
        
            if ( ipv6_header != NULL )
            {
                UINT icmpv6_length = sizeof( WINDIVERT_IPV6HDR ) +
                    sizeof( WINDIVERT_TCPHDR );
                memcpy( dnrv6->data, ipv6_header, icmpv6_length );
                icmpv6_length += sizeof( ICMPV6PACKET );
                memcpy( dnrv6->ipv6.SrcAddr, ipv6_header->DstAddr,
                    sizeof( dnrv6->ipv6.SrcAddr ) );
                memcpy( dnrv6->ipv6.DstAddr, ipv6_header->SrcAddr,
                    sizeof( dnrv6->ipv6.DstAddr ) );
                
                memcpy( &send_addr, &recv_addr, sizeof( send_addr ) );
                send_addr.Outbound = !recv_addr.Outbound;
                WinDivertHelperCalcChecksums( (PVOID)dnrv6, icmpv6_length,
                    &send_addr, 0 );
                if ( !WinDivertSend( handle, (PVOID)dnrv6, icmpv6_length, NULL, &send_addr ) )
                {
                    fprintf( stderr, "warning: failed to send ICMPv6 message "
                        "(%lu)\n", GetLastError() );
                }
            }
        }
        putchar( '\n' );
    }
}

/*
 * Initialize a PACKET.
 */
static void PacketIpInit( PWINDIVERT_IPHDR packet )
{
    memset( packet, 0, sizeof( WINDIVERT_IPHDR ) );
    packet->Version = 4;
    packet->HdrLength = sizeof( WINDIVERT_IPHDR ) / sizeof( UINT32 );
    packet->Id = ntohs( 0xDEAD );
    packet->TTL = 64;
}

/*
 * Initialize a TCPPACKET.
 */
static void PacketIpTcpInit( PTCPPACKET packet )
{
    memset( packet, 0, sizeof( TCPPACKET ) );
    PacketIpInit( &packet->ip );
    packet->ip.Length = htons( sizeof( TCPPACKET ) );
    packet->ip.Protocol = IPPROTO_TCP;
    packet->tcp.HdrLength = sizeof( WINDIVERT_TCPHDR ) / sizeof( UINT32 );
}

/*
 * Initialize an ICMPPACKET.
 */
static void PacketIpIcmpInit( PICMPPACKET packet )
{
    memset( packet, 0, sizeof( ICMPPACKET ) );
    PacketIpInit( &packet->ip );
    packet->ip.Protocol = IPPROTO_ICMP;
}

/*
 * Initialize a PACKETV6.
 */
static void PacketIpv6Init( PWINDIVERT_IPV6HDR packet )
{
    memset( packet, 0, sizeof( WINDIVERT_IPV6HDR ) );
    packet->Version = 6;
    packet->HopLimit = 64;
}

/*
 * Initialize a TCPV6PACKET.
 */
static void PacketIpv6TcpInit( PTCPV6PACKET packet )
{
    memset( packet, 0, sizeof( TCPV6PACKET ) );
    PacketIpv6Init( &packet->ipv6 );
    packet->ipv6.Length = htons( sizeof( WINDIVERT_TCPHDR ) );
    packet->ipv6.NextHdr = IPPROTO_TCP;
    packet->tcp.HdrLength = sizeof( WINDIVERT_TCPHDR ) / sizeof( UINT32 );
}

/*
 * Initialize an ICMP PACKET.
 */
static void PacketIpv6Icmpv6Init( PICMPV6PACKET packet )
{
    memset( packet, 0, sizeof( ICMPV6PACKET ) );
    PacketIpv6Init( &packet->ipv6 );
    packet->ipv6.NextHdr = IPPROTO_ICMPV6;
}

