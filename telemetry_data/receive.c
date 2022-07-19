// C Program for Message Queue (Reader Process)
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cjson/cJSON.h"
  
struct msg_buf{
    /* data */
    long msg_type;
    char msg_text[220];
}msg;

int main()
{
        cJSON * jobj = NULL;
        cJSON * jpayload = NULL;
        cJSON * jstring = NULL;

        jobj = cJSON_CreateObject();
        jpayload = cJSON_CreateArray();
        cJSON_AddItemToObject(jobj,"payloads",jpayload);
        
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

        for(int i =0 ; i <  20; i++)
        {
                msgrcv(msgid, &msg, sizeof(msg), 1, 0);
                jstring = cJSON_CreateString(msg.msg_text);
                cJSON_AddItemToArray(jpayload,jstring);

        }

        printf("%s\n",cJSON_Print(jobj));
        end:
        // to destroy the message queue
        msgctl(msgid, IPC_RMID, NULL);
        cJSON_Delete(jobj);

        return 0;
}