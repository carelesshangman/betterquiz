#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>


#define BACKLOG 10
#define BUF_SIZE 256

void sigchld_handler(int sig)
{
    int saved_errno = errno;
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Otrok s PID %d se je končal\n", pid);
    }

    errno = saved_errno;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uporaba: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    /* SIGCHLD handler */
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

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

    printf("Strežnik posluša na 127.0.0.1:%d\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) { /* otrok */
            close(server_fd);

            int thread_num;
            read(client_fd, &thread_num, sizeof(thread_num));

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            int client_port = ntohs(client_addr.sin_port);

            pid_t mypid = getpid();
            printf("Otrok %d strežnika sem stregel niti %d odjemalca (%s:%d)\n",
                   mypid, thread_num, client_ip, client_port);

            char msg[BUF_SIZE];
            if (thread_num != 0 && mypid % thread_num == 0) {
                snprintf(msg, BUF_SIZE,
                         "Nit %d: Cestitam, dobili ste nagrado!\n", thread_num);
            } else {
                snprintf(msg, BUF_SIZE,
                         "Nit %d: Vazno je sodelovati, ne zmagati.\n", thread_num);
            }

            write(client_fd, msg, strlen(msg));
            close(client_fd);
            exit(0);
        } else {
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}

