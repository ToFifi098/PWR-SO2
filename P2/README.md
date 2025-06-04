# Prosty system chatu klient-serwer
### Filip Zmyślony
### Julia Sujka

## Uruchomienie programu

Program musi być uruchomiony na systemie Windows z użyciem MinGW.
```
g++ client.cpp -o client -lws2_32 
g++ server.cpp -o server-lws2_32
```

## Opis ogólny

Projekt ten to prosty system czatu klient–serwer stworzony w języku C++ przy użyciu gniazd Windows (Winsock). Pozwala wielu klientom na jednoczesne połączenie się z serwerem, przesyłanie wiadomości i otrzymywanie ich od innych użytkowników w czasie rzeczywistym.

Program składa się z dwóch części:

chat_server.cpp – Serwer czatu obsługujący wielu klientów jednocześnie za pomocą wątków i protokołu TCP.

chat_client.cpp – Klient czatu, który łączy się z serwerem, wysyła wiadomości i odbiera je od innych użytkowników.

## Jak działa

### Serwer (chat_server.cpp)

•	Nasłuchuje na porcie 12345.

•	Przyjmuje maksymalnie 10 jednoczesnych klientów.

•	Odbiera od każdego klienta jego pseudonim.

•	Każdy klient jest obsługiwany w osobnym wątku (CreateThread).

•	Wiadomości od jednego klienta są broadcastowane do wszystkich pozostałych.


### Klient (chat_client.cpp)

•	Łączy się z serwerem (domyślnie 127.0.0.1:12345).

•	Wysyła pseudonim.

•	Uruchamia osobny wątek do odbierania wiadomości.

•	Pozwala użytkownikowi pisać i wysyłać wiadomości w czasie rzeczywistym.

•	Komenda /exit kończy połączenie.

## Najważniejsze części kodu i funkcje
## Server

### Synchronizacja wątków

```
void lock() {
    while (InterlockedCompareExchange(&mutex, 1, 0) != 0) {
        Sleep(1);
    }
}

void unlock() {
    InterlockedExchange(&mutex, 0);
}
```

`lock()` – ustawia mutex na 1 tylko wtedy, gdy był równy 0 (czyli był wolny).
`unlock()` – zwalnia dostęp, ustawiając mutex z powrotem na 0.
Zabezpiecza sekcje krytyczne: np. dodawanie/usuwanie klienta lub broadcast wiadomości.

### Broadcast wiadomości

```
void broadcast(const std::string& message, SOCKET sender) {
    lock();
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].socket != sender) {
            send(clients[i].socket, message.c_str(), message.size(), 0);
        }
    }
    unlock();
}
```

Wysyła wiadomość message do wszystkich klientów poza nadawcą. Chroniona `lock()` zapobiega równoczesnemu wysyłaniu przez wiele wątków.

### Obsługa klienta

```
SOCKET client_socket = (SOCKET)lpParam;
char buffer[1024];
int recv_size;

recv_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
if (recv_size <= 0) return 1;
buffer[recv_size] = '\0';
std::string nickname(buffer);
```

Oczekuje pierwszej wiadomości – pseudonimu. Jeśli recv zwraca <=0, klient się nie połączył prawidłowo kończy obsługę.

```
lock();
clients[client_count++] = { client_socket, nickname };
unlock();
```

Dodaje nowego klienta do listy w sposób bezpieczny wątkowo.

```
while ((recv_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
    buffer[recv_size] = '\0';
    std::string msg = get_timestamp() + nickname + ": " + buffer + "\n";
    broadcast(msg, client_socket);
    std::cout << msg;
}
```
Odbiera dane od klienta w pętli. Każdą wiadomość opatruje pseudonimem i timestampem. Przesyła ją dalej do pozostałych.

```
std::string leave_msg = get_timestamp() + nickname + " left the chat.\n";

lock();
for (int i = 0; i < client_count; ++i) {
    if (clients[i].socket == client_socket) {
        clients[i] = clients[client_count - 1];
        client_count--;
        break;
    }
}
unlock();

broadcast(leave_msg, client_socket);
std::cout << leave_msg;

closesocket(client_socket);
return 0;
```

Po zerwaniu połączenia usuwa klienta z tablicy (swap with last). Powiadamia pozostałych o jego wyjściu oraz zamyka socket i kończy wątek.


### Główna pętla akceptowania klientów

```
while ((client_socket = accept(server_socket, (struct sockaddr*)&client, &c))) {
    CreateThread(NULL, 0, client_handler, (LPVOID)client_socket, 0, NULL);
}
```

Akceptuje nowe połączenie. Tworzy nowy wątek dla każdego klienta (CreateThread z client_handler).


## Klient

### Uruchomienie klienta

```
WSADATA wsa;
SOCKET s;
struct sockaddr_in server;

WSAStartup(MAKEWORD(2, 2), &wsa);
s = socket(AF_INET, SOCK_STREAM, 0);
```
Inicjalizacja biblioteki Winsock oraz utworzenie socketu klienta TCP (SOCK_STREAM).

### Konfiguracja i połączenie

```
server.sin_addr.s_addr = inet_addr(SERVER);
server.sin_family = AF_INET;
server.sin_port = htons(PORT);

connect(s, (struct sockaddr*)&server, sizeof(server));
```
Ustawia dane serwera (adres IP i port) oraz nawiązuje połączenie z serwerem (connect).

### Uruchomienie wątku odbierającego

```
std::thread recv_thread(receive_messages, s);
```
Tworzy nowy wątek do odbioru wiadomości w czasie rzeczywistym, bez blokowania wpisywania.

