#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX 90

struct msg_buf{
    /* data */
    long msg_type;
    char msg_text[220];
}msg;

int main(){
    //printf("1\n");
    FILE* ptr;

    key_t msg_key;
    int msgid;

    if ((msg_key = ftok("/etc/systemd/system.conf", 65)) == -1){
        perror("ftok: ");
        exit(1);
    }    

    if ((msgid = msgget(msg_key, 0666 | IPC_CREAT)) == -1){
        perror("msgid: ");
        exit(1);
    }

    msg.msg_type = 1;
    int i = 0;
    strcpy(msg.msg_text,"{\"time\": \"2022-07-01T16:14:09\", \"mac\": \"CD:2C:B8:88:9D:FB\", \"rssi\": -40, \"data\": \"[0, 1, 1, 0, 39, 255, 229, 252, 29, 0, 46, 255, 221, 252, 21, 0, 50, 255, 229, 252, 33, 0, 39, 255, 233, 252, 33]\"}");
    while(1){
        msgsnd(msgid, &msg, sizeof(msg), 0);
        printf("%d Data send is %s \n", i, msg.msg_text);
        i++;
        sleep(1);
    }
    return 0;
}