#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX 90

struct mesg_buffer {
    long mesg_type;
    char mesg_text[210];
};
struct mesg_buffer message;

int main(){
    //printf("1\n");
    FILE* ptr;

    key_t key;
    int msgid;
    srand(time(NULL));
    if ((key = ftok("/etc/systemd/system.conf", 65)) == -1){
        perror("ftok: ");
        exit(1);
    }printf("2\n");

    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
        perror("msgid: ");
        exit(1);
    }printf("3\n");

    message.mesg_type = 1;
    printf("4\n");
    int i = 0;
    while(1){
        // ptr = fopen("/proc/sys/kernel/random/uuid", "r");
        // fgets(message.mesg_text, MAX, ptr);
        strcpy(message.mesg_text, "{\"time\": \"2022-07-01T16:14:09\", \"mac\": \"CD:2C:B8:88:9D:FB\", \"rssi\": -40, \"data\": \"[0, 1, 1, 0, 39, 255, 229, 252, 29, 0, 46, 255, 221, 252, 21, 0, 50, 255, 229, 252, 33, 0, 39, 255, 233, 252, 33]\"}");
        msgsnd(msgid, &message, sizeof(message), 0);
        printf("%d Data send is %s \n", i, message.mesg_text);
        i++;
        sleep(1);
    }
    return 0;
}