#include "WinDivertFunctions.h"
#include "Limit.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>
#include <winsock.h>

namespace Klim
{
    INT16 priority = 1000;
    const char* err_str;
    HANDLE hWindivert = nullptr;
    HANDLE hThread2 = nullptr;

    void SetFilterRuleString(std::vector<std::atomic<Limit>*> limit_ptr_vector, char* combined_windivert_rules)
    {
        strcpy_s(combined_windivert_rules, 1000, "(udp.DstPort < 1 and udp.DstPort > 1)"); // set to rule that wont match anything

        for (int i = 0; i < limit_ptr_vector.size(); i++) {
            if (limit_ptr_vector[i]->load().state) {
                strcat_s(combined_windivert_rules, 1000, limit_ptr_vector[i]->load().windivert_rule);
            }
        }
        std::cout << "filter: " << combined_windivert_rules << "\n";
    }


    void UpdateFilter(char* combined_windivert_rules_ptr)
    {
        char combined_windivert_rules[1000];
        strcpy_s(combined_windivert_rules, sizeof(combined_windivert_rules), combined_windivert_rules_ptr);
        std::cout << "filter: " << combined_windivert_rules << "\n";
        if (hWindivert != nullptr) {
            std::cout << "deleting old filter\n";
            if (!WinDivertClose(hWindivert)) {
                std::cout << "error! failed to open the WinDivert device: " << GetLastError() << "\n";
            }
        }
        std::cout << "creating new filter\n";
        hWindivert = WinDivertOpen(combined_windivert_rules, WINDIVERT_LAYER_NETWORK, priority, 0);
        if (hWindivert == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_INVALID_PARAMETER && !WinDivertHelperCompileFilter(combined_windivert_rules_ptr, WINDIVERT_LAYER_NETWORK, nullptr, 0, &err_str, nullptr)) {
                std::cout << "error! invalid filter: " << err_str << "\n";
                exit(EXIT_FAILURE);
            }
            std::cout << "error! failed to open the WinDivert device: " << GetLastError() << "\n";
            exit(EXIT_FAILURE);
        }
        if (hThread2 == nullptr) {
            std::cout << "starting hotkey thread\n";
            hThread2 = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)WinDivertFilterThread, nullptr, 0, nullptr);
        }
    }

    unsigned long WinDivertFilterThread(LPVOID lpParam)
    {
        UINT receive_length;
        UINT address_length = sizeof(WINDIVERT_ADDRESS);
        WINDIVERT_ADDRESS receive_address;
        PWINDIVERT_TCPHDR tcp_header;
        PWINDIVERT_UDPHDR udp_header;
        UINT payload_len;

        // Get console for pretty colors.
        const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

        // Main loop:
        while (TRUE) {
            const UINT packet_len = 1500;
            unsigned char packet[MAXBUF];
            // Read a matching packet.
            if (!WinDivertRecvEx(hWindivert, packet, sizeof(packet), &receive_length, 0, &receive_address, &address_length, nullptr)) {
                continue;
            }

            WinDivertHelperParsePacket(packet, packet_len, nullptr, nullptr, nullptr, nullptr, nullptr, &tcp_header, &udp_header, nullptr, &payload_len, nullptr, nullptr);

            // Dump packet info...
            SetConsoleTextAttribute(console, FOREGROUND_RED);
            std::cout << "BLOCK ";
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

            if (tcp_header != nullptr) {
                std::cout << "tcp.SrcPort=" << ntohs(tcp_header->SrcPort) << " tcp.DstPort=" << ntohs(tcp_header->DstPort) << "\n";

                std::cout << " tcp.Flags=";
                if (tcp_header->Fin)
                    std::cout << "[FIN]";
                if (tcp_header->Rst)
                    std::cout << "[RST]";
                if (tcp_header->Urg)
                    std::cout << "[URG]";
                if (tcp_header->Syn)
                    std::cout << "[SYN]";
                if (tcp_header->Psh)
                    std::cout << "[PSH] ";
                if (tcp_header->Ack)
                    std::cout << "[ACK]";
                std::cout << ' ';

                if (tcp_header->Ack) {
                    std::cout << "AckNum=" << ntohl(tcp_header->AckNum);
                }
                std::cout << " SeqNum=" << ntohl(tcp_header->SeqNum);
                std::cout << " size=" << payload_len;
            }
            if (udp_header != nullptr) {
                std::cout << "udp.SrcPort=" << ntohs(udp_header->SrcPort) << " udp.DstPort=" << ntohs(udp_header->DstPort);
            }
            std::cout << "\n";
            if (tcp_header != nullptr && ntohs(tcp_header->SrcPort) == 7500 && !tcp_header->Fin && !tcp_header->Psh) {
                if (!WinDivertSendEx(hWindivert, packet, receive_length, nullptr, 0, &receive_address, address_length, nullptr)) {
                    std::cout << "\n"
                              << "warning! failed to re-inject packet: " << GetLastError();
                } else {
                    std::cout << "\n"
                              << "re-injected ack packet";
                }
            }
        }
    }
}