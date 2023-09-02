/*
 * netfilter.c
 * (C) 2019, all rights reserved,
 *
 * This file is part of WinDivert.
 *
 * WinDivert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WinDivert is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * DESCRIPTION:
 * This is a simple traffic filter/firewall using WinDivert.
 *
 * usage: netfilter.exe windivert-filter [priority]
 *
 * Any traffic that matches the windivert-filter will be blocked using one of
 * the following methods:
 * - TCP: send a TCP RST to the packet's source.
 * - UDP: send a ICMP(v6) "destination unreachable" to the packet's source.
 * - ICMP/ICMPv6: Drop the packet.
 *
 * This program is similar to Linux's iptables with the "-j REJECT" target.
 */
//#pragma clang diagnostic ignored "-Wwritable-strings"
#define _WIN32_WINNT 0x0400
#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "advapi32.lib" )
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
//#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <stdbool.h>
#include <iostream>
#include <wchar.h>
#include <shellapi.h>

#include "windivert.h"

int __cdecl Overlay(LPTSTR);   // a function from a DLL

typedef UINT (CALLBACK* LPFNDLLMYPUTS)(LPTSTR);

using namespace std;

HINSTANCE hDLL;               // Handle to DLL
LPFNDLLMYPUTS lpfnDllOverlay;    // Function pointer
HHOOK hKeyboardHook;
HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;
HANDLE gDoneEvent;
HANDLE hThread = NULL;
HANDLE handle = NULL;
bool can_trigger_3074 = TRUE;
bool can_trigger_3074_UL = TRUE;
bool can_trigger_7k = TRUE;
bool can_trigger_27k = TRUE;
bool can_trigger_30k = TRUE;
wchar_t pathToIni[MAX_PATH];
wchar_t szFilePathSelf[MAX_PATH];
char hotkey_exitapp, hotkey_3074, hotkey_3074_UL, hotkey_27k, hotkey_30k, hotkey_7k;
bool state3074 = FALSE;
bool state3074_UL = FALSE;
bool state27k = FALSE; 
bool state30k = FALSE;
bool state7k = FALSE;
// function declaration so function can be below main
void toggle3074();
void toggle3074_UL();
void toggle27k(); 
void toggle30k(); 
void toggle7k(); 
void combinerules();
void updateOverlay();
void updateOverlayLine1(wchar_t arg[]);
void updateOverlayLine2(wchar_t arg[]);
void updateOverlayLine3(wchar_t arg[]);
void updateOverlayLine4(wchar_t arg[]);
void updateOverlayLine5(wchar_t arg[]);
void updateOverlayLine6(wchar_t arg[]);
unsigned long block_traffic(LPVOID lpParam);
char myNetRules[1000];
const char *err_str;
INT16 priority = 1000;
wchar_t combined_overlay[1000], overlay_line_1[100], overlay_line_2[100], overlay_line_3[100], overlay_line_4[100], overlay_line_5[100], overlay_line_6[100];



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
static void PacketIpInit(PWINDIVERT_IPHDR packet);
static void PacketIpTcpInit(PTCPPACKET packet);
static void PacketIpIcmpInit(PICMPPACKET packet);
static void PacketIpv6Init(PWINDIVERT_IPV6HDR packet);
static void PacketIpv6TcpInit(PTCPV6PACKET packet);
static void PacketIpv6Icmpv6Init(PICMPV6PACKET packet);


VOID CALLBACK TimerRoutine(bool* arg, BOOLEAN TimerOrWaitFired){
    printf("hotkey re-enabled\n");
    *arg = TRUE;
}

int setTimer(bool* arg){
    // Set a timer to call the timer routine in hotkey_timeout ms.
    int hotkey_timeout = 1000;
    if (!CreateTimerQueueTimer( &hTimer, hTimerQueue, 
            (WAITORTIMERCALLBACK)TimerRoutine, arg, hotkey_timeout, 0, 0))
    {
        printf("CreateTimerQueueTimer failed (%lu)\n", GetLastError());
        return 3;
    }
    return 0;
}

void startFilter(){
    printf("Starting thread\n");
    hThread = CreateThread(NULL, 0, block_traffic, NULL, 0, NULL);
}

__declspec(dllexport) LRESULT CALLBACK KeyboardEvent (int nCode, WPARAM wParam, LPARAM lParam)
{
    DWORD SHIFT_key=0;
    DWORD CTRL_key=0;
    DWORD ALT_key=0;

    if  ((nCode == HC_ACTION) &&   ((wParam == WM_SYSKEYDOWN) ||  (wParam == WM_KEYDOWN)))      
    {
        KBDLLHOOKSTRUCT hooked_key =    *((KBDLLHOOKSTRUCT*)lParam);
        DWORD dwMsg = 1;
        dwMsg += hooked_key.scanCode << 16;
        dwMsg += hooked_key.flags << 24;
        wchar_t lpszKeyName[1024] = {0};
        lpszKeyName[0] = L'[';

        int i = GetKeyNameText(dwMsg,   (lpszKeyName+1),0xFF) + 1;
        lpszKeyName[i] = L']';

        int key = hooked_key.vkCode;

        SHIFT_key = GetAsyncKeyState(VK_SHIFT);
        CTRL_key = GetAsyncKeyState(VK_CONTROL);
        ALT_key = GetAsyncKeyState(VK_MENU);

        //if (key >= 'A' && key <= 'Z')   
        if (key != (VK_SHIFT | VK_CONTROL | VK_MENU))   // this might be a bit broken
        {
            SHIFT_key = GetAsyncKeyState(VK_SHIFT); // double because async normally only checks if key was pressed since last time it was called, there is a way to bypass that but im lazy
            CTRL_key = GetAsyncKeyState(VK_CONTROL);
            ALT_key = GetAsyncKeyState(VK_MENU);

            //if  (GetAsyncKeyState(VK_SHIFT)>= 0) key +=32;
            
            // ============= 3074 ================
            if (CTRL_key !=0 && key == hotkey_3074 ) 
            {
                wcout << L"hotkey_3074 detected\n";
                if (can_trigger_3074){ // set time out to prevent multiple triggers
                    can_trigger_3074 = FALSE;
                    setTimer(&can_trigger_3074);
                    wcout << L"doing stuff\n";
                    toggle3074();
                    combinerules();
                    startFilter();
                }
                CTRL_key=0;
            }

            // ============= 3074UL ================
            if (CTRL_key !=0 && key == hotkey_3074_UL ) 
            {
                wcout << L"hotkey_3074 detected\n";
                if (can_trigger_3074_UL){ // set time out to prevent multiple triggers
                    can_trigger_3074_UL = FALSE;
                    setTimer(&can_trigger_3074_UL);
                    wcout << L"doing stuff\n";
                    toggle3074_UL();
                    combinerules();
                    startFilter();
                }
                CTRL_key=0;
            }

            // ============= 27k ================
            if (CTRL_key !=0 && key == hotkey_27k ) 
            {
                wcout << L"hotkey_27k detected\n";
                if (can_trigger_27k){ // set time out to prevent multiple triggers
                    can_trigger_27k = FALSE;
                    setTimer(&can_trigger_27k);
                    wcout << L"doing stuff\n";
                    toggle27k();
                    combinerules();
                    startFilter();
                }
                CTRL_key=0;
            }

            // ============= 30k ================
            if (CTRL_key !=0 && key == hotkey_30k ) 
            {
                wcout << L"hotkey_30k detected\n";
                if (can_trigger_30k){ // set time out to prevent multiple triggers
                    can_trigger_30k = FALSE;
                    setTimer(&can_trigger_30k);
                    wcout << L"doing stuff\n";
                    toggle30k();
                    combinerules();
                    startFilter();
                }
                CTRL_key=0;
            }

            // ============= 7k ================
            if (CTRL_key !=0 && key == hotkey_7k ) 
            {
                wcout << L"hotkey_7k detected\n";
                if (can_trigger_7k){ // set time out to prevent multiple triggers
                    can_trigger_7k = FALSE;
                    setTimer(&can_trigger_7k);
                    wcout << L"doing stuff\n";
                    toggle7k();
                    combinerules();
                    startFilter();
                }
                CTRL_key=0;
            } 


            // ============= exitapp ================
            if (CTRL_key !=0 && key == hotkey_exitapp)
            {
                wcout << "shutting down\n";
                PostQuitMessage(0);
            }
            //printf("key: %d\n", key);


            //printf("key = %c\n", key);

            SHIFT_key = 0; // not sure if this is needed
            CTRL_key = 0;
            ALT_key = 0;

        }

        //printf("lpszKeyName = %ls\n",  lpszKeyName );
    }
    return CallNextHookEx(hKeyboardHook,    nCode,wParam,lParam);
}

void MessageLoop()
{
    MSG message;
    while (GetMessage(&message,NULL,0,0)) 
    {
        TranslateMessage( &message );
        DispatchMessage( &message );
    }
}

DWORD WINAPI my_HotKey(LPVOID lpParm)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!hInstance) hInstance = LoadLibrary((LPCWSTR) lpParm); 
    if (!hInstance) return 1;

    hKeyboardHook = SetWindowsHookEx (  WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent,   hInstance,  NULL    );
    MessageLoop();
    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}

BOOL FileExists(LPCTSTR szPath)
{
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

const wchar_t* GetFileName(const wchar_t *path)
{
    const wchar_t *filename = wcsrchr(path, '\\');
    if (filename == NULL)
        filename = path;
    else
        filename++;
    return filename;
}


void setGlobalPathToIni(){ // this function does a bit too much, should prob split it up
    wchar_t szFilePathSelf[MAX_PATH], szFolderPathSelf[MAX_PATH];
    GetModuleFileName(NULL, szFilePathSelf, MAX_PATH);
    wcsncpy_s(szFolderPathSelf, sizeof(szFolderPathSelf), szFilePathSelf, (wcslen(szFilePathSelf) - wcslen(GetFileName(szFilePathSelf))));
    wchar_t filename[MAX_PATH], filePath[MAX_PATH];
    wcscpy_s(filename, MAX_PATH, L"config.ini");
    wcscpy_s(filePath, MAX_PATH, szFolderPathSelf);
    wcscat_s(filePath, MAX_PATH, filename);
    if (!FileExists(filePath)){
        printf("creating config file\n");
        CreateFileW((LPCTSTR)filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        printf("setting config file to default settings\n");
        WritePrivateProfileString(L"hotkeys", L"exitapp", L"k", filePath);
        WritePrivateProfileString(L"hotkeys", L"hotkey_3074", L"g", filePath);
        WritePrivateProfileString(L"hotkeys", L"hotkey_3074_UL", L"c", filePath);
        WritePrivateProfileString(L"hotkeys", L"hotkey_27k", L"t", filePath);
        WritePrivateProfileString(L"hotkeys", L"hotkey_30k", L"l", filePath);
        WritePrivateProfileString(L"hotkeys", L"hotkey_7k", L"j", filePath);
        
    }
    wcsncpy_s(pathToIni, sizeof(pathToIni), filePath, sizeof(pathToIni));
}

void setGlobalHotkeyVars(){
    wchar_t buffer[50];
    wchar_t* wcSingleChar = nullptr;
    // exitapp
    GetPrivateProfileStringW(L"hotkeys", L"exitapp", NULL, buffer, sizeof(buffer), pathToIni);
    if (GetLastError() == 0x2){
        printf("GetPrivateProfileString failed (%lu)\n", GetLastError());
    } else {
        //printf("buffer contains: %ls\n", buffer);
        wcSingleChar = &buffer[0];
        //printf("wcSingleChar: %ls\n", wcSingleChar);
        hotkey_exitapp = VkKeyScanW(*wcSingleChar);
        printf("set hotkey_exitapp to: %c\n", hotkey_exitapp);
    } 
    
    // TODO make this bs into a function
    // 3074
    GetPrivateProfileStringW(L"hotkeys", L"hotkey_3074", NULL, buffer, sizeof(buffer), pathToIni);
    if (GetLastError() == 0x2){
        printf("GetPrivateProfileString failed (%lu)\n", GetLastError());
    } else {
        //printf("buffer contains: %ls\n", buffer);
        wcSingleChar = &buffer[0];
        //printf("wcSingleChar: %ls\n", wcSingleChar);
        hotkey_3074 = VkKeyScanW(*wcSingleChar);
        printf("set hotkey_3074 to: %c\n", hotkey_3074);
    } 

    // 3074_UL
    GetPrivateProfileStringW(L"hotkeys", L"hotkey_3074_UL", NULL, buffer, sizeof(buffer), pathToIni);
    if (GetLastError() == 0x2){
        printf("GetPrivateProfileString failed (%lu)\n", GetLastError());
    } else {
        //printf("buffer contains: %ls\n", buffer);
        wcSingleChar = &buffer[0];
        //printf("wcSingleChar: %ls\n", wcSingleChar);
        hotkey_3074_UL = VkKeyScanW(*wcSingleChar);
        printf("set hotkey_3074_UL to: %c\n", hotkey_3074_UL);
    } 

    // 27k
    GetPrivateProfileStringW(L"hotkeys", L"hotkey_27k", NULL, buffer, sizeof(buffer), pathToIni);
    if (GetLastError() == 0x2){
        printf("GetPrivateProfileString failed (%lu)\n", GetLastError());
    } else {
        //printf("buffer contains: %ls\n", buffer);
        wcSingleChar = &buffer[0];
        //printf("wcSingleChar: %ls\n", wcSingleChar);
        hotkey_27k = VkKeyScanW(*wcSingleChar);
        printf("set hotkey_27k to: %c\n", hotkey_27k);
    } 

    // 30k 
    GetPrivateProfileStringW(L"hotkeys", L"hotkey_30k", NULL, buffer, sizeof(buffer), pathToIni);
    if (GetLastError() == 0x2){
        printf("GetPrivateProfileString failed (%lu)\n", GetLastError());
    } else {
        //printf("buffer contains: %ls\n", buffer);
        wcSingleChar = &buffer[0];
        //printf("wcSingleChar: %ls\n", wcSingleChar);
        hotkey_30k = VkKeyScanW(*wcSingleChar);
        printf("set hotkey_30k to: %c\n", hotkey_30k);
    } 

    // 7k
    GetPrivateProfileStringW(L"hotkeys", L"hotkey_7k", NULL, buffer, sizeof(buffer), pathToIni);
    if (GetLastError() == 0x2){
        printf("GetPrivateProfileString failed (%lu)\n", GetLastError());
    } else {
        //printf("buffer contains: %ls\n", buffer);
        wcSingleChar = &buffer[0];
        //printf("wcSingleChar: %ls\n", wcSingleChar);
        hotkey_7k = VkKeyScanW(*wcSingleChar);
        printf("set hotkey_7k to: %c\n", hotkey_7k);
    } 


}

void triggerHotkeyString(wchar_t* wcstring, char hotkey, wchar_t* action){ // TODO better name for this
    char charbuf[2];
    charbuf[0] = hotkey;
    charbuf[1] = '\0'; // has to be null terminated for strlen to work 
    
    int szCharbuf = strlen(charbuf) + 1;
    wchar_t* wcstringbuf = new wchar_t[szCharbuf];
    mbstowcs(wcstringbuf, charbuf, szCharbuf);

    wcscpy(wcstring, L"ctrl+");
    
    
    int sizeofWcstring = sizeof(wcstring);
    int wcslenWcstring = wcslen(wcstring);
    //wcscat_s(wcstring, wcslenWcstring, wcstringbuf);
    //wcscat(wcstring, L" to ");
    //wcscat_s(wcstring, wcslenWcstring, action);
    //wcscat(wcstring, action);

    wcscat(wcstring, wcstringbuf);
    wcscat(wcstring, L" to ");
    wcscat(wcstring, action);
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

/*
 * Entry.
 */

int __cdecl main(int argc, char** argv){
    if (argv[1] != NULL){
        if (strcmp(argv[1], "--help") == 0){
            printf("options:\n");
            printf("    --help      prints this message.\n");
            printf("    --debug     prevents console hiding. (currently does nothing).\n");
            return 0;
        }
    }
    // check if exe had admin, request if not
    bool elevated = IsElevated();
    if (!elevated){
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)L"ERROR: not running as admin",
            (LPCWSTR)L"ERROR",
            MB_ICONERROR | MB_DEFBUTTON2
        );
        GetModuleFileName(NULL, szFilePathSelf, MAX_PATH); // get own exe
        return 0;
    }
    
    // check if running with debug
    char* arg1 = (char *)"nothing";
    if (argv[1] != NULL){
        arg1 = argv[1];
    }

    if (!(strcmp(arg1, "--debug") == 0)){
        printf("arg1 != debug\n");
        if (!GetConsoleTitle(NULL, 0) && GetLastError() == ERROR_SUCCESS) {
            // Console
            printf("WARN: starting new process!\n");
            bool creationResult;
            STARTUPINFOA info={sizeof(info)};
            PROCESS_INFORMATION processInfo;
            //if (CreateProcessA(argv[0], NULL, NULL, NULL, FALSE, 0, NULL, NULL, &info, &processInfo))
            //{
                //printf("created new process; exiting\n");
                //goto cleanup;
            //} else {
             //   printf("WARN: failed to create new process!\n");
                //goto cleanup;
            //}
        } 
    }
    
    // load dll function

    hDLL = LoadLibrary(L"krekens_overlay");
    if (hDLL != NULL)
    {
        lpfnDllOverlay = (LPFNDLLMYPUTS)GetProcAddress(hDLL, "Overlay");
        if (!lpfnDllOverlay)
        {
            // handle the error
            FreeLibrary(hDLL);
            _tprintf(_T("handle the error"));
            return -3;
        }
    }
    


    DWORD dwThread;
    // prepare timer
    // Use an event object to track the TimerRoutine execution
    gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == gDoneEvent)
    {
        printf("CreateEvent failed (%lu)\n", GetLastError());
        return 1;
    }

    // Create the timer queue.
    hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        printf("CreateTimerQueue failed (%lu)\n", GetLastError());
        return 2;
    }
    
    // ini file stuff
    setGlobalPathToIni(); 
    printf("pathToIni %ls\n", pathToIni);

    setGlobalHotkeyVars();
    

    // TODO make this into a function
    wchar_t* wcstring = new wchar_t[200];
    triggerHotkeyString(wcstring, hotkey_3074, (wchar_t *)L"3074");
    updateOverlayLine1(wcstring);

    triggerHotkeyString(wcstring, hotkey_3074_UL, (wchar_t *)L"3074UL");
    updateOverlayLine2(wcstring);

    triggerHotkeyString(wcstring, hotkey_27k, (wchar_t *)L"27k");
    updateOverlayLine3(wcstring);

    triggerHotkeyString(wcstring, hotkey_30k, (wchar_t *)L"30k");
    updateOverlayLine4(wcstring);

    triggerHotkeyString(wcstring, hotkey_7k, (wchar_t *)L"7k");
    updateOverlayLine5(wcstring);

    triggerHotkeyString(wcstring, hotkey_exitapp, (wchar_t *)L"close");
    updateOverlayLine6(wcstring);
    
    delete []wcstring;


    printf("starting hotkey thread\n");
    hThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)   my_HotKey, (LPVOID) NULL, NULL, &dwThread);



    if (hThread) return WaitForSingleObject(hThread,INFINITE);
    else return 1;
    cleanup:
    CloseHandle(hThread);
    FreeLibrary(hDLL);
    return 0;
}

//LPWSTR combined;
void updateOverlay(){
    wcscpy_s(combined_overlay, L"");
    int szCombined_overlay = sizeof(combined_overlay);
    wcscat_s(combined_overlay, szCombined_overlay, overlay_line_1);
    wcscat_s(combined_overlay, szCombined_overlay, L"\n");
    wcscat_s(combined_overlay, szCombined_overlay, overlay_line_2);
    wcscat_s(combined_overlay, szCombined_overlay, L"\n");
    wcscat_s(combined_overlay, szCombined_overlay, overlay_line_3);
    wcscat_s(combined_overlay, szCombined_overlay, L"\n");
    wcscat_s(combined_overlay, szCombined_overlay, overlay_line_4);
    wcscat_s(combined_overlay, szCombined_overlay, L"\n");
    wcscat_s(combined_overlay, szCombined_overlay, overlay_line_5);
    wcscat_s(combined_overlay, szCombined_overlay, L"\n");
    wcscat_s(combined_overlay, szCombined_overlay, overlay_line_6);
    wcscat_s(combined_overlay, szCombined_overlay, L"\n");
    lpfnDllOverlay(combined_overlay);
}

void updateOverlayLine1(wchar_t arg[]){
    wcscpy_s(overlay_line_1, arg);
    updateOverlay();
}

void updateOverlayLine2(wchar_t arg[]){
    wcscpy_s(overlay_line_2, arg);
    updateOverlay();
}

void updateOverlayLine3(wchar_t arg[]){
    wcscpy_s(overlay_line_3, arg);
    updateOverlay();
}

void updateOverlayLine4(wchar_t arg[]){
    wcscpy_s(overlay_line_4, arg);
    updateOverlay();
}

void updateOverlayLine5(wchar_t arg[]){
    wcscpy_s(overlay_line_5, arg);
    updateOverlay();
}

void updateOverlayLine6(wchar_t arg[]){
    wcscpy_s(overlay_line_6, arg);
    updateOverlay();
}

void combinerules(){
    strcpy_s(myNetRules, sizeof(myNetRules), "(udp.DstPort < 1 and udp.DstPort > 1)"); // set to rule that wont match anything
    if (state3074){
        strcat_s(myNetRules, sizeof(myNetRules), " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)");
    }
    if (state3074_UL){
        strcat_s(myNetRules, sizeof(myNetRules), "or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)"); // TODO test this rule
    }
    if (state27k){
        strcat_s(myNetRules, sizeof(myNetRules), " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)");
    }
    if (state30k){
        strcat_s(myNetRules, sizeof(myNetRules), " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)");
    }
    if (state7k){
        strcat_s(myNetRules, sizeof(myNetRules), " or (inbound and udp.SrcPort >= 7500 and udp.SrcPort <= 7509) or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)");
    }
    printf("filter: %s\n", myNetRules);
    if (handle != NULL){
        printf("deleting old filter\n");
        if(!WinDivertClose(handle)){
            fprintf(stderr, "error: failed to open the WinDivert device (%lu)\n", GetLastError());
        }
    }


    printf("creating new filter\n");
    handle = WinDivertOpen(myNetRules, WINDIVERT_LAYER_NETWORK, priority, 0);
    if (handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_INVALID_PARAMETER &&
            !WinDivertHelperCompileFilter(myNetRules, WINDIVERT_LAYER_NETWORK,
                NULL, 0, &err_str, NULL))
        {
            fprintf(stderr, "error: invalid filter \"%s\"\n", err_str);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "error: failed to open the WinDivert device (%lu)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }
}

void toggle3074(){
    if (!state3074){
        state3074 = !state3074;
        printf("state3074 %s\n", state3074 ? "true" : "false");
        updateOverlayLine1((wchar_t *)L"3074 on");
    } else {
        state3074 = !state3074;
        printf("state3074 %s\n", state3074 ? "true" : "false");
        updateOverlayLine1((wchar_t *)L"3074 off");
    }
}

void toggle3074_UL(){
    if (!state3074_UL){
        state3074_UL = !state3074_UL;
        printf("state3074_UL %s\n", state3074_UL ? "true" : "false");
        updateOverlayLine2((wchar_t *)L"3074UL on");
    } else {
        state3074_UL = !state3074_UL;
        printf("state3074_UL %s\n", state3074_UL ? "true" : "false");
        updateOverlayLine2((wchar_t *)L"3074UL off");
    }
}

void toggle27k(){
    if (!state27k){
        state27k = !state27k;
        printf("state27k %s\n", state27k ? "true" : "false");
        updateOverlayLine3((wchar_t *)L"27k on");
    } else {
        state27k = !state27k;
        printf("state27k %s\n", state27k ? "true" : "false");
        updateOverlayLine3((wchar_t *)L"27k off");
    }
}

void toggle30k(){
    if (!state30k){
        state30k = !state30k;
        printf("state30k %s\n", state30k ? "true" : "false");
        updateOverlayLine4((wchar_t *)L"30k on");
    } else {
        state30k = !state30k;
        printf("state30k %s\n", state30k ? "true" : "false");
        updateOverlayLine4((wchar_t *)L"30k off");
    }
}

void toggle7k(){
    if (!state7k){
        state7k = !state7k;
        printf("state7k %s\n", state7k ? "true" : "false");
        updateOverlayLine5((wchar_t *)L"7k on");
    } else {
        state7k = !state7k;
        printf("state7k %s\n", state7k ? "true" : "false");
        updateOverlayLine5((wchar_t *)L"7k off");
    }
}


unsigned long block_traffic(LPVOID lpParam)
{
    printf("i am a thread!\n");
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
    PacketIpTcpInit(reset);
    reset->tcp.Rst = 1;
    reset->tcp.Ack = 1;
    PacketIpIcmpInit(dnr);
    dnr->icmp.Type = 3;         // Destination not reachable.
    dnr->icmp.Code = 3;         // Port not reachable.
    PacketIpv6TcpInit(resetv6);
    resetv6->tcp.Rst = 1;
    resetv6->tcp.Ack = 1;
    PacketIpv6Icmpv6Init(dnrv6);
    dnrv6->ipv6.Length = htons(sizeof(WINDIVERT_ICMPV6HDR) + 4 +
        sizeof(WINDIVERT_IPV6HDR) + sizeof(WINDIVERT_TCPHDR));
    dnrv6->icmpv6.Type = 1;     // Destination not reachable.
    dnrv6->icmpv6.Code = 4;     // Port not reachable.

    // Get console for pretty colors.
    console = GetStdHandle(STD_OUTPUT_HANDLE);


    // Main loop:
    while (TRUE){ 
    // Read a matching packet.
        if (!WinDivertRecv(handle, packet, sizeof(packet), &packet_len,
                &recv_addr))
        {
            fprintf(stderr, "warning: failed to read packet (if you just switched filters its fine)\n");
            continue;
        }
       
        // Print info about the matching packet.
        WinDivertHelperParsePacket(packet, packet_len, &ip_header, &ipv6_header,
            NULL, &icmp_header, &icmpv6_header, &tcp_header, &udp_header, NULL,
            &payload_len, NULL, NULL);
        if (ip_header == NULL && ipv6_header == NULL)
        {
            continue;
        }

        // Dump packet info: 
        SetConsoleTextAttribute(console, FOREGROUND_RED);
        fputs("BLOCK ", stdout);
        SetConsoleTextAttribute(console,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        if (ip_header != NULL)
        {
            WinDivertHelperFormatIPv4Address(ntohl(ip_header->SrcAddr),
                src_str, sizeof(src_str));
            WinDivertHelperFormatIPv4Address(ntohl(ip_header->DstAddr),
                dst_str, sizeof(dst_str));
        }
        if (ipv6_header != NULL)
        {
            WinDivertHelperNtohIpv6Address(ipv6_header->SrcAddr, src_addr);
            WinDivertHelperNtohIpv6Address(ipv6_header->DstAddr, dst_addr);
            WinDivertHelperFormatIPv6Address(src_addr, src_str,
                sizeof(src_str));
            WinDivertHelperFormatIPv6Address(dst_addr, dst_str,
                sizeof(dst_str));
        }
        //printf("ip.SrcAddr=%s ip.DstAddr=%s ", src_str, dst_str);
        if (icmp_header != NULL)
        {
            printf("icmp.Type=%u icmp.Code=%u ",
                icmp_header->Type, icmp_header->Code);
            // Simply drop ICMP
        }
        if (icmpv6_header != NULL)
        {
            printf("icmpv6.Type=%u icmpv6.Code=%u ",
                icmpv6_header->Type, icmpv6_header->Code);
            // Simply drop ICMPv6
        }
        if (tcp_header != NULL)
        {
            printf("tcp.SrcPort=%u tcp.DstPort=%u tcp.Flags=",
                ntohs(tcp_header->SrcPort), ntohs(tcp_header->DstPort));
            if (tcp_header->Fin)
            {
                fputs("[FIN]", stdout);
            }
            if (tcp_header->Rst)
            {
                fputs("[RST]", stdout);
            }
            if (tcp_header->Urg)
            {
                fputs("[URG]", stdout);
            }
            if (tcp_header->Syn)
            {
                fputs("[SYN]", stdout);
            }
            if (tcp_header->Psh)
            {
                fputs("[PSH]", stdout);
            }
            if (tcp_header->Ack)
            {
                fputs("[ACK]", stdout);
            }
            putchar(' ');


            if (ip_header != NULL && !tcp_header->Rst && !tcp_header->Fin)
            {
                reset->ip.SrcAddr = ip_header->DstAddr;
                reset->ip.DstAddr = ip_header->SrcAddr;
                reset->tcp.SrcPort = tcp_header->DstPort;
                reset->tcp.DstPort = tcp_header->SrcPort;
                reset->tcp.SeqNum = 
                    (tcp_header->Ack? tcp_header->AckNum: 0);
                reset->tcp.AckNum =
                    (tcp_header->Syn?
                        htonl(ntohl(tcp_header->SeqNum) + 1):
                        htonl(ntohl(tcp_header->SeqNum) + payload_len));

                memcpy(&send_addr, &recv_addr, sizeof(send_addr));
                send_addr.Outbound = !recv_addr.Outbound;
                WinDivertHelperCalcChecksums((PVOID)reset, sizeof(TCPPACKET),
                    &send_addr, 0);
                if (!WinDivertSend(handle, (PVOID)reset, sizeof(TCPPACKET),
                        NULL, &send_addr))
                {
                    fprintf(stderr, "warning: failed to send TCP reset (%lu)\n",
                        GetLastError());
                }
            }

            if (ipv6_header != NULL && !tcp_header->Rst && !tcp_header->Fin)
            {
                memcpy(resetv6->ipv6.SrcAddr, ipv6_header->DstAddr,
                    sizeof(resetv6->ipv6.SrcAddr));
                memcpy(resetv6->ipv6.DstAddr, ipv6_header->SrcAddr,
                    sizeof(resetv6->ipv6.DstAddr));
                resetv6->tcp.SrcPort = tcp_header->DstPort;
                resetv6->tcp.DstPort = tcp_header->SrcPort;
                resetv6->tcp.SeqNum =
                    (tcp_header->Ack? tcp_header->AckNum: 0);
                resetv6->tcp.AckNum =
                    (tcp_header->Syn?
                        htonl(ntohl(tcp_header->SeqNum) + 1):
                        htonl(ntohl(tcp_header->SeqNum) + payload_len));

                memcpy(&send_addr, &recv_addr, sizeof(send_addr));
                send_addr.Outbound = !recv_addr.Outbound;
                WinDivertHelperCalcChecksums((PVOID)resetv6,
                    sizeof(TCPV6PACKET), &send_addr, 0);
                if (!WinDivertSend(handle, (PVOID)resetv6, sizeof(TCPV6PACKET),
                        NULL, &send_addr))
                {
                    fprintf(stderr, "warning: failed to send TCP (IPV6) "
                        "reset (%lu)\n", GetLastError());
                }
            }
        }
        if (udp_header != NULL)
        {
            printf("udp.SrcPort=%u udp.DstPort=%u ",
                ntohs(udp_header->SrcPort), ntohs(udp_header->DstPort));
        
            if (ip_header != NULL)
            {
                UINT icmp_length = ip_header->HdrLength*sizeof(UINT32) + 8;
                memcpy(dnr->data, ip_header, icmp_length);
                icmp_length += sizeof(ICMPPACKET);
                dnr->ip.Length = htons((UINT16)icmp_length);
                dnr->ip.SrcAddr = ip_header->DstAddr;
                dnr->ip.DstAddr = ip_header->SrcAddr;
                
                memcpy(&send_addr, &recv_addr, sizeof(send_addr));
                send_addr.Outbound = !recv_addr.Outbound;
                WinDivertHelperCalcChecksums((PVOID)dnr, icmp_length,
                    &send_addr, 0);
                if (!WinDivertSend(handle, (PVOID)dnr, icmp_length, NULL,
                        &send_addr))
                {
                    fprintf(stderr, "warning: failed to send ICMP message "
                        "(%lu)\n", GetLastError());
                }
            }
        
            if (ipv6_header != NULL)
            {
                UINT icmpv6_length = sizeof(WINDIVERT_IPV6HDR) +
                    sizeof(WINDIVERT_TCPHDR);
                memcpy(dnrv6->data, ipv6_header, icmpv6_length);
                icmpv6_length += sizeof(ICMPV6PACKET);
                memcpy(dnrv6->ipv6.SrcAddr, ipv6_header->DstAddr,
                    sizeof(dnrv6->ipv6.SrcAddr));
                memcpy(dnrv6->ipv6.DstAddr, ipv6_header->SrcAddr,
                    sizeof(dnrv6->ipv6.DstAddr));
                
                memcpy(&send_addr, &recv_addr, sizeof(send_addr));
                send_addr.Outbound = !recv_addr.Outbound;
                WinDivertHelperCalcChecksums((PVOID)dnrv6, icmpv6_length,
                    &send_addr, 0);
                if (!WinDivertSend(handle, (PVOID)dnrv6, icmpv6_length,
                        NULL, &send_addr))
                {
                    fprintf(stderr, "warning: failed to send ICMPv6 message "
                        "(%lu)\n", GetLastError());
                }
            }
        }
        putchar('\n');
    }
    printf("THREAD SELF TERMINATED\n");
    return 0;
}

/*
 * Initialize a PACKET.
 */
static void PacketIpInit(PWINDIVERT_IPHDR packet)
{
    memset(packet, 0, sizeof(WINDIVERT_IPHDR));
    packet->Version = 4;
    packet->HdrLength = sizeof(WINDIVERT_IPHDR) / sizeof(UINT32);
    packet->Id = ntohs(0xDEAD);
    packet->TTL = 64;
}

/*
 * Initialize a TCPPACKET.
 */
static void PacketIpTcpInit(PTCPPACKET packet)
{
    memset(packet, 0, sizeof(TCPPACKET));
    PacketIpInit(&packet->ip);
    packet->ip.Length = htons(sizeof(TCPPACKET));
    packet->ip.Protocol = IPPROTO_TCP;
    packet->tcp.HdrLength = sizeof(WINDIVERT_TCPHDR) / sizeof(UINT32);
}

/*
 * Initialize an ICMPPACKET.
 */
static void PacketIpIcmpInit(PICMPPACKET packet)
{
    memset(packet, 0, sizeof(ICMPPACKET));
    PacketIpInit(&packet->ip);
    packet->ip.Protocol = IPPROTO_ICMP;
}

/*
 * Initialize a PACKETV6.
 */
static void PacketIpv6Init(PWINDIVERT_IPV6HDR packet)
{
    memset(packet, 0, sizeof(WINDIVERT_IPV6HDR));
    packet->Version = 6;
    packet->HopLimit = 64;
}

/*
 * Initialize a TCPV6PACKET.
 */
static void PacketIpv6TcpInit(PTCPV6PACKET packet)
{
    memset(packet, 0, sizeof(TCPV6PACKET));
    PacketIpv6Init(&packet->ipv6);
    packet->ipv6.Length = htons(sizeof(WINDIVERT_TCPHDR));
    packet->ipv6.NextHdr = IPPROTO_TCP;
    packet->tcp.HdrLength = sizeof(WINDIVERT_TCPHDR) / sizeof(UINT32);
}

/*
 * Initialize an ICMP PACKET.
 */
static void PacketIpv6Icmpv6Init(PICMPV6PACKET packet)
{
    memset(packet, 0, sizeof(ICMPV6PACKET));
    PacketIpv6Init(&packet->ipv6);
    packet->ipv6.NextHdr = IPPROTO_ICMPV6;
}

