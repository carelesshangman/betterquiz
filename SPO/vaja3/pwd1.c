#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH 4096

// Funkcija, ki poenostavljeno implementira getcwd()
char *my_getcwd(char *buf, size_t size) {
    struct stat current_stat, parent_stat;
    char *path = malloc(MAX_PATH);
    if (!path) {
        perror("malloc");
        return NULL;
    }
    path[0] = '\0'; // začnemo z prazno potjo

    while (1) {
        // Preberi podatke o trenutnem in starševskem direktoriju
        if (stat(".", &current_stat) == -1) {
            perror("stat .");
            free(path);
            return NULL;
        }

        if (stat("..", &parent_stat) == -1) {
            perror("stat ..");
            free(path);
            return NULL;
        }

        // Če imata trenutni in starševski isti inode, smo v korenskem direktoriju
        if (current_stat.st_ino == parent_stat.st_ino &&
            current_stat.st_dev == parent_stat.st_dev) {
            // Če smo v korenu, dodamo začetni '/'
            if (strlen(path) == 0)
                strcpy(path, "/");
            break;
        }

        // Premaknemo se v nadrejeni direktorij
        if (chdir("..") == -1) {
            perror("chdir ..");
            free(path);
            return NULL;
        }

        // Poiščemo ime trenutnega direktorija v nadrejenem direktoriju
        DIR *dir = opendir(".");
        if (!dir) {
            perror("opendir");
            free(path);
            return NULL;
        }

        struct dirent *entry;
        char dirname[256];
        dirname[0] = '\0';

        while ((entry = readdir(dir)) != NULL) {
            struct stat entry_stat;
            if (stat(entry->d_name, &entry_stat) == -1)
                continue;

            if (entry_stat.st_ino == current_stat.st_ino &&
                entry_stat.st_dev == current_stat.st_dev) {
                strncpy(dirname, entry->d_name, sizeof(dirname));
                dirname[sizeof(dirname) - 1] = '\0';
                break;
            }
        }
        closedir(dir);

        // Sestavimo pot: dodamo "/ime" pred obstoječo pot
        char temp[MAX_PATH];
        snprintf(temp, sizeof(temp), "/%s%s", dirname, path);
        strncpy(path, temp, MAX_PATH);
        path[MAX_PATH - 1] = '\0';
    }

    // Kopiramo v uporabnikov buffer (če obstaja)
    if (buf != NULL && size > 0) {
        strncpy(buf, path, size);
        buf[size - 1] = '\0';
        free(path);
        return buf;
    }

    return path; // vrnemo dinamično alociran niz
}

int main() {
    char buffer[MAX_PATH];

    if (my_getcwd(buffer, sizeof(buffer)) == NULL) {
        fprintf(stderr, "Napaka pri pridobivanju poti.\n");
        return 1;
    }

    printf("%s\n", buffer);
    return 0;
}

