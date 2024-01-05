#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h> 

// definicja stałych
#define MAX_CLIENTS 10
#define BUFFER_SIZE 256
#define MAX_ROOM_NAME_LENGTH 50
#define MAX_NAME_LENGTH 30

// struktura reprezentująca klienta
typedef struct {
    int socket;
    char name[MAX_NAME_LENGTH];
    char room_name[MAX_ROOM_NAME_LENGTH];
    int status;
} client;

// tablica przechowująca klientów
client clients[MAX_CLIENTS];

// mutexy dla synchronizacji dostępu do danych
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t user_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// funkcja obsługująca błędy
void error(const char* msg) {
    perror(msg);
    exit(1);
}

// funkcja rozsyłajaca wiadomość do wszystkich klientów w pokoju
void broadcast_to_room(const char* message, const char* room) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && strcmp(clients[i].room_name, room) == 0) {
            write(clients[i].socket, message, strlen(message));
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// funkcja dołączająca klienta do pokoju
void join_room(client* cli, const char* room_name) {
    char join_msg[BUFFER_SIZE];

    if (strlen(cli->room_name) > 0 && strcmp(cli->room_name, room_name) != 0) {
        snprintf(join_msg, BUFFER_SIZE, "%s has left the room %s.\n", cli->name, cli->room_name);
        printf("%s has left the room %s.\n", cli->name, cli->room_name);
        fflush(stdout);
        broadcast_to_room(join_msg, cli->room_name);
    }

    strncpy(cli->room_name, room_name, MAX_ROOM_NAME_LENGTH);
    cli->room_name[MAX_ROOM_NAME_LENGTH - 1] = '\0';

    snprintf(join_msg, BUFFER_SIZE, "%s has joined the room %s.\n", cli->name, cli->room_name);
    printf("%s has joined the room %s.\n", cli->name, cli->room_name);
    fflush(stdout);
    broadcast_to_room(join_msg, cli->room_name);
}

// funkcja odłączająca klienta od pokoju
void leave_room(client* cli) {
    char leave_msg[BUFFER_SIZE];
    if (strlen(cli->room_name) > 0) {
        snprintf(leave_msg, BUFFER_SIZE, "%s has left the room %s.\n", cli->name, cli->room_name);
        printf("%s has left the room %s.\n", cli->name, cli->room_name);
        fflush(stdout);
        broadcast_to_room(leave_msg, cli->room_name);
    }
    join_room(cli, "DefaultRoom");
}

// funkcja odłączająca klienta od serwera
void leave_server(client* cli) {
    char leave_msg[BUFFER_SIZE];

    if (strlen(cli->room_name) > 0) {
        snprintf(leave_msg, BUFFER_SIZE, "%s has left the server.\n", cli->name);
        broadcast_to_room(leave_msg, cli->room_name);
    }
    else {
        snprintf(leave_msg, BUFFER_SIZE, "%s has left the server.\n", cli->name);
    }
    if (strcmp(cli->room_name, "DefaultRoom") != 0) {
        broadcast_to_room(leave_msg, "DefaultRoom");
    }
}

// funkcja do tworzenia listy użytkowników
char* create_user_list() {
    char* user_list = (char*)malloc(BUFFER_SIZE);
    user_list[0] = '\0';

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0) {
            char user_status[20];
            snprintf(user_status, sizeof(user_status), "/%d", clients[i].status);
            strncat(user_list, clients[i].name, BUFFER_SIZE - strlen(user_list) - 1);
            strncat(user_list, user_status, BUFFER_SIZE - strlen(user_list) - 1);
            strncat(user_list, "/", BUFFER_SIZE - strlen(user_list) - 1);
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    return user_list;
}

// Wątek do wysyłania listy użytkowników co 10 sekund
void* user_list_thread(void* arg) {
    while (1) {
        sleep(10); // Oczekiwanie 10 sekund
        char* user_list = create_user_list();

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != 0) {
                char user_list_msg[BUFFER_SIZE];
                snprintf(user_list_msg, sizeof(user_list_msg), "USERLIST/%s\n", user_list);
                write(clients[i].socket, user_list_msg, strlen(user_list_msg));
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        free(user_list);
    }
    return NULL;
}

// wątek obsługujący pojedynczego klienta
void* client_handler(void* arg) {
    client* cli = (client*)arg;
    char buffer[BUFFER_SIZE];
    char channel_name[BUFFER_SIZE];
    char old_name[MAX_NAME_LENGTH];
    char response_msg[BUFFER_SIZE];
    int n;
    cli->status = 1;

    snprintf(cli->name, MAX_NAME_LENGTH, "User%d", cli->socket);
    join_room(cli, "DefaultRoom");

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        n = read(cli->socket, buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            break;
        }

        if (buffer[n - 1] == '\n') {
            buffer[n - 1] = '\0';
        }

        if (strncmp(buffer, "NICK ", 5) == 0) {
            char new_nick[MAX_NAME_LENGTH];
            strncpy(new_nick, buffer + 5, MAX_NAME_LENGTH);
            new_nick[MAX_NAME_LENGTH - 1] = '\0';

            strncpy(old_name, cli->name, MAX_NAME_LENGTH);
            // Sprawdzanie unikalno�ci nicku
            int nick_exists = 0;
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket != 0 && strcasecmp(clients[i].name, new_nick) == 0) {
                    nick_exists = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);

            if (nick_exists) {
                char nick_error_msg[BUFFER_SIZE];
                snprintf(nick_error_msg, BUFFER_SIZE, "Nick %s is already in use.\n", new_nick);
                write(cli->socket, nick_error_msg, strlen(nick_error_msg));
            }
            else {
                new_nick[0] = toupper(new_nick[0]);
                for (int i = 1; new_nick[i]; i++) {
                    new_nick[i] = tolower(new_nick[i]);
                }

                strncpy(cli->name, new_nick, MAX_NAME_LENGTH);
                cli->name[MAX_NAME_LENGTH - 1] = '\0';

                char nick_change_msg[BUFFER_SIZE];
                snprintf(nick_change_msg, BUFFER_SIZE, "%s is now %s\n", old_name, cli->name);
                broadcast_to_room(nick_change_msg, cli->room_name);
            }
        }
        else if (strncmp(buffer, "JOIN ", 5) == 0) {
            strncpy(channel_name, buffer + 5, BUFFER_SIZE - 5);
            channel_name[BUFFER_SIZE - 1] = '\0';
            if (channel_name[0] != '\0') {
                channel_name[0] = toupper(channel_name[0]);
                for (int i = 1; channel_name[i]; i++) {
                    channel_name[i] = tolower(channel_name[i]);
                }
            }
            join_room(cli, channel_name);
        }
        else if (strncmp(buffer, "LEAVE", 5) == 0) {
            leave_room(cli);
        }
        else if (strncmp(buffer, "WHERE", 5) == 0) {
            snprintf(response_msg, BUFFER_SIZE, "You are in room: %s\n", cli->room_name);
            printf("You are in room: %s\n", cli->room_name);
            fflush(stdout);
            write(cli->socket, response_msg, strlen(response_msg));
        }
        else if (strncmp(buffer, "EXIT", 4) == 0) {
            leave_server(cli);
            cli->status = 0; // Ustawienie statusu na offline
            break; // Wyj�cie z p�tli, co spowoduje roz��czenie klienta
        }
        else {
            char msg_with_sender[BUFFER_SIZE + MAX_NAME_LENGTH];
            snprintf(msg_with_sender, sizeof(msg_with_sender), "%s: %s\n", cli->name, buffer);
            broadcast_to_room(msg_with_sender, cli->room_name);
            printf("Received message from %s: %s\n", cli->name, buffer);
            fflush(stdout);
        }
    }
    cli->status = 0;
    leave_server(cli);
    close(cli->socket);
    pthread_mutex_lock(&clients_mutex);
    cli->socket = 0;
    pthread_mutex_unlock(&clients_mutex);
    return NULL;
}

// główna funkcja programu
int main(int argc, char* argv[]) {
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // sprawdzenie czy podano numer portu jako argument
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // utworzenie gniazda
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // inicjalizacja struktury adresu serwera
    bzero((char*)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // przypisanie adresu do gniazda
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // nasłuchiwanie na gnieździe
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    pthread_t threads[MAX_CLIENTS];
    int thread_count = 0;

    // tworzenie wątku do wysyłania listy użytkowników
    pthread_t user_list_tid;
    pthread_create(&user_list_tid, NULL, user_list_thread, NULL);

    // akceptowanie klientów
    while (thread_count < MAX_CLIENTS) {
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, (socklen_t*)&clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        clients[thread_count].socket = newsockfd;
        pthread_create(&threads[thread_count], NULL, client_handler, &clients[thread_count]);
        thread_count++;
    }

    // oczekiwanie na zakończenie wątków obsługujących klientów
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // zamknięcie gniazda
    close(sockfd);
    return 0;
}
