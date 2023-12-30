#include "windivertFunctions.h"
#include "Limit.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>
#include <winsock.h>

INT16 priority = 1000;
const char* err_str;
HANDLE hWindivert = NULL;
HANDLE hThread2   = NULL;

void SetFilterRuleString(std::vector<std::atomic<Limit>*> limit_ptr_vector, char* combined_windivert_rules)
{
    strcpy_s(combined_windivert_rules, 1000, "(udp.DstPort < 1 and udp.DstPort > 1)"); // set to rule that wont match anything

    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        if (limit_ptr_vector[i]->load().state) {
            strcat_s(combined_windivert_rules, 1000, limit_ptr_vector[i]->load().windivert_rule);
        }
    }
    std::cout << "filter: " << combined_windivert_rules << std::endl;
}


void UpdateFilter(char* ptrCombinedWindivertRules)
{
    char combinedWindivertRules[1000];
    strcpy_s(combinedWindivertRules, sizeof(combinedWindivertRules), ptrCombinedWindivertRules);
    std::cout << "filter: " << combinedWindivertRules << std::endl;
    if (hWindivert != NULL) {
        std::cout << "deleting old filter" << std::endl;
        if (!WinDivertClose(hWindivert)) {
            std::cout <<  "error! failed to open the WinDivert device: " <<  GetLastError() << std::endl;
        }
    }
    std::cout << "creating new filter" << std::endl;
    hWindivert = WinDivertOpen(combinedWindivertRules, WINDIVERT_LAYER_NETWORK, priority, 0);
    if (hWindivert == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_INVALID_PARAMETER && !WinDivertHelperCompileFilter(ptrCombinedWindivertRules, WINDIVERT_LAYER_NETWORK, NULL, 0, &err_str, NULL)) {
            std::cout << "error! invalid filter: " << err_str << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "error! failed to open the WinDivert device: " << GetLastError() << std::endl;
        exit(EXIT_FAILURE);
    }
    if (hThread2 == NULL) {
        std::cout << "starting hotkey thread" << std::endl;
        hThread2 = CreateThread(NULL, 0, ( LPTHREAD_START_ROUTINE )WindivertFilterThread, NULL, 0, NULL);
    }
}


unsigned long WindivertFilterThread(LPVOID lpParam)
{
    HANDLE console;
    unsigned char packet[MAXBUF];
    UINT packet_len = 1500;
    UINT recv_len;
    UINT addr_len = sizeof(WINDIVERT_ADDRESS);
    WINDIVERT_ADDRESS recv_addr;
    PWINDIVERT_TCPHDR tcp_header;
    PWINDIVERT_UDPHDR udp_header;
    UINT payload_len;

    // Get console for pretty colors.
    console = GetStdHandle(STD_OUTPUT_HANDLE);

    // Main loop:
    while (TRUE) {
        // Read a matching packet.
        if (!WinDivertRecvEx(hWindivert, packet, sizeof(packet), &recv_len, 0, &recv_addr, &addr_len, 0)) {
            continue;
        }

        WinDivertHelperParsePacket(packet, packet_len, 0, 0, 0, 0, 0, &tcp_header, &udp_header, 0, &payload_len, 0, 0);

        // Dump packet info..
        SetConsoleTextAttribute(console, FOREGROUND_RED);
        std::cout << "BLOCK ";
        SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        if (tcp_header != NULL) {
            std::cout << "tcp.SrcPort=" << ntohs(tcp_header->SrcPort) << " tcp.DstPort=" << ntohs(tcp_header->DstPort) << std::endl;

            std::cout << " tcp.Flags=";
            if (tcp_header->Fin)  std::cout << "[FIN]";
            if (tcp_header->Rst)  std::cout << "[RST]";
            if (tcp_header->Urg)  std::cout << "[URG]";
            if (tcp_header->Syn)  std::cout << "[SYN]";
            if (tcp_header->Psh)  std::cout << "[PSH] ";
            if (tcp_header->Ack)  std::cout << "[ACK]";
            std::cout << ' ';

            if (tcp_header->Ack) {
                std::cout << "AckNum=" << ntohl(tcp_header->AckNum);
            }
            std::cout << " SeqNum=" << ntohl(tcp_header->SeqNum);
            std::cout << " size=" << payload_len;
        }
        if (udp_header != NULL) {
            std::cout << "udp.SrcPort=" << ntohs(udp_header->SrcPort) << " udp.DstPort=" << ntohs(udp_header->DstPort);
        }
        std::cout << std::endl;
        if (tcp_header != NULL && ntohs(tcp_header->SrcPort) == 7500 && !tcp_header->Fin && !tcp_header->Psh) {
            if (!WinDivertSendEx(hWindivert, packet, recv_len, NULL, 0, &recv_addr, addr_len, NULL)) {
                std::cout << std::endl << "warning! failed to reinject packet: " <<  GetLastError();
            } else {
                std::cout << std::endl << "reinjected ack packet";
            }
        }
    }
}
