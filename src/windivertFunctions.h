#include "../WinDivert/windivert.h"
#include <iostream>

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
void __cdecl PacketIpInit( PWINDIVERT_IPHDR packet );
void __cdecl PacketIpTcpInit( PTCPPACKET packet );
void __cdecl PacketIpIcmpInit( PICMPPACKET packet );
void __cdecl PacketIpv6Init( PWINDIVERT_IPV6HDR packet );
void __cdecl PacketIpv6TcpInit( PTCPV6PACKET packet );
void __cdecl PacketIpv6Icmpv6Init( PICMPV6PACKET packet );

// actual functions
unsigned long windivert_filter_thread( LPVOID lpParam );
void updateFilter( char* myNetRules );