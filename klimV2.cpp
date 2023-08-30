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
#pragma clang diagnostic ignored "-Wwritable-strings"
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <stdbool.h>
#include <iostream>
#include <wchar.h>
// i love my gf tea <3

#include "windivert.h"

//int __cdecl Overlay(LPCTSTR);   // a function from a DLL
int __cdecl Overlay(LPTSTR);   // a function from a DLL

typedef UINT (CALLBACK* LPFNDLLMYPUTS)(LPTSTR);

HINSTANCE hDLL;               // Handle to DLL
LPFNDLLMYPUTS lpfnDllOverlay;    // Function pointer


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


/*
 * Entry.
 */
bool state3074 = FALSE;
bool state27k = FALSE; 
bool state30k = FALSE;
bool state7k = FALSE;
// function declaration so function can be below main
void toggle3074();
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
unsigned long block_traffic(LPVOID lpParam);
char myNetRules[500];
HANDLE hThread = NULL;
HANDLE handle = NULL;
const char *err_str;
INT16 priority = 1000;
wchar_t combined_overlay[500], overlay_line_1[100], overlay_line_2[100], overlay_line_3[100], overlay_line_4[100], overlay_line_5[100];

int __cdecl main(){
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


    // hotkey part
    if (RegisterHotKey(
        NULL,
        1, // hotkey id
        MOD_CONTROL | MOD_NOREPEAT,
        0x47))  //0x47 is 'g' // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    {
        _tprintf(_T("Hotkey 'CTRL+g' registered for 3074\n"));
        updateOverlayLine1(L"ctrl+g 3074");
    }

    if (RegisterHotKey(
        NULL,
        2, // hotkey id
        MOD_CONTROL | MOD_NOREPEAT,
        0x54))  //0x54 is 't' // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    {
        _tprintf(_T("Hotkey 'CTRL+t' registered for 27k\n"));
        updateOverlayLine2(L"ctrl+t 27k");
    }

    if (RegisterHotKey(
        NULL,
        3, // hotkey id
        MOD_CONTROL | MOD_NOREPEAT,
        0x59))  //0x59 is 'y' // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    {
        _tprintf(_T("Hotkey 'CTRL+y' registered for 30k\n"));
        updateOverlayLine3(L"ctrl+y 30k");
    }

    if (RegisterHotKey(
        NULL,
        4, // hotkey id
        MOD_CONTROL | MOD_NOREPEAT,
        0x4C))  //0x4C is 'l' // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    {
        _tprintf(_T("Hotkey 'CTRL+l' registered for 7k\n"));
        updateOverlayLine4(L"ctrl+l 7k");
    }

    if (RegisterHotKey(
        NULL,
        10, // hotkey id
        MOD_CONTROL | MOD_NOREPEAT,
        0x4B))  //0x4b is 'k' // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    {
        _tprintf(_T("Hotkey 'CTRL+k' registered for exitapp\n"));
        updateOverlayLine5(L"ctrl+k to close");
    }
 
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) != 0){
        if (msg.message == WM_HOTKEY){
            if (msg.wParam == 1){
                toggle3074();
            }
            if (msg.wParam == 2){
                toggle27k();
            }
            if (msg.wParam == 3){
                toggle30k();
            }
            if (msg.wParam == 4){
                toggle7k();
            }
            if (msg.wParam == 10){
                goto cleanup;
            }
            combinerules();
            
            if (GetThreadId(hThread) == 0){
                printf("Starting thread\n");
                hThread = CreateThread(NULL, 0, block_traffic, NULL, 0, NULL);
            }
        }
    } 

    cleanup:
    printf("Exiting thread\n");
    CloseHandle(hThread);
    FreeLibrary(hDLL);
    return 0;
}

//LPWSTR combined;
void updateOverlay(){
    wcscpy(combined_overlay, L"");
    wcscat(combined_overlay, overlay_line_1);
    wcscat(combined_overlay, L"\n");
    wcscat(combined_overlay, overlay_line_2);
    wcscat(combined_overlay, L"\n");
    wcscat(combined_overlay, overlay_line_3);
    wcscat(combined_overlay, L"\n");
    wcscat(combined_overlay, overlay_line_4);
    wcscat(combined_overlay, L"\n");
    wcscat(combined_overlay, overlay_line_5);
    wcscat(combined_overlay, L"\n");
    lpfnDllOverlay(combined_overlay);
}

void updateOverlayLine1(wchar_t arg[]){
    wcscpy(overlay_line_1, arg);
    updateOverlay();
}

void updateOverlayLine2(wchar_t arg[]){
    wcscpy(overlay_line_2, arg);
    updateOverlay();
}

void updateOverlayLine3(wchar_t arg[]){
    wcscpy(overlay_line_3, arg);
    updateOverlay();
}

void updateOverlayLine4(wchar_t arg[]){
    wcscpy(overlay_line_4, arg);
    updateOverlay();
}

void updateOverlayLine5(wchar_t arg[]){
    wcscpy(overlay_line_5, arg);
    updateOverlay();
}

void combinerules(){
    strcpy_s(myNetRules, sizeof(myNetRules), "(udp.DstPort < 1 and udp.DstPort > 1)"); // set to rule that wont match anything
    if (state3074){
        strcat(myNetRules, " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)");
    }
    if (state27k){
        strcat(myNetRules, " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)");
    }
    if (state30k){
        strcat(myNetRules, " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)");
    }
    if (state7k){
        strcat(myNetRules, " or (inbound and udp.SrcPort >= 7500 and udp.SrcPort <= 7509) or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)");
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
        updateOverlayLine1(L"3074 on");
    } else {
        state3074 = !state3074;
        printf("state3074 %s\n", state3074 ? "true" : "false");
        updateOverlayLine1(L"3074 off");
    }
}

void toggle27k(){
    if (!state27k){
        state27k = !state27k;
        printf("state27k %s\n", state27k ? "true" : "false");
        updateOverlayLine2(L"27k on");
    } else {
        state27k = !state27k;
        printf("state27k %s\n", state27k ? "true" : "false");
        updateOverlayLine2(L"27k off");
    }
}

void toggle30k(){
    if (!state30k){
        state30k = !state30k;
        printf("state30k %s\n", state30k ? "true" : "false");
        updateOverlayLine3(L"30k on");
    } else {
        state30k = !state30k;
        printf("state30k %s\n", state30k ? "true" : "false");
        updateOverlayLine3(L"30k off");
    }
}

void toggle7k(){
    if (!state7k){
        state7k = !state7k;
        printf("state7k %s\n", state7k ? "true" : "false");
        updateOverlayLine4(L"7k on");
    } else {
        state7k = !state7k;
        printf("state7k %s\n", state7k ? "true" : "false");
        updateOverlayLine4(L"7k off");
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

