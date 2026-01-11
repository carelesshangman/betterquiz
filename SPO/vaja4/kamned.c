#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define TEMP_FILE "temp_kamned.txt"

void append_lines(const char *filename);
void delete_line(const char *filename, int n);
void insert_line(const char *filename, int n);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uporaba:\n");
        fprintf(stderr, "  %s <filename> -a\n", argv[0]);
        fprintf(stderr, "  %s <filename> -d <n>\n", argv[0]);
        fprintf(stderr, "  %s <filename> -i <n>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *option = argv[2];

    if (strcmp(option, "-a") == 0) {
        append_lines(filename);
    } 
    else if (strcmp(option, "-d") == 0 && argc == 4) {
        int n = atoi(argv[3]);
        if (n <= 0) {
            fprintf(stderr, "Neveljavna številka vrstice.\n");
            return 1;
        }
        delete_line(filename, n);
    } 
    else if (strcmp(option, "-i") == 0 && argc == 4) {
        int n = atoi(argv[3]);
        if (n <= 0) {
            fprintf(stderr, "Neveljavna številka vrstice.\n");
            return 1;
        }
        insert_line(filename, n);
    } 
    else {
        fprintf(stderr, "Neveljavna uporaba argumentov.\n");
        return 1;
    }

    return 0;
}

/* --- doda vrstice na konec dat --- */
void append_lines(const char *filename) {
    FILE *f = fopen(filename, "a");
    if (!f) {
        perror("Napaka pri odpiranju datoteke");
        return;
    }

    char line[MAX_LINE];
    printf("Vnašaj vrstice (prazna vrstica za konec):\n");
    while (1) {
        if (!fgets(line, sizeof(line), stdin)) break;
        if (strcmp(line, "\n") == 0) break;
        fputs(line, f);
    }

    fclose(f);
    printf("Vrstice dodane v %s.\n", filename);
}

/* --- izbriše n-to vrstico  --- */
void delete_line(const char *filename, int n) {
    FILE *f = fopen(filename, "r");
    FILE *temp = fopen(TEMP_FILE, "w");

    if (!f || !temp) {
        perror("Napaka pri odpiranju datoteke");
        if (f) fclose(f);
        if (temp) fclose(temp);
        return;
    }

    char line[MAX_LINE];
    int line_no = 0;
    int found = 0;

    while (fgets(line, sizeof(line), f)) {
        line_no++;
        if (line_no == n) {
            found = 1;
            continue; // preskoči n-to vrstico
        }
        fputs(line, temp);
    }

    fclose(f);
    fclose(temp);

    if (!found) {
        printf("Vrstica %d ne obstaja.\n", n);
        remove(TEMP_FILE);
    } else {
        remove(filename);
        rename(TEMP_FILE, filename);
        printf("Vrstica %d izbrisana.\n", n);
    }
}

/* --- vstavi n-to vrstico --- */
void insert_line(const char *filename, int n) {
    FILE *f = fopen(filename, "r");
    FILE *temp = fopen(TEMP_FILE, "w");

    if (!f || !temp) {
        perror("Napaka pri odpiranju datoteke");
        if (f) fclose(f);
        if (temp) fclose(temp);
        return;
    }

    char line[MAX_LINE];
    int line_no = 0;
    int inserted = 0;

    printf("Vnesi vrstico za vstavljanje:\n");
    char new_line[MAX_LINE];
    if (!fgets(new_line, sizeof(new_line), stdin)) {
        printf("Napaka pri branju vrstice.\n");
        fclose(f);
        fclose(temp);
        remove(TEMP_FILE);
        return;
    }

    while (fgets(line, sizeof(line), f)) {
        line_no++;
        if (line_no == n) {
            fputs(new_line, temp);
            inserted = 1;
        }
        fputs(line, temp);
    }

    /* Če je vstavljanje na konec */
    if (!inserted && n == line_no + 1) {
        fputs(new_line, temp);
        inserted = 1;
    }

    fclose(f);
    fclose(temp);

    if (inserted) {
        remove(filename);
        rename(TEMP_FILE, filename);
        printf("Vrstica vstavljena na mesto %d.\n", n);
    } else {
        printf("Vrstica %d ne obstaja.\n", n);
        remove(TEMP_FILE);
    }
}

