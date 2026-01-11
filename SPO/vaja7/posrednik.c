// posrednik.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x1234
#define MSG_KEY 0x5678

struct message {
    int id;
    char text[100];
};

struct msgbuf {
    long mtype;
    struct message msg;
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
    int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msgid < 0) { perror("msgget"); exit(1); }

    int shmid = shmget(SHM_KEY, sizeof(struct message), 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    int semid = semget(SEM_KEY, 1, 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    struct message* shm = shmat(shmid, NULL, 0);
    if (shm == (void*) -1) { perror("shmat"); exit(1); }

    struct message prev = {-1, ""};

    while (1) {
        sleep(1);

        sem_lock(semid);
        struct message current = *shm;
        sem_unlock(semid);

        if (current.id != prev.id || strcmp(current.text, prev.text) != 0) {
            struct msgbuf buf;
            buf.mtype = 1;
            buf.msg = current;

            msgsnd(msgid, &buf, sizeof(buf.msg), 0);

            printf("[POSREDNIK] Poslal: ID %d | '%s'\n", current.id, current.text);
            prev = current;
        }

        if (current.text[0] == '\0')
            break;
    }

    msgctl(msgid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    shmctl(shmid, IPC_RMID, NULL);
    shmdt(shm);

    return 0;
}

