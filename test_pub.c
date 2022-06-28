#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
//#include "/usr/local/include/MQTTClient.h"

#define ADDRESS     "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"  
#define CLIENTID    "gatewayPubSub"
#define TOPIC       "gateway"
#define PAYLOAD     "payload to be replaced!"
#define QOS         1
#define TIMEOUT     10000L

char * cafile = "/etc/gateway/certificates/root-CA.crt";
char * cert = "/etc/gateway/certificates/test_device2.cert.pem";
char * key = "/etc/gateway/certificates/test_device2.private.key";

MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;
int rc;

volatile MQTTClient_deliveryToken deliveredtoken;

void acknowledge()
{
    pubmsg.payload = "msg received successfully";
    pubmsg.payloadlen = (int)strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
        printf("failed to acknowledge, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    // printf("Waiting for up to %d seconds for publication of %s\n"
    //         "on topic %s for client with ClientID: %s\n",
    //         (int)(TIMEOUT/1000), PAYLOAD, TOPIC, CLIENTID);
    //rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("acknowledgement delivered %d delivered\n", token);
    
}


void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    acknowledge();
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
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
        if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to publish message, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }

}

int main(int argc, char* argv[])
{
    

    if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
         printf("Failed to create client, return code %d\n", rc);
         exit(EXIT_FAILURE);
    }

    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    conn_opts.keepAliveInterval = 45;
    conn_opts.cleansession = 1;

    ssl_opts.keyStore = cert;
    ssl_opts.trustStore = cafile;
    ssl_opts.privateKey = key;
    conn_opts.ssl = &ssl_opts;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	rc = EXIT_FAILURE;
    }
    printf("Successfully subscribed to topic %s\n",TOPIC);
    while(1)
    {
        //publish();

        sleep(15);

    }

    if ((rc = MQTTClient_unsubscribe(client, TOPIC)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to unsubscribe, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    	printf("Failed to disconnect, return code %d\n", rc);
    
    destroy_exit:
        MQTTClient_destroy(&client);
    exit:
        return rc;
}