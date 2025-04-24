#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048

static unsigned int cli_count = 0;
static int uid = 10;

typedef struct {
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

FILE *log_file;

void trim_newline(char *str) {
    int len = strlen(str);
    if (str[len - 1] == '\n')
        str[len - 1] = '\0';
}

void add_client(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(!clients[i]){
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int uid) {
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i]->uid == uid){
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *s, int uid) {
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i]->uid != uid){
                write(clients[i]->sockfd, s, strlen(s));
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    char buff_out[BUFFER_SZ];
    int leave_flag = 0;

    client_t *cli = (client_t *)arg;

    while(1){
        if (leave_flag) break;

        int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
        if (receive > 0){
            if(strlen(buff_out) > 0){
                // Asegurar salto de línea
                size_t len = strlen(buff_out);
                if (buff_out[len - 1] != '\n') {
                    strcat(buff_out, "\n");
                }

                send_message(buff_out, cli->uid);
                printf("%s", buff_out);

                fprintf(log_file, "Mensaje de %s: %s", cli->name, buff_out);
                fflush(log_file);
            }
        } else if (receive == 0 || strcmp(buff_out, "exit") == 0){
            sprintf(buff_out, "%s ha salido del chat.\n", cli->name);
            printf("%s", buff_out);
            send_message(buff_out, cli->uid);

            fprintf(log_file, "Desconexión: %s [%d]\n", cli->name, cli->uid);
            fflush(log_file);

            leave_flag = 1;
        }

        bzero(buff_out, BUFFER_SZ);
    }

    close(cli->sockfd);
    remove_client(cli->uid);
    free(cli);
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    log_file = fopen("chat_server.log", "a");
    if (!log_file) {
        perror("No se pudo abrir el archivo de log");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int option = 1;
    int listenfd = 0, connfd = 0;

    struct sockaddr_in serv_addr, cli_addr;
    pthread_t tid;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);

    printf("=== Servidor de chat iniciado en el puerto %d ===\n", port);

    while(1){
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        if((cli_count + 1) == MAX_CLIENTS){
            printf("Máximo de clientes alcanzado. Conexión rechazada.\n");
            close(connfd);
            continue;
        }

        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;

        // Recibir nombre antes de crear hilo
        char name_buffer[32];
        if (recv(connfd, name_buffer, 32, 0) <= 0 || strlen(name_buffer) < 2 || strlen(name_buffer) >= 31) {
            printf("Cliente sin nombre. Conexión rechazada.\n");
            close(connfd);
            free(cli);
            continue;
        }
        strcpy(cli->name, name_buffer);

        add_client(cli);

        // Notificar ingreso
        char join_msg[BUFFER_SZ];
        sprintf(join_msg, "%s se ha unido al chat.\n", cli->name);
        send_message(join_msg, cli->uid);
        printf("%s", join_msg);

        fprintf(log_file, "Conexión: %s [%d]\n", cli->name, cli->uid);
        fflush(log_file);

        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        sleep(1);
    }

    fclose(log_file);
    return 0;
}
