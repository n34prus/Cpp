#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    std::cout << "Hello World!\n";
}
