#pragma once
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <psapi.h>
#include "../WinDivert/windivert.h"

#define ntohs(x)            WinDivertHelperNtohs(x)
#define ntohl(x)            WinDivertHelperNtohl(x)
#define htons(x)            WinDivertHelperHtons(x)
#define htonl(x)            WinDivertHelperHtonl(x)

#define MAXBUF              0xFFFF
#define INET6_ADDRSTRLEN    45
#define IPPROTO_ICMPV6      58


bool isD2Active();
const wchar_t* GetFileName( const wchar_t *path );
void triggerHotkeyString( wchar_t* wcstring, int szWcstring, char hotkey, char modkey, wchar_t* action, wchar_t* state );
BOOL IsElevated();
unsigned long block_traffic( LPVOID lpParam );
void updateFilter( char* myNetRules );
BOOL FileExists( LPCTSTR szPath );
void writeIniContents( wchar_t* filepath );
void toggle3074( struct limit* lim3074, COLORREF colorOn, COLORREF colorOff );
struct limit {
    char hotkey;
    char modkey;
    bool state = FALSE;
    bool hotkey_keydown = FALSE;
    DWORD modkey_state = 0;
};

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
void __cdecl PacketIpInit( PWINDIVERT_IPHDR packet );
void __cdecl PacketIpTcpInit( PTCPPACKET packet );
void __cdecl PacketIpIcmpInit( PICMPPACKET packet );
void __cdecl PacketIpv6Init( PWINDIVERT_IPV6HDR packet );
void __cdecl PacketIpv6TcpInit( PTCPV6PACKET packet );
void __cdecl PacketIpv6Icmpv6Init( PICMPV6PACKET packet );
