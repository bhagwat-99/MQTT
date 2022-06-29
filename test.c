#include <stdio.h>
#include <string.h>
#define SUB_TOPIC1    "gateway_sub1"

char * topic = "gateway_sub1";

int main()
{
        printf("sub topic = %s\n",SUB_TOPIC1);
        printf("length = %ld\n",strlen(SUB_TOPIC1));

        printf("topic = %s\n",topic);
        printf("topic = %ld\n",strlen(topic));

        if(SUB_TOPIC1 == topic)
        {
                printf("in if\n");
        }

        return 0;
}