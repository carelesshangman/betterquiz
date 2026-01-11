// zapisovalec.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_KEY 0x5678

struct message {
    int id;
    char text[100];
};

struct msgbuf {
    long mtype;
    struct message msg;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Uporaba: %s <izhodna_datoteka>\n", argv[0]);
        exit(1);
    }

    FILE* out = fopen(argv[1], "w");
    if (!out) { perror("fopen"); exit(1); }

    int msgid = msgget(MSG_KEY, 0666);
    if (msgid < 0) { perror("msgget"); exit(1); }

    struct msgbuf buf;

    while (1) {
        msgrcv(msgid, &buf, sizeof(buf.msg), 1, 0);

        fprintf(out, "ID %d: %s\n", buf.msg.id, buf.msg.text);
        printf("[ZAPISOVALEC] Prejel: ID %d | '%s'\n",
               buf.msg.id, buf.msg.text);

        if (buf.msg.text[0] == '\0')
            break;
    }

    fclose(out);
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}

