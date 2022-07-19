#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS                 "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"
#define CLIENTID                "gateway_G001"
#define PUB_TOPIC1               "gateway/telemetry_data/G001"
#define PUB_TOPIC2               "gateway/status/G001"
#define SUB_TOPIC              "gateway/request/G001"
//#define SUB_TOPIC2              "gateway_sub2"
#define SUB_TOPIC_MAX_LENGTH    30
//#define ACK_TOPIC               "gateway_ack"
#define PAYLOAD                 "payload to be replaced!"
#define QOS                     1
#define TIMEOUT                 10000L

//certificate path
#define cafile                      "/etc/gateway/certificates/AmazonRootCA1.pem";
#define cert                        "/etc/gateway/certificates/device_certificate.crt";
#define key                         "/etc/gateway/certificates/device_private.key";


char * data_buf = NULL;

int rc; //return value variable
int msg_arrvd_flag = 0; //flag to check msg arrived


void fail(void) {printf("error allocating memory \n"); exit(EXIT_FAILURE);}


void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("message delivered\n");
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
	printf("publish\n");

}


int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{

	printf("Message arrived on ");
	printf("topic: %s\n", topicName);
	//printf("payload length %d\n",message->payloadlen);
	strcpy(data_buf,message->payload);
	//data_buf = message->payload;
	printf("%s\n",data_buf);


	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	msg_arrvd_flag = 1;

	return 1;
}


int main(int argc, char* argv[])
{

    data_buf = malloc(4000 * sizeof(char));
	if (data_buf == NULL) fail();

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



	//MQTTClient_create(&client, gateway_cloud.aws_url, gateway_device.client_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	MQTTClient_create(&client, ADDRESS,CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	//printf("create\n");
	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	//printf("trying to connect\n");

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
	printf("Subscribed to topic %s for client %s using QoS%d \n", SUB_TOPIC , CLIENTID, QOS);

    



    while(1)
    {
        publish(client,pubmsg,token,PUB_TOPIC1,PAYLOAD);

        publish(client,pubmsg,token,PUB_TOPIC2,PAYLOAD);

        sleep(30);

    }




    free(data_buf);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return 0;
}