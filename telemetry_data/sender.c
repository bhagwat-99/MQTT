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
    char msg_text[230];
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
    strcpy(msg.msg_text,"{\"time\": \"2022-07-19T17:13:21\", \"mac\": \"F4:45:C4:41:7A:86\", \"rssi\": -70, \"pr\": 150, \"pt\": \"temp\", \"data\": \"[2, 1, 23.05, 100]\"}");
    
    while(1){
        msgsnd(msgid, &msg, sizeof(msg), 0);
        printf("%d Data send is %s \n", i, msg.msg_text);
        i++;
        sleep(1);
    }
    return 0;
}