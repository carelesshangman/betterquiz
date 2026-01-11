#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uporaba: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Povezano na %s:%d\n", ip, port);

    char buffer[256];
    char recvbuf[256];
    int n;

    while (1) {
        printf("Vnesi izraz (npr. 3.5 * 2): ");
        if (!fgets(buffer, sizeof(buffer), stdin))
            break;

        if (write(sock, buffer, strlen(buffer)) < 0) {
            perror("write");
            break;
        }

        n = read(sock, recvbuf, sizeof(recvbuf)-1);
        if (n <= 0) {
            printf("Povezava zaprta.\n");
            break;
        }

        recvbuf[n] = '\0';
        printf("%s", recvbuf);
    }

    close(sock);
    return 0;
}

