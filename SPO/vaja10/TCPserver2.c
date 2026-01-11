#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BACKLOG 10
#define BUF_SIZE 256

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} client_data_t;

void *client_handler(void *arg)
{
    client_data_t *data = (client_data_t *)arg;
    int client_fd = data->client_fd;
    struct sockaddr_in addr = data->client_addr;
    free(data);

    int thread_num;
    read(client_fd, &thread_num, sizeof(thread_num));

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(addr.sin_port);

    pid_t pid = getpid();   /* PID strežnika (enak za vse niti) */
    pthread_t tid = pthread_self();

    printf("Nit strežnika (TID %lu, PID %d) streže niti %d odjemalca (%s:%d)\n",
           tid, pid, thread_num, client_ip, client_port);

    char msg[BUF_SIZE];
    if (thread_num != 0 && pid % thread_num == 0) {
        snprintf(msg, BUF_SIZE,
                 "Nit %d: Cestitam, dobili ste nagrado!\n", thread_num);
    } else {
        snprintf(msg, BUF_SIZE,
                 "Nit %d: Vazno je sodelovati, ne zmagati.\n", thread_num);
    }

    write(client_fd, msg, strlen(msg));
    close(client_fd);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uporaba: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("TCP strežnik z nitmi posluša na 127.0.0.1:%d\n", port);

    while (1) {
        client_data_t *data = malloc(sizeof(client_data_t));
        socklen_t len = sizeof(data->client_addr);

        data->client_fd = accept(server_fd,
                                 (struct sockaddr *)&data->client_addr,
                                 &len);

        if (data->client_fd < 0) {
            perror("accept");
            free(data);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, data);
        pthread_detach(tid);   /* samodejno čiščenje niti */
    }

    close(server_fd);
    return 0;
}

