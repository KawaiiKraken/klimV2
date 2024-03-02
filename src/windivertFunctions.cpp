#include "WinDivertFunctions.h"
#include "ConfigFile.h"
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

    WinDivertShit::WinDivertShit(const std::vector<std::atomic<Limit>*>& _limit_ptr_vector, UserInterface* _ui_instance, std::shared_ptr<spdlog::logger> logger, Settings* settings)
        : _limit_ptr_vector(_limit_ptr_vector)
        , _ui_instance(_ui_instance)
        , console(nullptr)
        , logger(logger)
        , _settings(settings)
    {
    }


    // sets the string that is used by the windivert capture filter
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
        logger->info("filter: {}", combined_windivert_rules);
    }


    // updates the windivert capture filter
    void WinDivertShit::UpdateFilter(char* combined_windivert_rules_ptr)
    {
        char combined_windivert_rules[1000];
        strcpy_s(combined_windivert_rules, sizeof(combined_windivert_rules), combined_windivert_rules_ptr);
        if (hWindivert != nullptr)
        {
            logger->info("deleting old filter");
            if (!WinDivertClose(hWindivert))
            {
                logger->error("Failed to close WinDivert device, {}", GetLastError());
            }
        }
        logger->info("creating new filter");
        hWindivert = WinDivertOpen(combined_windivert_rules, WINDIVERT_LAYER_NETWORK, priority, 0);
        if (hWindivert == INVALID_HANDLE_VALUE)
        {
            if (GetLastError() == ERROR_INVALID_PARAMETER && !WinDivertHelperCompileFilter(combined_windivert_rules_ptr, WINDIVERT_LAYER_NETWORK, nullptr, 0, &err_str, nullptr))
            {
                logger->error("Invalid filter, {}", err_str);
                exit(EXIT_FAILURE);
            }
            logger->error("Failed to open WinDivert device, {}", GetLastError());
            exit(EXIT_FAILURE);
        }
        if (!filterThreadRunning)
        {
            logger->info("Starting windivert thread");
            std::thread myThread([this] { this->WinDivertFilterThread(); });
            myThread.detach();
            filterThreadRunning = true;
        }
    }


    // Dump packet info
    void WinDivertShit::LogPacket(packet_data* packet)
    {
        std::string packet_info = "";

        if (!passthrough)
        {
            packet_info += "BLOCK";
        }

        if (packet->tcp_header != nullptr)
        {
            packet_info += " tcp.Flags=";
            if (packet->tcp_header->Fin)
            {
                packet_info += "[FIN]";
            }
            if (packet->tcp_header->Rst)
            {
                packet_info += "[RST]";
            }
            if (packet->tcp_header->Urg)
            {
                packet_info += "[URG]";
            }
            if (packet->tcp_header->Syn)
            {
                packet_info += "[SYN]";
            }
            if (packet->tcp_header->Psh)
            {
                packet_info += "[PSH]";
            }
            if (packet->tcp_header->Ack)
            {
                packet_info += "[ACK]";
            }

            if (packet->tcp_header->Ack)
            {
                packet_info += " AckNum=" + std::to_string(ntohl(packet->tcp_header->AckNum));
            }
            packet_info += " SeqNum=" + std::to_string(ntohl(packet->tcp_header->SeqNum));
        }

        if (packet->udp_header != nullptr)
        {
            packet_info += " udp.SrcPort=" + std::to_string(ntohs(packet->udp_header->SrcPort));
            packet_info += " udp.DstPort=" + std::to_string(ntohs(packet->udp_header->DstPort));
        }

        packet_info += " size=" + std::to_string(packet->receive_length);
        logger->info(packet_info);
    }


    // this is really jank i need to rework this
    bool WinDivertShit::should_reinject(packet_data* packet)
    {
        // the Is{port} thing is not associated with the limit types so it has to have a separate if statement for each rn
        if (Is3074DL(packet))
        {
            if (!Limit::GetLimitPtrByType(_limit_ptr_vector, Klim::limit_3074_dl)->load().block)
            {
                return true;
            }
        }

        if (Is3074UL(packet))
        {
            if (!Limit::GetLimitPtrByType(_limit_ptr_vector, Klim::limit_3074_ul)->load().block)
            {
                return true;
            }
        }

        if (Is27kDL(packet))
        {
            if (!Limit::GetLimitPtrByType(_limit_ptr_vector, Klim::limit_27k_dl)->load().block)
            {
                return true;
            }
        }

        if (Is27kUL(packet))
        {
            if (!Limit::GetLimitPtrByType(_limit_ptr_vector, Klim::limit_27k_ul)->load().block)
            {
                return true;
            }
        }

        if (Is30kDL(packet))
        {
            if (!Limit::GetLimitPtrByType(_limit_ptr_vector, Klim::limit_30k_dl)->load().block)
            {
                return true;
            }
        }

        if (Is7500DL(packet))
        {
            if (!Limit::GetLimitPtrByType(_limit_ptr_vector, Klim::limit_7500_dl)->load().block)
            {
                return true;
            }
        }

        return false;
    };

    // inject all given packets
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
                        logger->info("reinjecting packets ({})...", packet_buffer->size());
                        printed_start = true;
                    }

                    if (!WinDivertSendEx(hWindivert, packet_data.packet, sizeof(packet_data.packet), nullptr, 0, &packet_data.receive_address, address_length, nullptr))
                    {
                        if (GetLastError() != 6) // expected error so no need for debug output
                        {
                            logger->warn("Failed to re-inject buffer packet: {}. retrying...", GetLastError());
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

    // function containing the main loop for packet processing
    unsigned long WinDivertShit::WinDivertFilterThread()
    {
        UINT address_length = sizeof(WINDIVERT_ADDRESS);
        // contains all blocked packets
        std::unique_ptr<std::vector<packet_data>> packet_buffer = std::make_unique<std::vector<packet_data>>();
        // current packet
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
                    logger->warn("switching filters.. {}", GetLastError());
                }
                continue;
            }

            if (!WinDivertHelperParsePacket(cur_packet.packet, cur_packet.receive_length, nullptr, nullptr, nullptr, nullptr, nullptr, &cur_packet.tcp_header, &cur_packet.udp_header, nullptr, &cur_packet.receive_length, nullptr, nullptr))
            {
                logger->warn("parse failed: {}", GetLastError());
            }

            if (!WinDivertHelperCalcChecksums(cur_packet.packet, sizeof(cur_packet.packet), nullptr, 0))
            {
                logger->warn("Failed to recalculate checksums: {}", GetLastError());
            }

            if (Is7500DL(&cur_packet) && !cur_packet.tcp_header->Fin && !cur_packet.tcp_header->Psh)
            {
                passthrough = true;
            }

            if (_settings->force_passthrough)
            {
                passthrough = true;
            }
            if (passthrough)
            {
                if (!WinDivertSendEx(hWindivert, cur_packet.packet, sizeof(cur_packet.packet), nullptr, 0, &cur_packet.receive_address, address_length, nullptr))
                {
                    logger->warn("Failed to re-inject packet: {}", GetLastError());
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
