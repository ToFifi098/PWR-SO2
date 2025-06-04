# Prosty system chatu klient-serwer

## Uruchomienie projektu

## Opis ogólny

Projekt ten to prosty system czatu klient–serwer stworzony w języku C++ przy użyciu gniazd Windows (Winsock). Pozwala wielu klientom na jednoczesne połączenie się z serwerem, przesyłanie wiadomości i otrzymywanie ich od innych użytkowników w czasie rzeczywistym.
Program składa się z dwóch części:
chat_server.cpp – Serwer czatu obsługujący wielu klientów jednocześnie za pomocą wątków i protokołu TCP.
chat_client.cpp – Klient czatu, który łączy się z serwerem, wysyła wiadomości i odbiera je od innych użytkowników.

## Jak działa

Serwer (chat_server.cpp)
Nasłuchuje na porcie 12345.
Przyjmuje maksymalnie MAX_CLIENTS (domyślnie 10) jednoczesnych klientów.
Odbiera od każdego klienta jego pseudonim.
Każdy klient jest obsługiwany w osobnym wątku (CreateThread).
Wiadomości od jednego klienta są broadcastowane do wszystkich pozostałych.

Klient (chat_client.cpp)
Łączy się z serwerem (domyślnie 127.0.0.1:12345).
Wysyła pseudonim.
Uruchamia osobny wątek do odbierania wiadomości.
Pozwala użytkownikowi pisać i wysyłać wiadomości w czasie rzeczywistym.
Komenda /exit kończy połączenie.

## Najważniejsze części kodu i funkcje

broadcast(const std::string& message, SOCKET sender)
Rozsyła wiadomość do wszystkich klientów oprócz nadawcy. 
Funkcja ta zabezpieczona jest przed równoczesnym dostępem przez użycie mechanizmu synchronizacji 
(lock() / unlock()), ponieważ może być wywoływana równocześnie przez wiele wątków.
`client_handler(LPVOID lpParam)`

Wątek uruchamiany dla każdego klienta. Obsługuje:
•	rejestrację klienta (przyjęcie pseudonimu),
•	odbieranie i przekazywanie wiadomości,
•	usuwanie klienta po jego rozłączeniu.
`lock()` i `unlock()`

Prosty mechanizm sekcji krytycznej:
•	`InterlockedCompareExchange` sprawdza i ustawia zmienną mutex, blokując inne wątki.
•	`Sleep(1)` w pętli zapobiega zużyciu 100% CPU.
•	`unlock()` resetuje mutex.
Dzięki temu tylko jeden wątek może w danej chwili:
•	modyfikować listę klientów (clients),
•	zmieniać wartość `client_count`,
•	nadawać wiadomości (aby uniknąć kolizji przy `send()`).
`get_timestamp()`
Zwraca aktualną godzinę w formacie [HH:MM:SS], która jest dołączana do każdej wiadomości.

## Sekcje krytyczne

W programie zastosowano ręczną synchronizację wątków przy użyciu zmiennej volatile LONG mutex.
Przykłady użycia:

```
lock();
clients[client_count++] = { client_socket, nickname };
unlock();
```

```
lock();
for (int i = 0; i < client_count; ++i) {
    if (clients[i].socket != sender) {
        send(clients[i].socket, message.c_str(), message.size(), 0);
    }
}
unlock();
```
Wiele wątków działa równolegle – każdy klient ma własny wątek. Bez synchronizacji mogłoby dojść do błędów wyścigu, 
możliwy byłby błąd w indeksowaniu tablicy klientów, dane mogłyby być uszkodzone (np. nadpisane lub utracone).

