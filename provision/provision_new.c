#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "cjson/cJSON.h"
#include "MQTTClient.h"

#define PUB_TOPIC_CERT              "$aws/certificates/create/json"
#define PUB_TOPIC_THING             "$aws/provisioning-templates/gatewayTemplate/provision/json"
#define SUB_TOPIC_CERT_ACCEPT       "$aws/certificates/create/json/accepted"
#define SUB_TOPIC_CERT_REJECT       "$aws/certificates/create/json/rejected"
#define SUB_TOPIC_THING_ACCEPT      "$aws/provisioning-templates/gatewayTemplate/provision/json/accepted"
#define SUB_TOPIC_THING_REJECT      "$aws/provisioning-templates/gatewayTemplate/provision/json/rejected"

#define QOS                         1
#define TIMEOUT                     10000L

#define CONF_FILE_PATH "/etc/gateway/gateway.conf"

//provision certificate path
#define cafile                      "/etc/gateway/provision_certificates/AmazonRootCA1.pem";
#define cert                        "/etc/gateway/provision_certificates/device_certificate.crt";
#define key                         "/etc/gateway/provision_certificates/device_private.key";


int rc;                             //return value variable
int msg_arrvd_flag = 0;             //flag to check msg arrived

char * data_buf;


typedef struct DEVICE
{

	char *certOwnership_token;
	char *device_ID;
	char *client_ID ;

}DEVICE;

typedef struct CLOUD
{
	char *	aws_url;
	char *	pub_topic_telemetry;
	char *	pub_topic_status;
	char *	sub_topic_request;

}CLOUD;


void fail(void) {printf("error allocating memory \n"); exit(EXIT_FAILURE);}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
	printf("message delivered \n");
}

void connlost(void *context, char *cause)
{
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}


int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	msg_arrvd_flag = 1;

	printf("Message arrived on ");
	printf("topic: %s\n", topicName);
	//printf("payload length %d\n",message->payloadlen);
	//strcpy(data_buf,message->payload);
	//printf("%s\n",pcert_data);


	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);


	return 1;
}

int read_config_file(char * filename,char * mode)
{
	FILE * fd;
	fd = fopen(filename,mode);
	if(fd == NULL)
	{
		printf("error opening file : /etc/gateway/gateway.conf");
		exit(1);
	}
	fread(data_buf,4000,1,fd);
	fclose(fd);
	return 0;

}


int parse_json_file(DEVICE *device, CLOUD *cloud)
{
	cJSON * jcloud = NULL;
	cJSON * jaws_url = NULL;
	cJSON * jclient_ID = NULL;
	cJSON * jpub_topic_telemetry = NULL;
	cJSON * jpub_topic_status = NULL;
	cJSON * jsub_topic_request = NULL;

	cJSON * jdevice = NULL;
	cJSON * jserial_ID = NULL;
	

	cJSON * jgateway = cJSON_Parse(data_buf);

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

	jdevice = cJSON_GetObjectItemCaseSensitive(jgateway,"device");
	jserial_ID = cJSON_GetObjectItemCaseSensitive(jdevice,"SERIAL_ID");

	device->device_ID = malloc(strlen(cJSON_Print(jserial_ID)));
	if (device->device_ID == NULL) fail();
	//strcpy(device->device_ID,cJSON_Print(jserial_ID));
	device->device_ID = cJSON_Print(jserial_ID);

	jclient_ID = cJSON_GetObjectItemCaseSensitive(jdevice,"NAME");

	device->client_ID = malloc(strlen(cJSON_Print(jclient_ID)));
	if (device->client_ID == NULL) fail();
	//strcpy(device->client_ID,cJSON_Print(jclient_ID));
	device->client_ID = cJSON_Print(jclient_ID);


	jcloud 	= cJSON_GetObjectItemCaseSensitive(jgateway,"cloud");

	jaws_url = cJSON_GetObjectItemCaseSensitive(jcloud,"HOST");
	cloud->aws_url = malloc(strlen(cJSON_Print(jaws_url)));
	if (cloud->aws_url == NULL) fail();
	//strcpy(cloud->aws_url,cJSON_Print(jaws_url));
	cloud->aws_url = cJSON_Print(jaws_url);

	jpub_topic_telemetry = cJSON_GetObjectItemCaseSensitive(jcloud,"TELEMETRY_PUBLISH_TOPIC");
	cloud->pub_topic_telemetry = malloc(strlen(cJSON_Print(jpub_topic_telemetry)));
	if(cloud->pub_topic_telemetry == NULL) fail();
	//strcpy(cloud->pub_topic_telemetry,cJSON_Print(jpub_topic_telemetry));
	cloud->pub_topic_telemetry = cJSON_Print(jpub_topic_telemetry);


	jpub_topic_status = cJSON_GetObjectItemCaseSensitive(jcloud,"STATUS_PUBLISH_TOPIC");
	cloud->pub_topic_status = malloc(strlen(cJSON_Print(jpub_topic_status)));
	if(cloud->pub_topic_status == NULL) fail();
	//strcpy(cloud->pub_topic_status,cJSON_Print(jpub_topic_status));
	cloud->pub_topic_status = cJSON_Print(jpub_topic_status);

	jsub_topic_request = cJSON_GetObjectItemCaseSensitive(jcloud,"JOB_TOPIC");
	cloud->sub_topic_request = malloc(strlen(cJSON_Print(jsub_topic_request)));
	if(cloud->sub_topic_request == NULL) fail();
	//strcpy(cloud->sub_topic_request,cJSON_Print(sub_topic_request));
	cloud->sub_topic_request = cJSON_Print(jsub_topic_request);

	end:
		cJSON_Delete(jgateway);

		return 0;


}


int main()
{
        data_buf = malloc(4000 * sizeof(char));
	if (data_buf == NULL) fail();

        DEVICE gateway_device;
	CLOUD gateway_cloud;

        read_config_file(CONF_FILE_PATH,"r");
        printf("%s\n",data_buf);
        

        parse_json_file(&gateway_device,&gateway_cloud);

        printf("%s\n",gateway_cloud.aws_url);
        printf("%ld\n",strlen(gateway_cloud.aws_url));

        printf("%s\n",gateway_device.client_ID);
        printf("%ld\n",strlen(gateway_device.client_ID));


        printf("%ld\n",strlen("ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"));

        if(!strcmp(gateway_cloud.aws_url,"ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"))
        {
                printf("strings are same \n");
        }

        free(data_buf);


        // strcpy(gateway_cloud.aws_url,"ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883");
        // strcpy(gateway_device.client_ID,"gateway_G001");
        // gateway_cloud.aws_url = "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883";
        // gateway_device.device_ID = "gateway_G001";



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
	MQTTClient_create(&client,gateway_cloud.aws_url , gateway_device.device_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	printf("create\n");
	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	printf("trying to connect\n");

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("connected\n");

	// subscribe_to_reserved_topic(client,&gateway_device);
	sleep(500);

        MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);


        return 0;
}