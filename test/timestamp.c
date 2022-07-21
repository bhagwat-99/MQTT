#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char * buf;

void fail()
{
        printf("failed to allocate memory\n");
        exit(1);
}

char * date_time_fuc(char * time_buf)
{
        time_t now;
        struct tm ts;
        // char time_buf[30];
        // char time_str[30];
        time ( &now );
        ts = *localtime ( &now );

        // time format to save in csv file
        strftime(time_buf, 30, "%a %Y-%m-%d %H:%M:%S %Z", &ts);

        //printf("%s\n",time_buf);
        return time_buf;
        
}

int main()
{
        buf = malloc(30*sizeof(char));
        if (buf == NULL) fail();

        //char time_str[30];

        //printf("%ld",sizeof(time_str));

        date_time_fuc(buf);
        printf("%s\n",buf);
        
}