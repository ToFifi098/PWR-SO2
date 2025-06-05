#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>

// Automatyczne linkowanie biblioteki Winsock
#pragma comment(lib, "ws2_32.lib")

// Definicje stałych
#define PORT 12345      // Port serwera
#define SERVER "127.0.0.1" // Adres IP serwera (localhost)

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Funkcja odbierająca wiadomości z serwera (uruchamiana w osobnym wątku)
void receive_messages(SOCKET s) {
    char buffer[1024];  // Bufor na odbierane wiadomości
    int recv_size;      // Rozmiar odebranej wiadomości

    // Nieskończona pętla odbierająca wiadomości
    while ((recv_size = recv(s, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[recv_size] = '\0';

        std::cout << "\r";
        std::cout << std::string(100, ' ');
        std::cout << "\r";

        set_color(11);
        std::cout << buffer << std::endl;
        set_color(14);
        std::cout << "> ";
        std::cout.flush();
    }
}

int main() {
    WSADATA wsa;            // Struktura Winsock
    SOCKET s;               // Gniazdo klienta
    struct sockaddr_in server; // Struktura adresu serwera

    // Inicjalizacja Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Utworzenie gniazda (TCP, strumieniowe)
    s = socket(AF_INET, SOCK_STREAM, 0);

    // Konfiguracja adresu serwera
    server.sin_addr.s_addr = inet_addr(SERVER); // Adres IP serwera
    server.sin_family = AF_INET;               // Rodzina adresów - IPv4
    server.sin_port = htons(PORT);             // Port w kolejności sieciowej

    // Nawiązanie połączenia z serwerem
    connect(s, (struct sockaddr*)&server, sizeof(server));

    // Pobranie pseudonimu od użytkownika
    std::string nickname;
    set_color(14);
    std::cout << "Enter your nickname: ";
    set_color(15);
    std::getline(std::cin, nickname);

    // Wysłanie pseudonimu do serwera
    send(s, nickname.c_str(), nickname.size(), 0);

    // Uruchomienie wątku odbierającego wiadomości
    std::thread recv_thread(receive_messages, s);

    std::string message; // Zmienna na wiadomości do wysłania

    // Główna pętla wysyłająca wiadomości
    while (true) {
        set_color(14);
        std::cout << "> ";
        set_color(15);
        std::getline(std::cin, message);

        // Wyjście z pętli jeśli wpisano /exit
        if (message == "/exit") break;

        // Wysłanie wiadomości do serwera
        send(s, message.c_str(), message.size(), 0);
    }

    // Zamknięcie gniazda
    closesocket(s);
    WSACleanup();
    return 0;
}