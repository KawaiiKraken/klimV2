#pragma once

#include <iostream>
#include <vector>

#include "../WinDivert/windivert.h"

#define ntohs(x) WinDivertHelperNtohs(x)
#define ntohl(x) WinDivertHelperNtohl(x)
#define htons(x) WinDivertHelperHtons(x)
#define htonl(x) WinDivertHelperHtonl(x)

#define MAXBUF 0xFFFF
#define INET6_ADDRSTRLEN 45
#define IPPROTO_ICMPV6 58

class Limit;

unsigned long WinDivertFilterThread(LPVOID lpParam);
void UpdateFilter(char* combined_windivert_rules_ptr);
void SetFilterRuleString(std::vector<std::atomic<Limit>*> limit_ptr_vector, char* combined_windivert_rules);
