#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS                 "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"
#define CLIENTID                "gateway"
#define PUB_TOPIC               "gateway_pub"
#define SUB_TOPIC1              "gateway_sub1"
#define SUB_TOPIC2              "gateway_sub2"
#define SUB_TOPIC_MAX_LENGTH    30
#define ACK_TOPIC               "gateway_ack"
#define PAYLOAD                 "payload to be replaced!"
#define QOS                     1
#define TIMEOUT                 10000L


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


int rc; //return value variable
int msg_arrvd = 0; //flag to check msg arrived

char * ack_topic;
char * ack_payload;
//char * payload;


int acknowledge()
{
    strcpy(ack_payload,"msg received successfully from topic : ");
    strcat(ack_payload,ack_topic);

    //reset the flag   
    msg_arrvd = 0;


    pubmsg.payload = ack_payload;
    pubmsg.payloadlen = (int)strlen((const char *)(pubmsg.payload));
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    if ((rc = MQTTClient_publishMessage(client, ACK_TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
        printf("failed to acknowledge, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("ack ");

    
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("message delivered\n");
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    msg_arrvd = 1;
    strcpy(ack_topic,topicName);
    
    printf("\nMessage arrived on ");
    printf("topic: %s\n", topicName);
    printf("message: \n%s\n\n",(char *)message->payload);
    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
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
        printf("publish ");

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

    //subscribing to first topic 
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC1, CLIENTID, QOS);
    MQTTClient_subscribe(client, SUB_TOPIC1, QOS);


    //subscribing to the second topic
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC2, CLIENTID, QOS);
    MQTTClient_subscribe(client, SUB_TOPIC2, QOS);


    //allocating memory for ack_topic
    ack_topic = malloc(SUB_TOPIC_MAX_LENGTH); 
    if(ack_topic == NULL)
    {
        printf("Error allocating memory for ack_topic\n");
        exit(1);
    }
    ack_topic[0] = '\0';   // ensures the memory is an empty string

    //allocating memory for ack_payload
    ack_payload = malloc(strlen("msg received successfully from topic : ") + SUB_TOPIC_MAX_LENGTH + 1);
    if(ack_payload == NULL)
    {
        printf("Error allocating memory for ack_payload\n");
        exit(1);
    }
    ack_payload[0] = '\0';   // ensures the memory is an empty string

    while(1)
    {
        if(msg_arrvd)
        {
                acknowledge();
        }

       publish();

       sleep(1);

    }


    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
