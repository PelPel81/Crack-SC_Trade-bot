//windivert обыч by pelpel81
//https://open.spotify.com/track/0WVwOSGn9J8Ccq5wuBDLmJ?si=77601e035e6b44b9
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "windivert.h"

#pragma comment(lib, "Ws2_32.lib")

#define FROM_IP "193.160.209.236"
#define FROM_PORT 5000
#define TO_IP "127.0.0.1"
#define TO_PORT 5001


#include <time.h>

//Лог время
void print_time() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
}


int main()
{
    HANDLE handle;
    WINDIVERT_ADDRESS addr;
    char packet[0xFFFF];
    UINT packetLen;
    UINT packetCount = 0;

    const char* filter = "ip and tcp and (tcp.DstPort == 5000 or tcp.SrcPort == 5001)";

    handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[!] WinDivertOpen failed (%lu)\n", GetLastError());
        printf("[!] Run at admin.\n");
        return 1;
    }

    printf("[+] NAT redirect %s:%d <-> %s:%d started\n", FROM_IP, FROM_PORT, TO_IP, TO_PORT);

    while (TRUE)
    {
        if (!WinDivertRecv(handle, packet, sizeof(packet), &packetLen, &addr))
            continue;

        PWINDIVERT_IPHDR ip_header;
        PWINDIVERT_TCPHDR tcp_header;
        UINT8* payload;

        if (!WinDivertHelperParsePacket(
            packet, packetLen,
            &ip_header, NULL, NULL, NULL, NULL,
            &tcp_header, NULL, (PVOID*)&payload, NULL,
            NULL, NULL))
            continue;

        if (ip_header == NULL || tcp_header == NULL)
            continue;

        packetCount++;

        // Запрос от клиента 
        if (ip_header->DstAddr == inet_addr(FROM_IP) && ntohs(tcp_header->DstPort) == FROM_PORT) {
            print_time();
            printf("[#%u] -> client -> server | %s:%d -> %s:%d (%d byte)\n",
                packetCount,
                inet_ntoa(*(struct in_addr*)&ip_header->SrcAddr),
                ntohs(tcp_header->SrcPort),
                FROM_IP,
                FROM_PORT,
                packetLen);

            ip_header->DstAddr = inet_addr(TO_IP);
            tcp_header->DstPort = htons(TO_PORT);

            // Ответ 
        }
        else if (ip_header->SrcAddr == inet_addr(TO_IP) && ntohs(tcp_header->SrcPort) == TO_PORT) {
            print_time();
            printf("[#%u] <- server -> client | %s:%d <- %s:%d (%d byte)\n",
                packetCount,
                inet_ntoa(*(struct in_addr*)&ip_header->DstAddr),
                ntohs(tcp_header->DstPort),
                TO_IP,
                TO_PORT,
                packetLen);

            ip_header->SrcAddr = inet_addr(FROM_IP);
            tcp_header->SrcPort = htons(FROM_PORT);
        }
        else {
            continue;
        }

        WinDivertHelperCalcChecksums(packet, packetLen, WINDIVERT_LAYER_NETWORK, 0);
        WinDivertSend(handle, packet, packetLen, NULL, &addr);
    }

    WinDivertClose(handle);
    return 0;
}
