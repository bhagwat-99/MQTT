#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
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

typedef struct
{

	char *certOwnership_token;
	char *device_ID;
	char *client_ID;

}DEVICE;

#define DEVICE_initializer {NULL,NULL,NULL};

typedef struct 
{
	char *	aws_url;// = "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883";
	char *	pub_topic_telemetry;
	char *	pub_topic_status;
	char *	sub_topic_request;

}CLOUD;

#define CLOUD_initializer {"ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883","gateway/telemetry_data/","gateway/status/","gataway/request/"}


typedef struct
{
        char * serial_ID; //serial id of device eg G001
        char * timestamp; 
        char * type; //type of payload eg boot/status
        char * data; //send requested data from device
        char * location; // gps location

}PAYLOAD;

#define PAYLOAD_initializer {NULL,NULL,NULL,NULL,NULL}

int rc;
int msg_arrvd_flag = 0;


void fail(void) {printf("error allocating memory \n"); exit(EXIT_FAILURE);}

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

void delivered(void *context, MQTTClient_deliveryToken dt)
{
        printf("message delivered\n");
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
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
	cloud->pub_topic_telemetry = malloc(strlen(cJSON_Print(jpub_topic_telemetry))+10);// +10 for concatinating device id to basic topic
	if(cloud->pub_topic_telemetry == NULL) fail();
	strcpy(cloud->pub_topic_telemetry,jpub_topic_telemetry->valuestring);
	//cloud->pub_topic_telemetry = jpub_topic_telemetry->valuestring;


	jpub_topic_status = cJSON_GetObjectItem(jcloud,"STATUS_PUBLISH_TOPIC");
	cloud->pub_topic_status = malloc(strlen(cJSON_Print(jpub_topic_status))+10);// +10 for concatinating device id to basic topic
	if(cloud->pub_topic_status == NULL) fail();
	strcpy(cloud->pub_topic_status,jpub_topic_status->valuestring);
	//cloud->pub_topic_status = jpub_topic_status->valuestring;

	jsub_topic_request = cJSON_GetObjectItemCaseSensitive(jcloud,"JOB_TOPIC");
	cloud->sub_topic_request = malloc(strlen(cJSON_Print(jsub_topic_request))+10);// +10 for concatinating device id to basic topic
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



char * create_payload(PAYLOAD *payload, char * buffer)
{

	/*Creating a json object*/
	cJSON * jobj = cJSON_CreateObject();
	
	/*Creating a json string*/
	cJSON * jserial_ID = cJSON_CreateString(payload->serial_ID);
	cJSON_AddItemToObject(jobj,"serial_ID", jserial_ID);

	cJSON * jtimestamp = cJSON_CreateString(payload->timestamp);
	cJSON_AddItemToObject(jobj, "timestamp", jtimestamp);

	cJSON * jtype = cJSON_CreateString(payload->type);
	cJSON_AddItemToObject(jobj,"type", jtype);

        cJSON * jdata = cJSON_CreateString(payload->data);
        cJSON_AddItemToObject(jobj,"data", jdata);
        
        cJSON * jlocation = cJSON_CreateString(payload->location);
        cJSON_AddItemToObject(jobj,"jlocation", jlocation);

	strcpy(buffer,cJSON_Print(jobj));

	cJSON_Delete(jobj);
	

	return buffer;
}


int main()
{

        char * data_buf = NULL;
	data_buf = malloc(4000*(sizeof(char)));

        DEVICE gateway_device = DEVICE_initializer;
	CLOUD gateway_cloud = CLOUD_initializer;
        PAYLOAD gateway_payload = PAYLOAD_initializer;

        //volatile MQTTClient_deliveryToken deliveredtoken;
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


        read_config_file(CONF_FILE_PATH,"r",data_buf);

        parse_json_file(&gateway_device,&gateway_cloud,data_buf);

        strcat(gateway_cloud.pub_topic_telemetry,gateway_device.device_ID);
	strcat(gateway_cloud.pub_topic_status,gateway_device.device_ID);


	MQTTClient_create(&client, gateway_cloud.aws_url, gateway_device.client_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);


	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("connected\n");


        //printf("serial_ID: %s\n",gateway_device.device_ID);
        gateway_payload.serial_ID = malloc(strlen(gateway_device.device_ID)+1);
        if(gateway_payload.serial_ID == NULL) fail();
        strcpy(gateway_payload.serial_ID,gateway_device.device_ID);

        //location of device
        gateway_payload.location = "NULL";
	gateway_payload.data = "NULL";

        //payload for boot
	gateway_payload.type = malloc(10);
	if(gateway_payload.type == NULL) fail();
        strcpy(gateway_payload.type,"boot");
	printf("%s\n",gateway_payload.type);
	//while(1);
        create_payload(&gateway_payload,data_buf);
        printf("%s\n",data_buf);
        publish(client, pubmsg , token, gateway_cloud.pub_topic_telemetry,data_buf);

        //payload to status
        strcpy(gateway_payload.type,"status");
        create_payload(&gateway_payload,data_buf);

        while(1)
        {
                //printf("%s\n",data_buf);
                publish(client, pubmsg , token, gateway_cloud.pub_topic_status,data_buf);

                sleep(5);
        }

        free(gateway_payload.serial_ID);
	free(gateway_payload.type);
        free(data_buf);
        return 0;
}