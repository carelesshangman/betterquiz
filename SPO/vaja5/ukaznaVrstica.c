#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 32
#define MAX_CMD  128

// lastna parse funkcija (za sode ukaze)
int parse(char *cmd, char *args[]) {
    int count = 0;
    int i = 0;

    while (cmd[i] != '\0') {
        while (cmd[i] == ' ') i++;      // preskoči presledke
        if (cmd[i] == '\0') break;

        args[count++] = &cmd[i];         // začetek argumenta

        // poišči konec argumenta
        while (cmd[i] != '\0' && cmd[i] != ' ') i++;

        // zamenjaj presledek z '\0'
        if (cmd[i] == ' ') {
            cmd[i] = '\0';
            i++;
        }
    }

    args[count] = NULL;
    return count;
}

// parser z strtok (za lihe ukaze)
int parse_strtok(char *cmd, char *args[]) {
    int count = 0;
    char *token = strtok(cmd, " ");

    while (token != NULL && count < MAX_ARGS - 1) {
        args[count++] = token;
        token = strtok(NULL, " ");
    }

    args[count] = NULL;
    return count;
}

int main() {
    char cmd[MAX_CMD];
    char *args[MAX_ARGS];
    int ukaz_st = 0;

    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            printf("\nExiting...\n");
            break;
        }

        // odstrani '\n'
        size_t len = strlen(cmd);
        if (len > 0 && cmd[len - 1] == '\n') cmd[len - 1] = '\0';

        if (cmd[0] == '\0') continue;      // prazna vrstica

        // sodo → parse(), liho → strtok()
        ukaz_st++;
        if (ukaz_st % 2 == 0) {
            parse(cmd, args);
        } else {
            parse_strtok(cmd, args);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            // otrok izvede ukaz z argumenti
            execvp(args[0], args);

            perror("execvp"); 
            exit(1);
        } else {
            // starš počaka - prepreči zombije
            waitpid(pid, NULL, 0);
        }
    }

    return 0;
}

