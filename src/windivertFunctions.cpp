// most of this code is taken directly from the windivert netfilter.exe example
#include "windivertFunctions.h"
#include <chrono>
#include <vector>
#include <thread>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <winsock.h>

INT16 priority = 1000;
const char *err_str;
HANDLE handle2 = NULL;
HANDLE hThread2 = NULL;



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
    UINT packet_len = 1500;
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
    
    // Get console for pretty colors.
    console = GetStdHandle( STD_OUTPUT_HANDLE );

    UINT addr_len = sizeof(WINDIVERT_ADDRESS);
    // Main loop:
    while ( TRUE ){ 
        UINT recv_len;
        // Read a matching packet.
        if ( !WinDivertRecvEx( handle2, packet, sizeof( packet ), &recv_len, 0, &recv_addr, &addr_len, NULL ) )
        {
            fprintf( stderr, "warning: failed to read packet (if you just switched filters its fine)\n" );
            continue;
        }
       
        PVOID payload;
        WinDivertHelperParsePacket( packet, packet_len, &ip_header, &ipv6_header,
            NULL, &icmp_header, &icmpv6_header, &tcp_header, &udp_header, &payload,
            &payload_len, NULL, NULL );
        if ( ip_header == NULL && ipv6_header == NULL )
        {
            continue;
        }

        // Dump packet info..
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
        }
        if ( icmpv6_header != NULL )
        {
            printf( "icmpv6.Type=%u icmpv6.Code=%u ",
                icmpv6_header->Type, icmpv6_header->Code );
        }
        if ( tcp_header != NULL)
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
            if ( tcp_header->Ack ) {
                printf( "AckNum=%d", ntohl( tcp_header->AckNum ) );
            }
		    printf(" SeqNum=%d", ntohl( tcp_header->SeqNum ) );
            printf( " len=%d", packet_len );
        }
        if (udp_header != NULL)
        {
            printf("udp.SrcPort=%u udp.DstPort=%u ",
                ntohs(udp_header->SrcPort), ntohs(udp_header->DstPort));
        }
        putchar( '\n' );
    }
}



