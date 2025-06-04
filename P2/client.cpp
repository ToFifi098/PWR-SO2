// chat_client.cpp
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define PORT 12345
#define SERVER "127.0.0.1"

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void receive_messages(SOCKET s) {
    char buffer[1024];
    int recv_size;
    while ((recv_size = recv(s, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[recv_size] = '\0';

        std::cout << "\r";
        std::cout << std::string(100, ' ') << "\r";

        set_color(11);
        std::cout << buffer << std::endl;
        set_color(14);
        std::cout << "> ";
        std::cout.flush();
    }
}

int main() {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;

    WSAStartup(MAKEWORD(2, 2), &wsa);
    s = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_addr.s_addr = inet_addr(SERVER);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    connect(s, (struct sockaddr*)&server, sizeof(server));

    std::string nickname;
    std::cout << "Enter your nickname: ";
    std::getline(std::cin, nickname);
    send(s, nickname.c_str(), nickname.size(), 0);

    std::thread recv_thread(receive_messages, s);
    std::string message;

    while (true) {
        set_color(14); // Yellow
        std::cout << "> ";  // show prompt
        std::getline(std::cin, message);
        set_color(15); // Reset color

        if (message == "/exit") break;
        send(s, message.c_str(), message.size(), 0);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
