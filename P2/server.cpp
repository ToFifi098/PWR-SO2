#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include "./Semaphore.h"

// Automatyczne linkowanie biblioteki Winsock
#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 10
#define PORT 12345

struct Client {
    SOCKET socket;
    std::string nickname;
};

// Globalne zmienne
Client clients[MAX_CLIENTS];
int client_count = 0;
Semaphore stdSemaphore(1);  // Synchronizacja klientów
std::vector<std::string> chat_history; // Historia czatu

// Pobieranie aktualnego czasu
std::string get_timestamp() {
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "[%H:%M:%S] ", local);
    return std::string(buffer);
}

// Rozsyłanie wiadomości do wszystkich oprócz nadawcy
void broadcast(const std::string& message, SOCKET sender) {
    stdSemaphore.wait();

    // Zapisz wiadomość do historii
    chat_history.push_back(message);

    for (int i = 0; i < client_count; ++i) {
        if (clients[i].socket != sender) {
            send(clients[i].socket, message.c_str(), message.size(), 0);
        }
    }

    stdSemaphore.signal();
}

// Funkcja obsługująca klienta
DWORD WINAPI client_handler(LPVOID lpParam) {
    SOCKET client_socket = (SOCKET)lpParam;
    char buffer[1024];
    int recv_size;

    // Odbierz pseudonim
    recv_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (recv_size <= 0) return 1;
    buffer[recv_size] = '\0';
    std::string nickname(buffer);

    stdSemaphore.wait();
    clients[client_count++] = { client_socket, nickname };

    // Wyślij historię czatu temu użytkownikowi
    for (const std::string& msg : chat_history) {
        send(client_socket, msg.c_str(), msg.size(), 0);
    }

    stdSemaphore.signal();

    // Informacja o dołączeniu
    std::string join_msg = get_timestamp() + nickname + " joined the chat.\n";
    broadcast(join_msg, client_socket);
    std::cout << join_msg;

    // Główna pętla odbioru
    while ((recv_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[recv_size] = '\0';
        std::string msg = get_timestamp() + nickname + ": " + buffer + "\n";
        broadcast(msg, client_socket);
        std::cout << msg;
    }

    // Wyjście użytkownika
    std::string leave_msg = get_timestamp() + nickname + " left the chat.\n";

    stdSemaphore.wait();
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].socket == client_socket) {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    chat_history.push_back(leave_msg);
    stdSemaphore.signal();

    broadcast(leave_msg, client_socket);
    std::cout << leave_msg;

    closesocket(client_socket);
    return 0;
}

// Główna funkcja
int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c = sizeof(struct sockaddr_in);

    WSAStartup(MAKEWORD(2, 2), &wsa);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr*)&server, sizeof(server));
    listen(server_socket, MAX_CLIENTS);

    std::cout << "Server started on port " << PORT << "...\n";

    while ((client_socket = accept(server_socket, (struct sockaddr*)&client, &c))) {
        CreateThread(NULL, 0, client_handler, (LPVOID)client_socket, 0, NULL);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
