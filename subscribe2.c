#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS     "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"
#define CLIENTID    "gatewaySub"
#define PUB_TOPIC    "gateway_pub"
#define SUB_TOPIC    "gateway_sub"
#define ACK_TOPIC   "gateway_ack"
#define PAYLOAD     "payload to be replaced!"
#define QOS         1
#define TIMEOUT     10000L

MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;

int rc;
int msg_arrvd = 0; //flag to check msg arrived

char * cafile = "/etc/gateway/certificates/root-CA.crt";
char * cert = "/etc/gateway/certificates/test_device2.cert.pem";
char * key = "/etc/gateway/certificates/test_device2.private.key";


volatile MQTTClient_deliveryToken deliveredtoken;


void acknowledge()
{
    pubmsg.payload = "{msg received successfully}";
    pubmsg.payloadlen = (int)strlen((const char *)(pubmsg.payload));
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    if ((rc = MQTTClient_publishMessage(client, ACK_TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
        printf("failed to acknowledge, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    //reset the flag   
    msg_arrvd = 0;
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message delivered\n");
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    msg_arrvd = 1;
    return 1;
}
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void publish()
{
        pubmsg.payload = PAYLOAD;
        pubmsg.payloadlen = (int)strlen(PAYLOAD);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        if ((rc = MQTTClient_publishMessage(client, PUB_TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to publish message, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }

}


int main(int argc, char* argv[])
{

    
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

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
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC, CLIENTID, QOS);

    MQTTClient_subscribe(client, SUB_TOPIC, QOS);

    while(1)
    {
        if(msg_arrvd)
        {
                acknowledge();
        }

        publish();

        sleep(5);

    }


    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
