#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")

char ip[INET_ADDRSTRLEN];

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket failed\n";
        return 1;
    }

    // enable broadcst
    BOOL broadcast = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast));

    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(40000);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    const char* message = "DISCOVER_MY_SERVER";

    int sent = sendto(sock, message, (int)strlen(message), 0,
        (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    if (sent == SOCKET_ERROR) {
        std::cerr << "sendto failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // answer
    char buffer[256];
    sockaddr_in from{};
    int from_len = sizeof(from);

    timeval tv;
	tv.tv_sec = 2;   // 2 seconds timeout
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
        (sockaddr*)&from, &from_len);
    if (n > 0) {
        buffer[n] = '\0';
        inet_ntop(AF_INET, &from.sin_addr, ip, sizeof(ip));
        std::cout << "Received reply from " << ip << ": " << buffer << "\n";
    }
    else {
        std::cerr << "No reply received\n";
    }


    //////////////// tcp

    SOCKET tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp == INVALID_SOCKET) {
        std::cerr << "socket failed\n";
        return 1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(54000);
    inet_pton(AF_INET, /*"127.0.0.1"*/ ip, &server.sin_addr);

    if (connect(tcp, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "connect failed\n";
        return 1;
    }

	while (true)
	{
		std::cout << "Press Enter to get server time...";
		std::cin.get();

        send(tcp, "TIME\n", 5, 0);

        char buffer[256]{};
        int received = recv(tcp, buffer, sizeof(buffer) - 1, 0);

        if (received > 0) {
            buffer[received] = '\0';
            std::cout << "Server time: " << buffer;
        }
		else
        {
			std::cout << "Server disconnected\n";
			break;
		}
	}

    closesocket(tcp);
    WSACleanup();
}