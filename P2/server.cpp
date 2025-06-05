#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

// Automatyczne linkowanie biblioteki Winsock
#pragma comment(lib, "ws2_32.lib")

// Definicje stałych
#define MAX_CLIENTS 10  // Maksymalna liczba klientów
#define PORT 12345      // Port serwera

// Struktura przechowująca informacje o kliencie
struct Client {
    SOCKET socket;      // Gniazdo sieciowe klienta
    std::string nickname; // Pseudonim klienta
};

// Globalne zmienne
Client clients[MAX_CLIENTS];  // Tablica przechowująca klientów
int client_count = 0;         // Liczba aktualnie podłączonych klientów
volatile LONG mutex = 0;      // Zmienna do synchronizacji wątków

// Funkcja blokująca dostęp do zasobów współdzielonych
void lock() {
    // Czekaj aż mutex będzie równy 0 i ustaw go na 1
    while (InterlockedCompareExchange(&mutex, 1, 0) != 0) {
        Sleep(1);  // Czekaj 1ms przed ponowną próbą
    }
}

// Funkcja odblokowująca dostęp do zasobów współdzielonych
void unlock() {
    InterlockedExchange(&mutex, 0);  //ustaw mutex na 0
}

// Funkcja zwracająca aktualny czas w formacie [HH:MM:SS]
std::string get_timestamp() {
    time_t now = time(nullptr);      // Pobierz aktualny czas
    struct tm* local = localtime(&now); // Konwertuj na czas lokalny
    char buffer[16];                 // Bufor na wynik
    strftime(buffer, sizeof(buffer), "[%H:%M:%S] ", local);
    return std::string(buffer);
}

// Funkcja rozsyłająca wiadomość do wszystkich klientów oprócz nadawcy
void broadcast(const std::string& message, SOCKET sender) {
    lock();  // Zablokuj dostęp do tablicy klientów

    // Przeiteruj przez wszystkich klientów
    for (int i = 0; i < client_count; ++i) {
        // Jeśli to nie jest nadawca, wyślij wiadomość
        if (clients[i].socket != sender) {
            send(clients[i].socket, message.c_str(), message.size(), 0);
        }
    }

    unlock();  // Odblokuj dostęp
}

// Funkcja obsługująca pojedynczego klienta (uruchamiana w osobnym wątku)
DWORD WINAPI client_handler(LPVOID lpParam) {
    SOCKET client_socket = (SOCKET)lpParam; // Pobierz gniazdo klienta
    char buffer[1024];                      // Bufor na dane
    int recv_size;                          // Rozmiar odebranych danych

    // Odbierz pseudonim klienta
    recv_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (recv_size <= 0) return 1;  // Jeśli błąd, zakończ wątek
    buffer[recv_size] = '\0';
    std::string nickname(buffer);  // Zapisz pseudonim

    // Dodaj klienta do listy
    lock();
    clients[client_count++] = { client_socket, nickname };
    unlock();

    // Przygotuj i rozgłoś informację o dołączeniu klienta
    std::string join_msg = get_timestamp() + nickname + " joined the chat.\n";
    broadcast(join_msg, client_socket);
    std::cout << join_msg;  // Wyświetl na serwerze

    // Główna pętla odbierająca wiadomości od klienta
    while ((recv_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[recv_size] = '\0';
        std::string msg = get_timestamp() + nickname + ": " + buffer + "\n";
        broadcast(msg, client_socket);  // Rozgłoś do wszystkich
        std::cout << msg;               // Wyświetl na serwerze
    }

    // Przygotuj informację o opuszczeniu czatu
    std::string leave_msg = get_timestamp() + nickname + " left the chat.\n";

    // Usuń klienta z listy
    lock();
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].socket == client_socket) {
            // Przesuń ostatniego klienta na miejsce opuszczającego
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    unlock();

    // Rozgłoś informację o wyjściu
    broadcast(leave_msg, client_socket);
    std::cout << leave_msg;

    // Zamknij gniazdo klienta
    closesocket(client_socket);
    return 0;
}

// Główna funkcja serwera
int main() {
    WSADATA wsa;                         // Struktura Winsock
    SOCKET server_socket, client_socket; // Gniazda serwera i klienta
    struct sockaddr_in server, client;    // Struktury adresów
    int c = sizeof(struct sockaddr_in);   // Rozmiar struktury adresu

    // Inicjalizacja Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Utwórz gniazdo serwera
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Konfiguracja adresu serwera
    server.sin_family = AF_INET;          // Rodzina adresów - IPv4
    server.sin_addr.s_addr = INADDR_ANY;  // Akceptuj połączenia na wszystkich interfejsach
    server.sin_port = htons(PORT);        // Port w kolejności sieciowej

    // Powiąż gniazdo z adresem
    bind(server_socket, (struct sockaddr*)&server, sizeof(server));

    // Rozpocznij nasłuchiwanie połączeń
    listen(server_socket, MAX_CLIENTS);

    std::cout << "Server started on port " << PORT << "...\n";

    // Główna pętla serwera - akceptowanie połączeń
    while ((client_socket = accept(server_socket, (struct sockaddr*)&client, &c))) {
        // Dla każdego nowego klienta utwórz osobny wątek
        CreateThread(NULL, 0, client_handler, (LPVOID)client_socket, 0, NULL);
    }

    // Zamknij gniazdo serwera
    closesocket(server_socket);
    WSACleanup();
    return 0;
}