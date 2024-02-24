#pragma once

#include "../WinDivert/windivert.h"
#include "Limit.h"
#include <iomanip>
#include <thread>
#include <vector>

#define ntohs(x) WinDivertHelperNtohs(x)
#define ntohl(x) WinDivertHelperNtohl(x)
#define htons(x) WinDivertHelperHtons(x)
#define htonl(x) WinDivertHelperHtonl(x)

#define MAXBUF 0xFFFF
#define INET6_ADDRSTRLEN 45
#define IPPROTO_ICMPV6 58

namespace Klim
{
    class Limit;
    class UserInterface;

    class WinDivertShit
    {
        public:
            WinDivertShit(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, UserInterface* _ui_instance);
            unsigned long WinDivertFilterThread();
            void UpdateFilter(char* combined_windivert_rules_ptr);
            void SetFilterRuleString(std::vector<std::atomic<Limit>*> limit_ptr_vector, char* combined_windivert_rules);
            bool passthrough = false;

            struct packet_data
            {
                    unsigned char packet[MAXBUF];
                    UINT receive_length;
                    WINDIVERT_ADDRESS receive_address;
                    PWINDIVERT_TCPHDR tcp_header;
                    PWINDIVERT_UDPHDR udp_header;
                    std::chrono::time_point<std::chrono::high_resolution_clock> recv_time;
                    bool buffer = false;
            };

        private:
            void LogPacket(packet_data* packet);
            // void ReinjectAll(std::unique_ptr<std::vector<packet_data>> packet_buffer);
            void ReinjectAll(std::vector<packet_data>* packet_buffer);
            bool reinject = false;
            std::vector<std::atomic<Limit>*> _limit_ptr_vector;
            UserInterface* _ui_instance;
            HANDLE console;
    };
}
