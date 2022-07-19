// C Program for Message Queue (Reader Process)
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cjson/cJSON.h"
char *create_monitor(void)
{
    const unsigned int resolution_numbers[3][2] = {
        {1280, 720},
        {1920, 1080},
        {3840, 2160}
    };
    char *string = NULL;
//     cJSON *name = NULL;
    cJSON *jpayloads = NULL;
    cJSON *jpayload = NULL;
    cJSON *width = NULL;
    cJSON *height = NULL;
    size_t index = 0;

    cJSON *jobj = cJSON_CreateObject();
    if (jobj == NULL)
    {
        goto end;
    }

//     name = cJSON_CreateString("Awesome 4K");
//     if (name == NULL)
//     {
//         goto end;
//     }
//     /* after creation was successful, immediately add it to the monitor,
//      * thereby transferring ownership of the pointer to it */
//     cJSON_AddItemToObject(monitor, "name", name);

    jpayloads = cJSON_CreateArray();
    if (jpayloads == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(jobj, "payloads", jpayloads);

    for (index = 0; index < 2 ; ++index)
    {
        jpayload = cJSON_CreateObject();
        if (jpayload == NULL)
        {
            goto end;
        }
        cJSON_AddItemToArray(jpayloads, jpayload);

        width = cJSON_CreateString("hello");
        if (width == NULL)
        {
            goto end;
        }
        cJSON_AddItemToObject(jpayload, "width", width);

        height = cJSON_CreateNumber(resolution_numbers[index][1]);
        if (height == NULL)
        {
            goto end;
        }
        cJSON_AddItemToObject(jpayload, "height", height);
    }

    string = cJSON_Print(jobj);
    if (string == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");
    }

end:
    cJSON_Delete(jobj);
    return string;
}

int my_func()
{
        cJSON * jobj = NULL;
        cJSON * jpayload = NULL;
        cJSON * jstring = NULL;

        jobj = cJSON_CreateObject();
        jpayload = cJSON_CreateArray();
        cJSON_AddItemToObject(jobj,"payloads",jpayload);

        for(int i =0 ; i <  3; i++)
        {
                jstring = cJSON_CreateString("hello");
                cJSON_AddItemToArray(jpayload,jstring);

        }

        printf("%s\n",cJSON_Print(jobj));

        end:
                cJSON_Delete(jobj);
                return 0;
}

int main()
{
        my_func();

        return 0;

}