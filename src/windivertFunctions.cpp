// most of this code is taken directly from the windivert netfilter.exe example
#include "windivertFunctions.h"
#include <chrono>
#include <vector>
#include <thread>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <winsock.h>
#include "Limit.h"

INT16 priority = 1000;
const char *err_str;
HANDLE hWindivert = NULL;
HANDLE hThread2 = NULL;

void SetFilterRuleString( std::vector<limit*> limit_ptr_vector, char* combined_windivert_rules[1000]) {
    strcpy_s( *combined_windivert_rules, sizeof( *combined_windivert_rules ), "(udp.DstPort < 1 and udp.DstPort > 1)" ); // set to rule that wont match anything

    for ( int i = 0; i < limit_ptr_vector.size(); i++ ){
        if ( strcmp( limit_ptr_vector[i]->windivert_rule, "" ) != 0 ){
            if ( limit_ptr_vector[i]->state ){
                strcat_s( *combined_windivert_rules, sizeof( *combined_windivert_rules ), limit_ptr_vector[i]->windivert_rule );
            }
        }
    }
    printf( "filter: %s\n", *combined_windivert_rules );
}


void UpdateFilter( char* ptrCombinedWindivertRules ){
    char combinedWindivertRules[1000];
    strcpy_s( combinedWindivertRules, sizeof( combinedWindivertRules ), ptrCombinedWindivertRules);
    printf( "filter: %s\n", combinedWindivertRules );
    if ( hWindivert != NULL ){
        printf( "deleting old filter\n" );
        if ( !WinDivertClose( hWindivert ) ){
            fprintf( stderr, "error: failed to open the WinDivert device (%lu)\n", GetLastError() );
        }
    }
    printf( "creating new filter\n" );
    hWindivert = WinDivertOpen( combinedWindivertRules, WINDIVERT_LAYER_NETWORK, priority, 0 );
    if ( hWindivert == INVALID_HANDLE_VALUE ){
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
    WINDIVERT_ADDRESS send_addr;
    PWINDIVERT_TCPHDR tcp_header;
    PWINDIVERT_UDPHDR udp_header;
    UINT payload_len;
    
    // Get console for pretty colors.
    console = GetStdHandle( STD_OUTPUT_HANDLE );

    // Main loop:
    while ( TRUE ){ 
        // Read a matching packet.
        if (!WinDivertRecvEx(hWindivert, packet, sizeof(packet), 0, 0, 0, 0, 0)) {
            continue;
        }
       
        WinDivertHelperParsePacket( packet, packet_len, 0, 0, 0, 0, 0, &tcp_header, &udp_header, 0, &payload_len, 0, 0 );

        // Dump packet info..
        SetConsoleTextAttribute( console, FOREGROUND_RED );
        fputs( "BLOCK ", stdout );
        SetConsoleTextAttribute( console,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );

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
            printf( " size=%d", payload_len );
        }
        if (udp_header != NULL)
        {
            printf("udp.SrcPort=%u udp.DstPort=%u ",
                ntohs(udp_header->SrcPort), ntohs(udp_header->DstPort));
        }
        putchar( '\n' );
    }
}



