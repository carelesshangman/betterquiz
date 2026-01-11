// generator.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x1234

struct message {
    int id;
    char text[100];
};

void sem_lock(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_unlock(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(struct message), 0666 | IPC_CREAT);
    if (shmid < 0) { perror("shmget"); exit(1); }

    int semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
    if (semid < 0) { perror("semget"); exit(1); }

    semctl(semid, 0, SETVAL, 1);

    struct message* shm = shmat(shmid, NULL, 0);
    if (shm == (void*) -1) { perror("shmat"); exit(1); }

    for (int i = 1; i <= 5; i++) {
        sleep(2); // 2 seconds outside semaphore

        sem_lock(semid);

        if (i == 5) {
            shm->id = 5;
            shm->text[0] = '\0'; // empty message
        } else {
            shm->id = i;
            snprintf(shm->text, sizeof(shm->text), "To je sporocilo.");
        }

        printf("[GENERATOR] Zapisal: ID %d | '%s'\n", shm->id, shm->text);

        sleep(2); // 2 seconds inside semaphore
        sem_unlock(semid);
    }

    shmdt(shm);
    return 0;
}

