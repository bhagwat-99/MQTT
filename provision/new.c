#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS                     "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"
#define CLIENTID                    "gateway_G001"
#define PUB_TOPIC_CERT              "$aws/certificates/create/json"
#define PUB_TOPIC_THING             "$aws/provisioning-templates/gatewayTemplate/provision/json"
#define SUB_TOPIC_CERT_ACCEPT       "$aws/certificates/create/json/accepted"
#define SUB_TOPIC_CERT_REJECT       "$aws/certificates/create/json/rejected"

#define SUB_TOPIC_THING_ACCEPT       "$aws/provisioning-templates/gatewayTemplate/provision/json/accepted"
#define SUB_TOPIC_THING_REJECT       "$aws/provisioning-templates/gatewayTemplate/provision/json/rejected"

#define QOS                         1
#define TIMEOUT                     10000L

int rc; //return value variable
int msg_arrvd_flag = 0; //flag to check msg arrived



// //provision certificate path
// char * cafile = "/etc/gateway/provision_certificates/AmazonRootCA1.pem";
// char * cert = "/etc/gateway/provision_certificates/device_certificate.crt";
// char * key = "/etc/gateway/provision_certificates/device_private.key";

//provision certificate path
#define cafile  "/etc/gateway/provision_certificates/AmazonRootCA1.pem";
#define cert  "/etc/gateway/provision_certificates/device_certificate.crt";
#define key  "/etc/gateway/provision_certificates/device_private.key";



//volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("message delivered \n");
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    //printf("enter msgarrvd\n");
    msg_arrvd_flag = 1;
    
    printf("\nMessage arrived on ");
    printf("topic: %s\n", topicName);
    //printf("message: \n%s\n\n",(char *)message->payload);
    //printf("hello\n");
    //printf("payload length %d\n",message->payloadlen);
    //strcpy(pcert_data,message->payload);
    //printf("%s\n",pcert_data);

    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    //printf("exit msgarrvd\n");

    
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}


int main()
{
        MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
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
        printf("connected\n");

        sleep(100);

        return 0;
}