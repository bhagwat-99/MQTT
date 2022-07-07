#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS                 "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"
#define CLIENTID                "gateway_client"
#define PUB_TOPIC               "gateway_heartbeat"
#define MAX_PAYLOAD_LENGTH      100
#define QOS                     0
#define TIMEOUT                 10000L
#define conf_file_path          "/etc/gateway/serial_ID"


//certificate path
char * cafile = "/etc/gateway/certificates/root-CA.crt";
char * cert = "/etc/gateway/certificates/test_device2.cert.pem";
char * key = "/etc/gateway/certificates/test_device2.private.key";

//volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;

int rc;                 //return value variable

char * payload;         //mqtt payload

char serial_ID[20];     //char array to store serial ID of device

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("message delivered\n");
    //fflush(stdout);
}


void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

//fuction to publish heartbeat to aws
void publish()
{
        //printf("in publish\n");
        //fflush(stdout);
        pubmsg.payload = payload;
        pubmsg.payloadlen = (int)strlen(payload);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        if ((rc = MQTTClient_publishMessage(client, PUB_TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to publish message, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }
        //printf("publish ");

}

int read_serial_ID()
{
        //printf("in read_serial_ID\n");
        int fd;
        fd = open(conf_file_path,O_RDWR);
        if(fd < 0)
        {
                printf("Failed to open %s\n.",conf_file_path);
                exit(1);
        }

        rc = read(fd,serial_ID,10);
        if(rc < 0)
        {
                printf("Failed to open %s\n.",conf_file_path);
                exit(1);
        }
        close(fd);
        return 0;
}

int main(int argc, char* argv[])
{
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, NULL, delivered);

    ssl_opts.keyStore = cert;
    ssl_opts.trustStore = cafile;
    ssl_opts.privateKey = key;

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.ssl = &ssl_opts;


    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("connected\n");

    //read serial_ID of device
    read_serial_ID();

    //allocating memory for payload
    payload = malloc(MAX_PAYLOAD_LENGTH); 
    if(payload == NULL)
    {
        printf("Error allocating memory for payload\n");
        exit(1);
    }
    payload[0] = '\0';   // ensures the memory is an empty string
    strcpy(payload,"initilizing the payload");

    //boot payload
    strcpy(payload,"boot : ");
    strcat(payload,serial_ID);
    //publish boot msh
    publish();

    //heartbeat payload
    strcpy(payload,"heartbeat : ");
    strcat(payload,serial_ID);

    while(1)
    {
        
        publish();

        sleep(300);

    }

    
    
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
