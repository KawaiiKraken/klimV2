#include "helperFunctions.h"

const wchar_t* GetFileName( const wchar_t *path )
{
    const wchar_t *filename = wcsrchr( path, '\\' );
    if ( filename == NULL )
        filename = path;
    else
        filename++;
    return filename;
}

bool isD2Active()
{
    TCHAR buffer[MAX_PATH] = {0};
    DWORD dwProcId = 0; 
    GetWindowThreadProcessId( GetForegroundWindow(), &dwProcId );
    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, dwProcId );
    GetModuleFileNameEx( hProc, NULL, buffer, MAX_PATH );
    printf( "buffer: %ls\n", buffer );
    const wchar_t* bufferFilename = GetFileName( buffer );
    printf( "filename: %ls\n", bufferFilename );
    CloseHandle( hProc );
    if ( wcscmp( bufferFilename, L"destiny2.exe" ) == 0 ){
        return TRUE;
    } else {
        return FALSE;
    }
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


/*
 * Initialize a PACKET.
 */
void __cdecl PacketIpInit( PWINDIVERT_IPHDR packet )
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
void __cdecl PacketIpTcpInit( PTCPPACKET packet )
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
void __cdecl PacketIpIcmpInit( PICMPPACKET packet )
{
    memset( packet, 0, sizeof( ICMPPACKET ) );
    PacketIpInit( &packet->ip );
    packet->ip.Protocol = IPPROTO_ICMP;
}

/*
 * Initialize a PACKETV6.
 */
void __cdecl PacketIpv6Init( PWINDIVERT_IPV6HDR packet )
{
    memset( packet, 0, sizeof( WINDIVERT_IPV6HDR ) );
    packet->Version = 6;
    packet->HopLimit = 64;
}

/*
 * Initialize a TCPV6PACKET.
 */
void __cdecl PacketIpv6TcpInit( PTCPV6PACKET packet )
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
void __cdecl PacketIpv6Icmpv6Init( PICMPV6PACKET packet )
{
    memset( packet, 0, sizeof( ICMPV6PACKET ) );
    PacketIpv6Init( &packet->ipv6 );
    packet->ipv6.NextHdr = IPPROTO_ICMPV6;
}

unsigned long block_traffic( HANDLE handle )
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
