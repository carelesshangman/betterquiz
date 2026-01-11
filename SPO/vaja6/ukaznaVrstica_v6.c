#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_ARGS 32
#define MAX_CMD  128

volatile sig_atomic_t sigint_pending = 0;

void sigint_handler(int signo) {
    if (!sigint_pending) {
        sigint_pending = 1;
        printf("\nZa izhod iz programa ponovno pritisnite CTRL+C v roku 4 sekund.\n");
        fflush(stdout);
        alarm(4);
    } else {
        printf("\nNasvidenje!\n");
        fflush(stdout);
        exit(0);
    }
}

void alarm_handler(int signo) {
    sigint_pending = 0;
}

int parse(char *cmd, char *args[]) {
    int count = 0;
    int i = 0;

    while (cmd[i] != '\0') {
        while (cmd[i] == ' ') i++;
        if (cmd[i] == '\0') break;

        args[count++] = &cmd[i];

        while (cmd[i] != '\0' && cmd[i] != ' ') i++;

        if (cmd[i] == ' ') {
            cmd[i] = '\0';
            i++;
        }
    }
    args[count] = NULL;
    return count;
}

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
    signal(SIGINT, sigint_handler);
    signal(SIGALRM, alarm_handler);

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

        size_t len = strlen(cmd);
        if (len > 0 && cmd[len - 1] == '\n')
            cmd[len - 1] = '\0';

        if (cmd[0] == '\0')
            continue;

        // preverimo, ali obstaja =>
        char *pipe_pos = strstr(cmd, "=>");

        if (pipe_pos == NULL) {
            // NI PIPE → navaden ukaz
            ukaz_st++;
            if (ukaz_st % 2 == 0) parse(cmd, args);
            else parse_strtok(cmd, args);

            pid_t pid = fork();
            if (pid == 0) {
                execvp(args[0], args);
                perror("execvp");
                exit(1);
            }
            waitpid(pid, NULL, 0);
            continue;
        }

        // PIPE: ukaz1 => ukaz2

        *pipe_pos = '\0';            // razdelimo vrstico
        char *left = cmd;
        char *right = pipe_pos + 2;  // preskoči "=>"

        // odstrani začetne presledke na desni strani
        while (*right == ' ') right++;

        // parsing obeh strani
        char *args_left[MAX_ARGS];
        char *args_right[MAX_ARGS];

        ukaz_st++;
        // leve in desne argumente razčlenimo z istim načinom kot prej
        if (ukaz_st % 2 == 0) {
            parse(left, args_left);
            parse(right, args_right);
        } else {
            parse_strtok(left, args_left);
            parse_strtok(right, args_right);
        }

        // Ustvarimo cev
        int fd[2];
        if (pipe(fd) < 0) {
            perror("pipe");
            continue;
        }

        // P2 - izvede levi ukaz
        pid_t p2 = fork();
        if (p2 < 0) {
            perror("fork");
            continue;
        }

        if (p2 == 0) {
            // P2 → naredi P3
            pid_t p3 = fork();
            if (p3 < 0) {
                perror("fork");
                exit(1);
            }

            if (p3 == 0) {
                // P3 (desni ukaz)
                close(fd[1]);            // zapremo pisalni konec
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);

                execvp(args_right[0], args_right);
                perror("execvp");
                exit(1);
            }

            // P2 (levi ukaz)
            close(fd[0]);               // zapremo bralni konec
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);

            execvp(args_left[0], args_left);
            perror("execvp");
            exit(1);
        }

        // P1 (lupina)
        close(fd[0]);
        close(fd[1]);

        waitpid(p2, NULL, 0);
    }

    return 0;
}

