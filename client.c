#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>

#define LENGTH 2048

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void str_overwrite_stdout() {
    printf("\r> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length) {
    for (int i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_msg_handler() {
    char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};

    while (1) {
        str_overwrite_stdout();
        fgets(message, LENGTH, stdin);
        str_trim_lf(message, LENGTH);

        if (strcmp(message, "exit") == 0) {
            break;
        }

        if (strlen(message) == 0) {
            continue; // Ignorar mensajes vacíos
        }

        snprintf(buffer, sizeof(buffer), "%s: %s\n", name, message);
        send(sockfd, buffer, strlen(buffer), 0);

        memset(message, 0, LENGTH);
        memset(buffer, 0, LENGTH + 32);
    }

    catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
    char message[LENGTH] = {};
    while (1) {
        int receive = recv(sockfd, message, LENGTH, 0);
        if (receive > 0) {
            printf("\r%s\n", message);  // Agregado \n para evitar pegado de líneas
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        }
        memset(message, 0, LENGTH);
    }
}

int main(int argc, char **argv){
    if(argc != 3){
        printf("Uso: %s <ip_servidor> <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    signal(SIGINT, catch_ctrl_c_and_exit);

    printf("Ingrese su nombre: ");
    fgets(name, 32, stdin);
    str_trim_lf(name, strlen(name));

    if (strlen(name) < 2 || strlen(name) >= 31) {
        printf("El nombre debe tener entre 2 y 30 caracteres.\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int err = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("Error al conectar con el servidor.\n");
        return EXIT_FAILURE;
    }

    send(sockfd, name, 32, 0);

    printf("=== BIENVENIDO AL CHAT ===\n");

    pthread_t send_msg_thread;
    if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
        printf("Error al crear el hilo de envío\n");
        return EXIT_FAILURE;
    }

    pthread_t recv_msg_thread;
    if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
        printf("Error al crear el hilo de recepción\n");
        return EXIT_FAILURE;
    }

    while (1){
        if(flag){
            printf("\nSaliendo del chat...\n");
            break;
        }
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
