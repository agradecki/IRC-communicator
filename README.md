# Komunikator internetowy typu IRC

## Opis projektu

Założeniem projektu było stworzenie komunikatora internetowego typu IRC składającej się z klienta i serwera. Klient został napisany w języku Python przy użyciu biblioteki Tkinter, a serwer w języku C. Aplikacja umożliwia komunikację pomiędzy klientami w różnych pokojach, obsługę listy użytkowników, zmianę nicku, opuszczenie lub dołączenie do pokoju.

## Protokół komunikacyjny

Protokół komunikacyjny obejmuje kilka komend, takich jak:

- `NICK <nowy_nick>`: Zmiana nicku użytkownika.
- `JOIN <nazwa_pokoju>`: Dołączenie do określonego pokoju.
- `LEAVE`: Opuszczenie obecnego pokoju.
- `WHERE`: Wyświetlenie informacji o aktualnym pokoju.
- `EXIT`: Opuszczenie serwera.

Serwer przesyła również informacje o liście użytkowników w formie `USERLIST/użytkownik1/status1/użytkownik2/status2/...`.

## Implementacja

### Klient (język Python - biblioteka Tkinter)

Kod klienta został napisany w języku Python z wykorzystaniem biblioteki Tkinter do stworzenia interfejsu graficznego. Aplikacja utrzymuje połączenie z serwerem, obsługuje odbieranie wiadomości, aktualizuje listę użytkowników oraz umożliwia wysyłanie wiadomości do wybranego pokoju.

### Serwer (język C)

Kod serwera został napisany w języku C, używa gniazd do obsługi komunikacji sieciowej pomiędzy klientami. Zawiera funkcje do dołączania do pokojów, zmiany nicku, opuszczania pokojów oraz opuszczania serwera. Dodatkowo, serwer wysyła listę użytkowników co 10 sekund.

## Pliki źródłowe

### Klient

- `client.py`: Kod klienta zawierający interfejs graficzny, funkcje obsługi wiadomości, aktualizacji listy użytkowników i obsługi połączenia z serwerem.

### Serwer

- `server.c`: Kod serwera zawierający funkcje obsługi klientów, komend oraz logikę dołączania i opuszczania pokojów.

## Kompilacja, uruchomienie i obsługa

### Klient

1. Upewnij się, że masz zainstalowaną bibliotekę Tkinter: `pip install tk`.
2. Uruchom klienta: `python client.py`.

### Serwer

1. Kompiluj serwer: `gcc server.c -o server -fopenmp -lm`.
2. Uruchom serwer, podając numer portu jako argument: `./server <numer_portu>`.
3. Serwer będzie nasłuchiwał na podanym porcie.

## Podsumowanie

Projekt obejmował stworzenie w pełni funkcjonalnego komunikatora typu IRC z interfejsem graficznym dla klienta. Implementacja komunikacji między klientami opiera się na protokole komunikacyjnym, a serwer obsługuje funkcje dołączania i opuszczania pokojów oraz wysyłania listy użytkowników. Projekt jest gotowy do zastosowania w czasie rzeczywistym.

## Autorzy

Alan Grądecki 151126<br />
Mikołaj Jankowski 144267
