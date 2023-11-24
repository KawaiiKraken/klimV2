// most of this code is taken directly from the windivert netfilter.exe example
#include "windivertFunctions.h"

INT16 priority = 1000;
const char *err_str;
HANDLE handle2 = NULL;
HANDLE hThread2 = NULL;

/*
 * Initialize a PACKET.
 */
void __cdecl PacketIpInit( PWINDIVERT_IPHDR packet ){
    memset( packet, 0, sizeof( WINDIVERT_IPHDR ) );
    packet->Version = 4;
    packet->HdrLength = sizeof( WINDIVERT_IPHDR ) / sizeof( UINT32 );
    packet->Id = ntohs( 0xDEAD );
    packet->TTL = 64;
}

/*
 * Initialize a TCPPACKET.
 */
void __cdecl PacketIpTcpInit( PTCPPACKET packet ){
    memset( packet, 0, sizeof( TCPPACKET ) );
    PacketIpInit( &packet->ip );
    packet->ip.Length = htons( sizeof( TCPPACKET ) );
    packet->ip.Protocol = IPPROTO_TCP;
    packet->tcp.HdrLength = sizeof( WINDIVERT_TCPHDR ) / sizeof( UINT32 );
}

/*
 * Initialize an ICMPPACKET.
 */
void __cdecl PacketIpIcmpInit( PICMPPACKET packet )
{
    memset( packet, 0, sizeof( ICMPPACKET ) );
    PacketIpInit( &packet->ip );
    packet->ip.Protocol = IPPROTO_ICMP;
}

/*
 * Initialize a PACKETV6.
 */
void __cdecl PacketIpv6Init( PWINDIVERT_IPV6HDR packet ){
    memset( packet, 0, sizeof( WINDIVERT_IPV6HDR ) );
    packet->Version = 6;
    packet->HopLimit = 64;
}

/*
 * Initialize a TCPV6PACKET.
 */
void __cdecl PacketIpv6TcpInit( PTCPV6PACKET packet ){
    memset( packet, 0, sizeof( TCPV6PACKET ) );
    PacketIpv6Init( &packet->ipv6 );
    packet->ipv6.Length = htons( sizeof( WINDIVERT_TCPHDR ) );
    packet->ipv6.NextHdr = IPPROTO_TCP;
    packet->tcp.HdrLength = sizeof( WINDIVERT_TCPHDR ) / sizeof( UINT32 );
}

/*
 * Initialize an ICMP PACKET.
 */
void __cdecl PacketIpv6Icmpv6Init( PICMPV6PACKET packet ){
    memset( packet, 0, sizeof( ICMPV6PACKET ) );
    PacketIpv6Init( &packet->ipv6 );
    packet->ipv6.NextHdr = IPPROTO_ICMPV6;
}



void UpdateFilter( char* ptrCombinedWindivertRules ){
    char combinedWindivertRules[1000];
    strcpy_s( combinedWindivertRules, sizeof( combinedWindivertRules ), ptrCombinedWindivertRules);
    printf( "filter: %s\n", combinedWindivertRules );
    if ( handle2 != NULL ){
        printf( "deleting old filter\n" );
        if ( !WinDivertClose( handle2 ) ){
            fprintf( stderr, "error: failed to open the WinDivert device (%lu)\n", GetLastError() );
        }
    }
    printf( "creating new filter\n" );
    handle2 = WinDivertOpen( combinedWindivertRules, WINDIVERT_LAYER_NETWORK, priority, 0 );
    if ( handle2 == INVALID_HANDLE_VALUE ){
        if ( GetLastError() == ERROR_INVALID_PARAMETER && !WinDivertHelperCompileFilter( ptrCombinedWindivertRules, WINDIVERT_LAYER_NETWORK, NULL, 0, &err_str, NULL ) ){
            fprintf( stderr, "error: invalid filter \"%s\"\n", err_str );
            exit( EXIT_FAILURE );
        }
        fprintf( stderr, "error: failed to open the WinDivert device (%lu)\n", GetLastError() );
        exit( EXIT_FAILURE );
    }
    if (hThread2 == NULL){
        printf("starting hotkey thread\n");
        hThread2 = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)WindivertFilterThread, NULL, 0, NULL );
    }
}



unsigned long WindivertFilterThread( LPVOID lpParam ){
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
    int i = 0;
    while ( TRUE ){ 
        // Read a matching packet.
        if ( !WinDivertRecv( handle2, packet, sizeof( packet ), &packet_len, &recv_addr ) )
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
                //if (!WinDivertSend(handle2, (PVOID)reset, sizeof(TCPPACKET),
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
                send_addr.Outbound = ~recv_addr.Outbound;
                WinDivertHelperCalcChecksums( (PVOID)resetv6,
                    sizeof( TCPV6PACKET ), &send_addr, 0 );
                if ( !WinDivertSend(handle2, (PVOID)resetv6, sizeof(TCPV6PACKET), NULL, &send_addr ) )
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

            // buffer 
            //if (udp_header->SrcPort == ) {
//
 //           }
        
            if ( ip_header != NULL )
            {
                UINT icmp_length = ip_header->HdrLength*sizeof(UINT32) + 8;
                memcpy( dnr->data, ip_header, icmp_length );
                icmp_length += sizeof(ICMPPACKET);
                dnr->ip.Length = htons( (UINT16)icmp_length );
                dnr->ip.SrcAddr = ip_header->DstAddr;
                dnr->ip.DstAddr = ip_header->SrcAddr;
                
                memcpy( &send_addr, &recv_addr, sizeof( send_addr ) );
                send_addr.Outbound = ~recv_addr.Outbound;
                WinDivertHelperCalcChecksums( (PVOID)dnr, icmp_length,
                    &send_addr, 0 );
                if ( !WinDivertSend( handle2, (PVOID)dnr, icmp_length, NULL,
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
                send_addr.Outbound = ~recv_addr.Outbound;
                WinDivertHelperCalcChecksums( (PVOID)dnrv6, icmpv6_length,
                    &send_addr, 0 );
                if ( !WinDivertSend( handle2, (PVOID)dnrv6, icmpv6_length, NULL, &send_addr ) )
                {
                    fprintf( stderr, "warning: failed to send ICMPv6 message "
                        "(%lu)\n", GetLastError() );
                }
            }
        }
        putchar( '\n' );
    }
}



