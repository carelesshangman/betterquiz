#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

// Struktura za shranjevanje informacij o datoteki
typedef struct {
    char name[256];
    time_t mtime;
} FileEntry;

// Funkcija za primerjavo — od najstarejših do najnovejših
int compare_asc(const void *a, const void *b) {
    FileEntry *fa = (FileEntry *)a;
    FileEntry *fb = (FileEntry *)b;
    if (fa->mtime < fb->mtime) return -1;
    if (fa->mtime > fb->mtime) return 1;
    return 0;
}

// Funkcija za primerjavo — od najnovejših do najstarejših
int compare_desc(const void *a, const void *b) {
    return compare_asc(b, a);
}

int main(int argc, char *argv[]) {
    int reverse = 0;
    if (argc > 1 && strcmp(argv[1], "-r") == 0) {
        reverse = 1;
    }

    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    FileEntry *files = NULL;
    int count = 0;

    dir = opendir(".");
    if (dir == NULL) {
        perror("Napaka pri odpiranju direktorija");
        return 1;
    }

    // Preberi vse datoteke v trenutnem direktoriju
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (stat(entry->d_name, &fileStat) == -1) {
            perror("stat");
            continue;
        }

        files = realloc(files, (count + 1) * sizeof(FileEntry));
        if (files == NULL) {
            perror("Napaka pri alokaciji pomnilnika");
            closedir(dir);
            return 1;
        }

        strncpy(files[count].name, entry->d_name, sizeof(files[count].name));
        files[count].name[sizeof(files[count].name) - 1] = '\0';
        files[count].mtime = fileStat.st_mtime;
        count++;
    }
    closedir(dir);

    // Sortiraj po času modifikacije
    qsort(files, count, sizeof(FileEntry), reverse ? compare_desc : compare_asc);

    // Izpiši rezultate
    for (int i = 0; i < count; i++) {
        struct tm *timeinfo = localtime(&files[i].mtime);
        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", timeinfo);
        printf("%-30s  %s\n", files[i].name, timebuf);
    }

    free(files);
    return 0;
}

