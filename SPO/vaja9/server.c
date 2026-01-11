#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uporaba: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    printf("Strežnik posluša na portu %d...\n", port);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        printf("Povezan odjemalec.\n");

        char buffer[256];
        int n;

        while ((n = read(client_fd, buffer, sizeof(buffer)-1)) > 0) {
            buffer[n] = '\0';
            printf("Prejel: %s", buffer);

            double a, b, result = 0.0;
            char op;

            if (sscanf(buffer, "%lf %c %lf", &a, &op, &b) != 3) {
                char *err = "Napaka pri razčlenjevanju izraza.\n";
                write(client_fd, err, strlen(err));
                continue;
            }

            switch (op) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': 
                    if (b == 0) {
                        char *err = "Napaka: deljenje z 0.\n";
                        write(client_fd, err, strlen(err));
                        continue;
                    }
                    result = a / b;
                    break;
                default:
                    {
                        char *err = "Neznan operator.\n";
                        write(client_fd, err, strlen(err));
                        continue;
                    }
            }

            char out[256];
            snprintf(out, sizeof(out), "Rezultat: %.6f\n", result);
            write(client_fd, out, strlen(out));
        }

        close(client_fd);
        printf("Odjemalec prekinil povezavo.\n");
    }

    close(server_fd);
    return 0;
}

