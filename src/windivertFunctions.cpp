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
                strcat_s(combined_windivert_rules, 1000, limit_ptr_vector[i]->load().windivert_rule);
            }
        }
        std::cout << "filter: " << combined_windivert_rules << "\n";

        if (strcmp(combined_windivert_rules, "(udp.DstPort < 1 and udp.DstPort > 1)") == 0)
        {
            // TODO make it work for all limits separately
            reinject = true;
        }
        else
        {
            reinject = false;
        }
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

    void WinDivertShit::ReinjectAll(std::vector<packet_data>* packet_buffer)
    {
        UINT address_length = sizeof(WINDIVERT_ADDRESS);
        if (packet_buffer->size() != 0)
        {
            std::cout << "reinjecting packets (" << packet_buffer->size() << ")... ";
            for (int i = 0; i < packet_buffer->size(); i++)
            {
                packet_data packet_data = (*packet_buffer)[i];
                if (!WinDivertSendEx(hWindivert, packet_data.packet, sizeof(packet_data.packet), nullptr, 0, &packet_data.receive_address, address_length, nullptr))
                {
                    if (GetLastError() != 6) // expected error so i just dont output debug for it
                    {
                        std::cout << "warning! failed to re-inject buffer packet: " << GetLastError() << " retrying...\n";
                    }
                    i -= 1;
                }
                else
                {
                    std::cout << packet_data.receive_length << " ";
                }
            }
            std::cout << "\nall blocked packets reinjected\n";
            reinject = false;
            packet_buffer->clear();
        }
    }


    unsigned long WinDivertShit::WinDivertFilterThread()
    {
        UINT address_length = sizeof(WINDIVERT_ADDRESS);
        std::unique_ptr<std::vector<packet_data>> packet_buffer = std::make_unique<std::vector<packet_data>>();

        // Get console for pretty colors.
        console = GetStdHandle(STD_OUTPUT_HANDLE);

        auto Is27kDL = [](packet_data* packet) -> bool { return packet->udp_header != nullptr && ntohs(packet->udp_header->SrcPort) >= 27015 && ntohs(packet->udp_header->SrcPort) <= 27200; };
        auto Is27kUL = [](packet_data* packet) -> bool { return packet->udp_header != nullptr && ntohs(packet->udp_header->DstPort) >= 27015 && ntohs(packet->udp_header->DstPort) <= 27200; };
        auto Is3074DL = [](packet_data* packet) -> bool { return packet->udp_header != nullptr && ntohs(packet->udp_header->SrcPort) == 3074; };
        auto Is3074UL = [](packet_data* packet) -> bool { return packet->udp_header != nullptr && ntohs(packet->udp_header->DstPort) == 3074; };
        auto Is30kDL = [](packet_data* packet) -> bool { return packet->udp_header != nullptr && ntohs(packet->udp_header->SrcPort) >= 30000 && ntohs(packet->udp_header->SrcPort) <= 30009; };
        auto Is7500DL = [](packet_data* packet) -> bool { return packet->tcp_header != nullptr && ntohs(packet->tcp_header->SrcPort) >= 7500 && ntohs(packet->tcp_header->SrcPort) <= 7509; };

        // Main loop:
        while (TRUE)
        {
            if (hWindivert == INVALID_HANDLE_VALUE || hWindivert == 0)
            {
                continue;
            }

            // Read a matching packet.
            packet_data cur_packet {};


            if (reinject)
            {
                ReinjectAll(packet_buffer.get());
            }


            if (!WinDivertRecvEx(hWindivert, cur_packet.packet, sizeof(cur_packet.packet), &cur_packet.receive_length, 0, &cur_packet.receive_address, &address_length, nullptr))
            {
                if (GetLastError() != 6 && GetLastError() != 995) // expected error so i just dont output debug for it
                {
                    std::cout << "switching filters.. " << GetLastError() << "\n ";
                }
                continue;
            }
            else
            {
                cur_packet.recv_time = std::chrono::high_resolution_clock::now();
            }


            if (!WinDivertHelperParsePacket(cur_packet.packet, cur_packet.receive_length, nullptr, nullptr, nullptr, nullptr, nullptr, &cur_packet.tcp_header, &cur_packet.udp_header, nullptr, &cur_packet.receive_length, nullptr, nullptr))
            {
                std::cout << "parse failed: " << GetLastError() << "\n ";
            }

            if (!WinDivertHelperCalcChecksums(cur_packet.packet, sizeof(cur_packet.packet), nullptr, 0))
            {
                std::cout << "warning! Failed to recalculate checksums: " << GetLastError() << "\n";
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
