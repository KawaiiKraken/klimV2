#include "../WinDivert/windivert.h"
#include <iostream>

#define ntohs(x)            WinDivertHelperNtohs(x)
#define ntohl(x)            WinDivertHelperNtohl(x)
#define htons(x)            WinDivertHelperHtons(x)
#define htonl(x)            WinDivertHelperHtonl(x)

#define MAXBUF              0xFFFF
#define INET6_ADDRSTRLEN    45
#define IPPROTO_ICMPV6      58


// actual functions
unsigned long WindivertFilterThread( LPVOID lpParam );
void UpdateFilter( char* myNetRules );