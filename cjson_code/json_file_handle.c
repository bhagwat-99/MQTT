#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "cjson/cJSON.h"

#define CONF_FILE_PATH "/etc/gateway/gateway.conf"

void fail(void) {printf("error allocating memory \n"); exit(EXIT_FAILURE);}


typedef struct DEVICE
{

	char *certOwnership_token;
	char device_ID[10];
	char 	client_ID[20];

}DEVICE;

typedef struct CLOUD
{
	char	aws_url[100];
	char	pub_topic_telemetry[50];
	char	pub_topic_status[50];
	char 	sub_topic_request[50];

}CLOUD;



int read_config_file(char * filename,char * mode,char * data)
{
	FILE * fd;
	fd = fopen(filename,mode);
	if(fd == NULL)
	{
		printf("error opening file : /etc/gateway/gateway.conf");
		exit(1);
	}
	fread(data,4000,1,fd);
	fclose(fd);
	return 0;

}

int parse_json_file(const char * jsonfile, DEVICE *device, CLOUD *cloud)
{
	cJSON * jcloud = NULL;
	cJSON * aws_url = NULL;
	cJSON * client_ID = NULL;
	cJSON * pub_topic_telemetry = NULL;
	cJSON * pub_topic_status = NULL;
	cJSON * sub_topic_request = NULL;

	cJSON * jdevice = NULL;
	cJSON * serial_ID = NULL;
	

	cJSON * gateway = cJSON_Parse(jsonfile);

	if ( gateway == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
		fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	//storing parsed data into device gateway and cloud structure

	jdevice 	= cJSON_GetObjectItemCaseSensitive(gateway,"device");
	serial_ID = cJSON_GetObjectItemCaseSensitive(jdevice,"SERIAL_ID");
	strcpy(device->device_ID,cJSON_Print(serial_ID));

	client_ID = cJSON_GetObjectItemCaseSensitive(jdevice,"NAME");
	strcpy(device->client_ID,cJSON_Print(client_ID));


	jcloud 	= cJSON_GetObjectItemCaseSensitive(gateway,"cloud");
	aws_url = cJSON_GetObjectItemCaseSensitive(jcloud,"HOST");
	strcpy(cloud->aws_url,cJSON_Print(aws_url));

	pub_topic_telemetry = cJSON_GetObjectItemCaseSensitive(jcloud,"TELEMETRY_PUBLISH_TOPIC");
	strcpy(cloud->pub_topic_telemetry,cJSON_Print(pub_topic_telemetry));

	pub_topic_status = cJSON_GetObjectItemCaseSensitive(jcloud,"STATUS_PUBLISH_TOPIC");
	strcpy(cloud->pub_topic_status,cJSON_Print(pub_topic_status));

	sub_topic_request = cJSON_GetObjectItemCaseSensitive(jcloud,"JOB_TOPIC");
	strcpy(cloud->sub_topic_request,cJSON_Print(sub_topic_request));



	end:
		cJSON_Delete(gateway);

		return 0;


}


int create_register_thing_payload(struct DEVICE *device, char * jdata)
{

	/*Creating a json object*/
	cJSON * jobj = cJSON_CreateObject();
	
	/*Creating a json string*/
	cJSON * jcertownertoken = cJSON_CreateString(device->certOwnership_token);
	cJSON_AddItemToObject(jobj,"certificateOwnershipToken", jcertownertoken);

	cJSON * jparameters = cJSON_CreateObject();
	cJSON_AddItemToObject(jobj, "parameters", jparameters);

	cJSON *jdevice_ID = cJSON_CreateString(device->device_ID);
	cJSON_AddItemToObject(jparameters,"SerialNumber", jdevice_ID);

	strcpy(jdata,cJSON_Print(jobj));

	cJSON_Delete(jobj);
	

	return 0;
}

int main()
{
	DEVICE gateway_device;
	CLOUD gateway_cloud;


	char * data_buf = malloc(4000 * sizeof(char));
	if (data_buf == NULL) fail();

	read_config_file(CONF_FILE_PATH,"r",data_buf);

	parse_json_file(data_buf,&gateway_device,&gateway_cloud);

	create_register_thing_payload(gateway_device,data_buf);

	printf("%s",data_buf);
	//printf("%s\n",gateway_device.device_ID);
	
	free(data_buf);
	return 0;
}