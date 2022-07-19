#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "cjson/cJSON.h"
#include "MQTTClient.h"


#define CONF_FILE_PATH "/etc/gateway/gateway.conf"


char * data_buf;


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


void fail(void) {printf("error allocating memory \n"); exit(EXIT_FAILURE);}

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

int main()
{
	data_buf = malloc(4000 * sizeof(char));
	if (data_buf == NULL) fail();

        DEVICE gateway_device;
	CLOUD gateway_cloud;

	read_config_file(CONF_FILE_PATH,"r");

	parse_json_file(&gateway_device,&gateway_cloud);

	printf("%s\n",gateway_device.client_ID);
	printf("%s\n",gateway_device.device_ID);
	printf("%s\n",gateway_cloud.aws_url);
	printf("%s\n",gateway_cloud.pub_topic_status);
	printf("%s\n",gateway_cloud.pub_topic_telemetry);
	printf("%s\n",gateway_cloud.sub_topic_request);


	

	
	return 0;

}