#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>

#define BUF_SIZE 256

typedef struct {
    int thread_num;
    char server_ip[16];
    int port;
} thread_data_t;

void *client_thread(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        pthread_exit(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    inet_pton(AF_INET, data->server_ip, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        pthread_exit(NULL);
    }

    write(sockfd, &data->thread_num, sizeof(data->thread_num));

    memset(buffer, 0, BUF_SIZE);
    read(sockfd, buffer, BUF_SIZE);
    printf("Odgovor: %s", buffer);

    sleep(5);
    close(sockfd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Uporaba: %s <IP> <port> <N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[3]);
    pthread_t threads[N];
    thread_data_t data[N];

    for (int i = 0; i < N; i++) {
        data[i].thread_num = i;
        strcpy(data[i].server_ip, argv[1]);
        data[i].port = atoi(argv[2]);

        pthread_create(&threads[i], NULL, client_thread, &data[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

