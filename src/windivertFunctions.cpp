#include "WinDivertFunctions.h"
#include "Limit.h"
#include "UserInterface.h"
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <thread>
#include <vector>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "WinDivert/WinDivert.lib")


namespace Klim
{
    INT16 priority = 1000;
    const char* err_str;
    HANDLE hWindivert = nullptr;
    HANDLE hThread2 = nullptr;
    bool filterThreadRunning = false;

    WinDivertShit::WinDivertShit(const std::vector<std::atomic<Limit>*>& _limit_ptr_vector, UserInterface* _ui_instance)
        : _limit_ptr_vector(_limit_ptr_vector)
        , _ui_instance(_ui_instance)
        , console(nullptr)
    {
    }


    void WinDivertShit::SetFilterRuleString(std::vector<std::atomic<Limit>*> limit_ptr_vector, char* combined_windivert_rules)
    {
        strcpy_s(combined_windivert_rules, 1000, "(udp.DstPort < 1 and udp.DstPort > 1)"); // set to rule that wont match anything

        for (int i = 0; i < limit_ptr_vector.size(); i++)
        {
            if (limit_ptr_vector[i]->load().state)
            {
                Limit limit = limit_ptr_vector[i]->load();
                limit.block = true;
                limit_ptr_vector[i]->store(limit);
                strcat_s(combined_windivert_rules, 1000, limit_ptr_vector[i]->load().windivert_rule);
            }
            else
            {
                Limit limit = limit_ptr_vector[i]->load();
                limit.block = false;
                limit_ptr_vector[i]->store(limit);
            }
        }
        if (strcmp(combined_windivert_rules, "(udp.DstPort < 1 and udp.DstPort > 1)") == 0)
        {
            reinject = true;
        }
        std::cout << "filter: " << combined_windivert_rules << "\n";
    }


    void WinDivertShit::UpdateFilter(char* combined_windivert_rules_ptr)
    {
        char combined_windivert_rules[1000];
        strcpy_s(combined_windivert_rules, sizeof(combined_windivert_rules), combined_windivert_rules_ptr);
        std::cout << "filter: " << combined_windivert_rules << "\n";
        if (hWindivert != nullptr)
        {
            std::cout << "deleting old filter\n";
            if (!WinDivertClose(hWindivert))
            {
                std::cout << "error! failed to open the WinDivert device: " << GetLastError() << "\n";
            }
        }
        std::cout << "creating new filter\n";
        hWindivert = WinDivertOpen(combined_windivert_rules, WINDIVERT_LAYER_NETWORK, priority, 0);
        if (hWindivert == INVALID_HANDLE_VALUE)
        {
            if (GetLastError() == ERROR_INVALID_PARAMETER && !WinDivertHelperCompileFilter(combined_windivert_rules_ptr, WINDIVERT_LAYER_NETWORK, nullptr, 0, &err_str, nullptr))
            {
                std::cout << "error! invalid filter: " << err_str << "\n";
                exit(EXIT_FAILURE);
            }
            std::cout << "error! failed to open the WinDivert device: " << GetLastError() << "\n";
            exit(EXIT_FAILURE);
        }
        if (!filterThreadRunning)
        {
            std::cout << "starting windivert thread\n";
            std::thread myThread([this] { this->WinDivertFilterThread(); });
            myThread.detach();
            filterThreadRunning = true;
        }
    }


    void WinDivertShit::LogPacket(packet_data* packet)
    {
        // Dump packet info...
        if (!passthrough)
        {
            SetConsoleTextAttribute(console, FOREGROUND_RED);
            std::cout << "BLOCK ";
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }

        if (packet->tcp_header != nullptr)
        {
            std::cout << " tcp.Flags=";
            if (packet->tcp_header->Fin)
                std::cout << "[FIN]";
            if (packet->tcp_header->Rst)
                std::cout << "[RST]";
            if (packet->tcp_header->Urg)
                std::cout << "[URG]";
            if (packet->tcp_header->Syn)
                std::cout << "[SYN]";
            if (packet->tcp_header->Psh)
                std::cout << "[PSH]";
            if (packet->tcp_header->Ack)
                std::cout << "[ACK]";

            if (packet->tcp_header->Ack)
            {
                std::cout << "AckNum=" << ntohl(packet->tcp_header->AckNum);
            }
            std::cout << " SeqNum=" << ntohl(packet->tcp_header->SeqNum);
        }

        if (packet->udp_header != nullptr)
        {
            std::cout << "udp.SrcPort=" << ntohs(packet->udp_header->SrcPort) << " udp.DstPort=" << ntohs(packet->udp_header->DstPort);
        }

        std::cout << " size=" << packet->receive_length;
        std::cout << "\n";
    }


    // this is really jank i need to rework this
    bool WinDivertShit::should_reinject(packet_data* packet)
    {
        if (Is3074DL(packet))
        {
            if (!Limit::GetLimitPtrByType(_limit_ptr_vector, Klim::limit_3074_dl)->load().block)
            {
                return true;
            }
        }

        return false;
    };

    void WinDivertShit::ReinjectAll(std::vector<packet_data>* packet_buffer)
    {
        UINT address_length = sizeof(WINDIVERT_ADDRESS);
        bool printed_start = false;
        if (packet_buffer->size() != 0)
        {
            for (int i = 0; i < packet_buffer->size(); i++)
            {
                packet_data packet_data = (*packet_buffer)[i];
                if (should_reinject(&packet_data) || reinject)
                {
                    if (!printed_start)
                    {
                        std::cout << "reinjecting packets (" << packet_buffer->size() << ")... ";
                        printed_start = true;
                    }

                    if (!WinDivertSendEx(hWindivert, packet_data.packet, sizeof(packet_data.packet), nullptr, 0, &packet_data.receive_address, address_length, nullptr))
                    {
                        if (GetLastError() != 6) // expected error so no need for debug output
                        {
                            std::cout << "warning! failed to re-inject buffer packet: " << GetLastError() << " retrying...\n";
                        }
                    }
                    else
                    {
                        std::cout << packet_data.receive_length << " ";
                        packet_buffer->erase(packet_buffer->begin() + i);
                    }
                    i -= 1; // either index is erased or its an error, either way need to roll back
                }
            }
            reinject = false;
        }
    }

    bool WinDivertShit::Is27kDL(packet_data* packet) { return packet->udp_header != nullptr && ntohs(packet->udp_header->SrcPort) >= 27015 && ntohs(packet->udp_header->SrcPort) <= 27200; };
    bool WinDivertShit::Is27kUL(packet_data* packet) { return packet->udp_header != nullptr && ntohs(packet->udp_header->DstPort) >= 27015 && ntohs(packet->udp_header->DstPort) <= 27200; };
    bool WinDivertShit::Is3074DL(packet_data* packet) { return packet->udp_header != nullptr && ntohs(packet->udp_header->SrcPort) == 3074; };
    bool WinDivertShit::Is3074UL(packet_data* packet) { return packet->udp_header != nullptr && ntohs(packet->udp_header->DstPort) == 3074; };
    bool WinDivertShit::Is30kDL(packet_data* packet) { return packet->udp_header != nullptr && ntohs(packet->udp_header->SrcPort) >= 30000 && ntohs(packet->udp_header->SrcPort) <= 30009; };
    bool WinDivertShit::Is7500DL(packet_data* packet) { return packet->tcp_header != nullptr && ntohs(packet->tcp_header->SrcPort) >= 7500 && ntohs(packet->tcp_header->SrcPort) <= 7509; };

    unsigned long WinDivertShit::WinDivertFilterThread()
    {
        UINT address_length = sizeof(WINDIVERT_ADDRESS);
        std::unique_ptr<std::vector<packet_data>> packet_buffer = std::make_unique<std::vector<packet_data>>();
        packet_data cur_packet {};

        // Get console for pretty colors.
        console = GetStdHandle(STD_OUTPUT_HANDLE);

        // Main loop:
        while (TRUE)
        {
            passthrough = false;

            if (hWindivert == INVALID_HANDLE_VALUE || hWindivert == 0)
            {
                continue;
            }

            ReinjectAll(packet_buffer.get());

            // Read a matching packet.
            if (!WinDivertRecvEx(hWindivert, cur_packet.packet, sizeof(cur_packet.packet), &cur_packet.receive_length, 0, &cur_packet.receive_address, &address_length, nullptr))
            {
                if (GetLastError() != 6 && GetLastError() != 995) // expected error so i just dont output debug for it
                {
                    std::cout << "switching filters.. " << GetLastError() << "\n ";
                }
                continue;
            }

            if (!WinDivertHelperParsePacket(cur_packet.packet, cur_packet.receive_length, nullptr, nullptr, nullptr, nullptr, nullptr, &cur_packet.tcp_header, &cur_packet.udp_header, nullptr, &cur_packet.receive_length, nullptr, nullptr))
            {
                std::cout << "parse failed: " << GetLastError() << "\n ";
            }

            if (!WinDivertHelperCalcChecksums(cur_packet.packet, sizeof(cur_packet.packet), nullptr, 0))
            {
                std::cout << "warning! Failed to recalculate checksums: " << GetLastError() << "\n";
            }

            if (Is7500DL(&cur_packet) && !cur_packet.tcp_header->Fin && !cur_packet.tcp_header->Psh)
            {
                passthrough = true;
            }

            if (passthrough)
            {
                if (!WinDivertSendEx(hWindivert, cur_packet.packet, sizeof(cur_packet.packet), nullptr, 0, &cur_packet.receive_address, address_length, nullptr))
                {
                    std::cout << "warning! failed to re-inject packet: " << GetLastError() << "\n";
                }
            }
            else
            {
                LogPacket(&cur_packet);
                packet_buffer->push_back(cur_packet);
            }
        }
    }
}
