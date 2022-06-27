#include <stdio.h>

char * cert_path = "/home/bhagwat/bhagwatws/mqtt/certificates/";

char * cert; char * key; char * cafile;
cert = malloc(strlen(cert_path) + strlen("test_device2.cert.pem") + 1);
if(cert == NULL)
{
        printf("Error allocating memory for cert\n");
        exit(1);
}
cert[0] = '\0';  // ensures the memory is an empty string
strcat(cert,cert_path);
strcat(cert,);

key = malloc(strlen(cert_path) + strlen("test_device2.private.key") + 1);
if(key == NULL)
{
        printf("Error allocating memory for key\n");
        exit(1);
}

cafile = malloc(strlen(cert_path) + strlen("root-CA.crt") + 1);
if(cafile == NULL)
{
        printf("Error allocating memory for cafile\n");
        exit(1);
}




int main()
{
        printf("hello world");
        return 0;

}
