#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS     "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"
#define CLIENTID    "gateway"
#define PUB_TOPIC    "gateway_pub"
#define SUB_TOPIC1    "gateway_sub1"
#define SUB_TOPIC2    "gateway_sub2"
// char * SUB_TOPIC1  =  "gateway_sub1";
// char * SUB_TOPIC2  =  "gateway_sub2";
#define ACK_TOPIC1   "gateway_ack1"
#define ACK_TOPIC2   "gateway_ack2"
#define PAYLOAD     "payload to be replaced"
#define QOS         1
#define TIMEOUT     10000L

// //flag to indentify who published msg
// int SUB1_MSG = 1;
// int SUB2_MSG = 2;



MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;

int rc;
int msg_arrvd_sub1 = 0; //flag to check msg arrived
int msg_arrvd_sub2 = 0; //flag to check msg arrived

char ack_topic[20];




char * cafile = "/etc/gateway/certificates/root-CA.crt";
char * cert = "/etc/gateway/certificates/test_device2.cert.pem";
char * key = "/etc/gateway/certificates/test_device2.private.key";



volatile MQTTClient_deliveryToken deliveredtoken;

/*flag to identify the subscriber
flag = 1 -> msg from first subscriber
flag = 2 -> msg from second subcriber
*/
void acknowledge(int flag)
{

    char ack_payload[20]={0};
    strcpy();

    if(flag == SUB1_MSG)
    {
        pubmsg.payload = "ack msg received successfully";
        pubmsg.payloadlen = (int)strlen((const char *)(pubmsg.payload));
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        if ((rc = MQTTClient_publishMessage(client, ACK_TOPIC1, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
        {
            printf("failed to acknowledge, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }
        printf("%s acknowledgement ",ACK_TOPIC1);

        //reset the flag   
        msg_arrvd_sub1 = 0;
    }
    else if(flag == SUB2_MSG)
    {
        pubmsg.payload = "ack msg received successfully";
        pubmsg.payloadlen = (int)strlen((const char *)(pubmsg.payload));
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        if ((rc = MQTTClient_publishMessage(client, ACK_TOPIC2, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
        {
            printf("failed to acknowledge, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }
        printf("%s acknowledgement ",ACK_TOPIC2);

        //reset the flag   
        msg_arrvd_sub2 = 0;
    }
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("message delivered successfully\n");
}



int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    // printf("topic = %s  ",topicName);
    // printf("length %d \n",strlen(topicName));

    // printf("topic local = %s  ",SUB_TOPIC1);
    // printf("length %d \n",strlen(SUB_TOPIC1));
    

    if(!strcmp(topicName,SUB_TOPIC1))
    {
        msg_arrvd_sub1 = 1;
    }
    else if(!strcmp(topicName,SUB_TOPIC2))
    {
        msg_arrvd_sub2 = 1;
    }
    
    printf("\nMessage arrived on ");
    printf("topic: %s\n", topicName);
    printf("message: \n%s\n\n",(char *)message->payload);
    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    // printf("msg_arrvd_sub1 = %d\n",msg_arrvd_sub1);
    // printf("msg_arrvd_sub2 = %d\n",msg_arrvd_sub2);

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

void check_msgarrvd_flag()
{
    if(msg_arrvd_sub1 == 1)
    {
        acknowledge(SUB1_MSG);
    }
    else if(msg_arrvd_sub2 == 1)
    {
        acknowledge(SUB2_MSG);
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

    //subscribing to first topic 
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC1, CLIENTID, QOS);

    MQTTClient_subscribe(client, SUB_TOPIC1, QOS);


    //subscribing to the second topic
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC2, CLIENTID, QOS);

    MQTTClient_subscribe(client, SUB_TOPIC2, QOS);


    while(1)
    {
        check_msgarrvd_flag();

        publish();

        sleep(1);

    }


    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}