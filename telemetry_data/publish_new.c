#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "cjson/cJSON.h"
#include "MQTTClient.h"

#define QOS                     1
#define TIMEOUT                 10000L

#define CONF_FILE_PATH "/etc/gateway/gateway.conf"


//certificate path
#define cafile                      "/etc/gateway/certificates/AmazonRootCA1.pem";
#define cert                        "/etc/gateway/certificates/device_certificate.crt";
#define key                         "/etc/gateway/certificates/device_private.key";

typedef struct DEVICE
{

	char *certOwnership_token;
	char *device_ID;
	char *client_ID;

}DEVICE;

typedef struct CLOUD
{
	char *	aws_url;// = "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883";
	char *	pub_topic_telemetry;
	char *	pub_topic_status;
	char *	sub_topic_request;

}CLOUD;

struct msg_buf{
    /* data */
    long msg_type;
    char msg_text[220];
}msg;


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
	//printf("payload length %d\n",message->payloadlen);
	printf("%s\n",(char *)message->payload);
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
        printf("in publish\n");
        if ((rc = MQTTClient_publishMessage(client, publish_topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to publish message, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }
	printf("publish\n");

}



int parse_json_file(DEVICE *device, CLOUD *cloud, char * jsonfile)
{
	cJSON * jcloud = NULL;
	cJSON * jaws_url = NULL;
	cJSON * jclient_ID = NULL;
	cJSON * jpub_topic_telemetry = NULL;
	cJSON * jpub_topic_status = NULL;
	cJSON * jsub_topic_request = NULL;

	cJSON * jdevice = NULL;
	cJSON * jserial_ID = NULL;
	

	cJSON * jgateway = cJSON_Parse(jsonfile);

	if ( jgateway == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
		fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	//storing parsed data into device gateway and cloud structure

	jdevice = cJSON_GetObjectItem(jgateway,"device");
	jserial_ID = cJSON_GetObjectItem(jdevice,"SERIAL_ID");

	device->device_ID = malloc(strlen(cJSON_Print(jserial_ID)));
	if (device->device_ID == NULL) fail();
	strcpy(device->device_ID,jserial_ID->valuestring);
	//device->device_ID = jserial_ID->valuestring;
	//printf("%s\n",jserial_ID->valuestring);

	jclient_ID = cJSON_GetObjectItem(jdevice,"NAME");

	device->client_ID = malloc(strlen(cJSON_Print(jclient_ID)));
	if (device->client_ID == NULL) fail();
	strcpy(device->client_ID,jclient_ID->valuestring);
	//device->client_ID = jclient_ID->valuestring;


	jcloud 	= cJSON_GetObjectItem(jgateway,"cloud");

	jaws_url = cJSON_GetObjectItem(jcloud,"HOST");
	cloud->aws_url = malloc(strlen(cJSON_Print(jaws_url)));
	if (cloud->aws_url == NULL) fail();
	strcpy(cloud->aws_url,jaws_url->valuestring);
	//cloud->aws_url = jaws_url->valuestring;

	jpub_topic_telemetry = cJSON_GetObjectItem(jcloud,"TELEMETRY_PUBLISH_TOPIC");
	cloud->pub_topic_telemetry = malloc(strlen(cJSON_Print(jpub_topic_telemetry)));
	if(cloud->pub_topic_telemetry == NULL) fail();
	strcpy(cloud->pub_topic_telemetry,jpub_topic_telemetry->valuestring);
	//cloud->pub_topic_telemetry = jpub_topic_telemetry->valuestring;


	jpub_topic_status = cJSON_GetObjectItem(jcloud,"STATUS_PUBLISH_TOPIC");
	cloud->pub_topic_status = malloc(strlen(cJSON_Print(jpub_topic_status)));
	if(cloud->pub_topic_status == NULL) fail();
	strcpy(cloud->pub_topic_status,jpub_topic_status->valuestring);
	//cloud->pub_topic_status = jpub_topic_status->valuestring;

	jsub_topic_request = cJSON_GetObjectItemCaseSensitive(jcloud,"JOB_TOPIC");
	cloud->sub_topic_request = malloc(strlen(cJSON_Print(jsub_topic_request)));
	if(cloud->sub_topic_request == NULL) fail();
	strcpy(cloud->sub_topic_request,jsub_topic_request->valuestring);
	//cloud->sub_topic_request = jsub_topic_request->valuestring;

	end:
		cJSON_Delete(jgateway);

		return 0;


}

char * read_config_file(char * filename,char * mode,char * dest_buf)
{
	FILE * fd;
	fd = fopen(filename,mode);
	if(fd == NULL)
	{
		printf("error opening file : /etc/gateway/gateway.conf");
		exit(1);
	}
	fread(dest_buf,4000,1,fd);
	fclose(fd);
	return dest_buf;

}



int main(int argc, char* argv[])
{
	char * data_buf = NULL;
	data_buf = malloc(4000*(sizeof(char)));

	DEVICE gateway_device;
	CLOUD gateway_cloud;

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

	data_buf = read_config_file(CONF_FILE_PATH,"r",data_buf);

	//printf("%s\n",data_buf);

	parse_json_file(&gateway_device,&gateway_cloud,data_buf);

	free(data_buf);

	MQTTClient_create(&client, gateway_cloud.aws_url, gateway_device.client_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);


	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("connected\n");

        cJSON * jobj = NULL;
        cJSON * jpayload = NULL;
        cJSON * jstring = NULL;

        jobj = cJSON_CreateObject();
        jpayload = cJSON_CreateArray();
        cJSON_AddItemToObject(jobj,"payloads",jpayload);

        FILE* ptr;

        key_t msg_key;
        int msgid;

        if ((msg_key = ftok("/etc/systemd/system.conf", 65)) == -1){
        perror("ftok: ");
        exit(1);
        }    

        if ((msgid = msgget(msg_key, 0666 | IPC_CREAT)) == -1){
        perror("msgid: ");
        exit(1);
        }

        
	//printf("%s\n",jpayload->valuestring);

	strcat(gateway_cloud.pub_topic_telemetry,gateway_device.device_ID);
	strcat(gateway_cloud.pub_topic_status,gateway_device.device_ID);

	// printf("status : %s\n",gateway_cloud.pub_topic_status);
	// printf("telemetry : %s\n",gateway_cloud.pub_topic_telemetry);
	//printf("jpayload : %s\n",cJSON_Print(jpayload));
	//strcpy(data_buf,jpayload->valuestring);
	// printf("%s\n",data_buf);
	
	while(1)
	{
		for(int i =0 ; i < 10; i++)
		{
			msgrcv(msgid, &msg, sizeof(msg), 1, 0);
			jstring = cJSON_CreateString(msg.msg_text);
			cJSON_AddItemToArray(jpayload,jstring);

		}
		publish(client, pubmsg , token, gateway_cloud.pub_topic_telemetry,cJSON_Print(jpayload));
	}

	end:
	msgctl(msgid, IPC_RMID, NULL);
	cJSON_Delete(jobj);
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
	return 0;
}
