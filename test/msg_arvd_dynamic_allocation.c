

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "cjson/cJSON.h"
#include "MQTTClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "cjson/cJSON.h"
#include "MQTTClient.h"

#define QOS                     0
#define TIMEOUT                 10000L
#define SUB_TOPIC               "gateway_testsub"
#define PUB_TOPIC               "gateway_testpub"


//certificate path
#define cafile                  "/test_certificates/AmazonRootCA1.pem";
#define cert                    "/test_certificates/device_certificate.crt";
#define key                     "/test_certificates/device_private.key";
#define aws_url                 "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"     


int rc;                             //return value variable
int msg_arrvd_flag = 0;             //flag to check msg arrived
//char * data_buf= NULL;

void fail(void) {printf("error allocating memory \n"); exit(EXIT_FAILURE);}


void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("message delivered\n");
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{

	printf("Message arrived on ");
	printf("topic: %s\n", topicName);
	printf("payload length %d\n",message->payloadlen);
	//printf("%s\n",(char *)message->payload);
	//data_buf = message->payload;
	//printf("%s\n",data_buf);


        MQTTClient_freeMessage(&message);
        MQTTClient_free(topicName);
	msg_arrvd_flag = 1;

	return 1;
}


void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void publish(MQTTClient client,MQTTClient_message pubmsg,MQTTClient_deliveryToken token,char * publish_topic,char * payload)
{
        pubmsg.payload = payload;
        pubmsg.payloadlen = (int)strlen(payload);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        //printf("in publish\n");
        if ((rc = MQTTClient_publishMessage(client, publish_topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to publish message, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }
	printf("publish ");

}

int main(int argc, char* argv[])
{
	char * data_buf = NULL;
	data_buf = malloc(4000*(sizeof(char)));


	// //volatile MQTTClient_deliveryToken deliveredtoken;
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;

	//connect options
	ssl_opts.keyStore = cert;
	ssl_opts.trustStore = cafile;
	ssl_opts.privateKey = key;

	conn_opts.keepAliveInterval = 45;
	conn_opts.cleansession = 1;
	conn_opts.ssl = &ssl_opts;



	MQTTClient_create(&client, aws_url, "gateway_device", MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

        


	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("connected\n");


	//subscribing to SUB_TOPIC_THING_ACCEPT 
	if ((rc = MQTTClient_subscribe(client, SUB_TOPIC , QOS)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to subscribe, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("Subscribed to topic %s for client %s using QoS%d \n", SUB_TOPIC , "gateway_device", QOS);





	while(1)
	{
                if(msg_arrvd_flag)
                {
                        printf("%s\n",(char *)pubmsg.payload);
                        msg_arrvd_flag = 0;
                }

                //publish(client, pubmsg , token, PUB_TOPIC,"test payload");
                sleep(5);
	}

	end:
                MQTTClient_disconnect(client, 10000);
                MQTTClient_destroy(&client);
                return 0;
}
