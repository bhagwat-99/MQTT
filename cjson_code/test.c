#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "cjson/cJSON.h"

int main()
{
        cJSON * jobj = cJSON_CreateObject();
        cJSON * jstring = cJSON_CreateString("hello");
        cJSON_AddStringToObject(jobj,"serial_ID","G001");
        cJSON_AddItemToObject(jobj,"payload",jstring);
        while(1)
        {
                printf("%s\n",cJSON_Print(jobj));
                
                sleep(1);
                cJSON * jstring = cJSON_CreateString("hello2");
                cJSON_ReplaceItemInObject(jobj,"payload",jstring);
                printf("%s\n",cJSON_Print(jobj));
        }
        
        
        
        


        return 0;
}