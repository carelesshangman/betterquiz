#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    FILE *f1, *f2;
    char buffer1[10];
    char buffer2[100];
    const char *stavek = "To je moj najljubši stavek.\n";
    long i;
    time_t start, end;
    double cas1, cas2;

    /* --- Prva datoteka: izhod1 (buffer = 10 B) --- */
    f1 = fopen("izhod1.txt", "w");
    if (f1 == NULL) {
        perror("Napaka pri odpiranju datoteke izhod1.txt");
        return 1;
    }

    if (setvbuf(f1, buffer1, _IOFBF, sizeof(buffer1)) != 0) {
        perror("Napaka pri nastavitvi medpomnilnika za izhod1.txt");
        fclose(f1);
        return 1;
    }

    printf("Zacetek pisanja v izhod1.txt (buffer 10 B)...\n");
    start = time(NULL);
    for (i = 0; i < 5000000; i++) {
        if (fputs(stavek, f1) == EOF) {
            perror("Napaka pri pisanju v izhod1.txt");
            fclose(f1);
            return 1;
        }
    }
    fflush(f1);
    end = time(NULL);
    cas1 = difftime(end, start);
    fclose(f1);
    printf("Cas pisanja v izhod1.txt: %.2f sekund.\n", cas1);

    /* --- Druga datoteka: izhod2 (buffer = 100 B) --- */
    f2 = fopen("izhod2.txt", "w");
    if (f2 == NULL) {
        perror("Napaka pri odpiranju datoteke izhod2.txt");
        return 1;
    }

    if (setvbuf(f2, buffer2, _IOFBF, sizeof(buffer2)) != 0) {
        perror("Napaka pri nastavitvi medpomnilnika za izhod2.txt");
        fclose(f2);
        return 1;
    }

    printf("Zacetek pisanja v izhod2.txt (buffer 100 B)...\n");
    start = time(NULL);
    for (i = 0; i < 5000000; i++) {
        if (fputs(stavek, f2) == EOF) {
            perror("Napaka pri pisanju v izhod2.txt");
            fclose(f2);
            return 1;
        }
    }
    fflush(f2);
    end = time(NULL);
    cas2 = difftime(end, start);
    fclose(f2);
    printf("Cas pisanja v izhod2.txt: %.2f sekund.\n", cas2);

    return 0;
}

