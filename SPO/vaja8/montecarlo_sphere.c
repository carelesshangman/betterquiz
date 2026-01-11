// montecarlo_sphere.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define P_LOCAL 1000000LL     // poskusov na nit v enem ciklu
#define PS_MAX 1000000000LL   // skupno poskusov (1e9)

long long Zs = 0;   // globalni zadetki
long long Ps = 0;   // globalni poskusi

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

double rand01() {
    return rand() / (double)RAND_MAX;
}

void* worker(void* arg) {
    while (1) {
        long long Z_local = 0;

        // 1) Lokalni Monte Carlo poskusi
        for (long long i = 0; i < P_LOCAL; i++) {
            double x = rand01();
            double y = rand01();
            double z = rand01();

            double r2 = x*x + y*y + z*z;
            if (r2 <= 1.0)
                Z_local++;
        }

        // 2) Dodaj v globalne spremenljivke (z zaklepanjem)
        pthread_mutex_lock(&mtx);
        Zs += Z_local;
        Ps += P_LOCAL;

        int done = (Ps >= PS_MAX);
        pthread_mutex_unlock(&mtx);

        if (done) break;
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uporaba: %s <stevilo_niti>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    if (N <= 0) {
        printf("Napacno stevilo niti.\n");
        return 1;
    }

    srand(time(NULL));

    pthread_t* threads = malloc(sizeof(pthread_t) * N);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Ustvari niti
    for (int i = 0; i < N; i++)
        pthread_create(&threads[i], NULL, worker, NULL);

    // Pocakaj niti
    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) * 1e-9;

    // Izracun prostornine
    // Delamo le 1/8 enotske krogle (r med 0..1 v 3D prostornini kocke 1x1x1)
    // Zato množimo z 8.
    double approx_volume = 8.0 * ((double)Zs / (double)Ps);

    printf("--------------------------------------------------\n");
    printf("Stevilo niti:         %d\n", N);
    printf("Skupni poskusi:       %lld\n", Ps);
    printf("Zadetki:              %lld\n", Zs);
    printf("Priblizek volumna:    %.10f\n", approx_volume);
    printf("Napaka:               %.10f\n",
           fabs((4.0/3.0)*M_PI - approx_volume));
    printf("Cas izvajanja:        %.6f s\n", elapsed);
    printf("--------------------------------------------------\n");

    free(threads);
    return 0;
}

