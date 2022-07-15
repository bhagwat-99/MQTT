#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "cjson/cJSON.h"


char * data_buf;


typedef struct DEVICE
{

	char *certOwnership_token;
	char device_ID[10];

}DEVICE;

typedef struct CLOUD
{
	char	aws_url[100];
	char	pub_topic_telemetry[50];
	char	pub_topic_status[50];
	char 	sub_topic_request[50];

}CLOUD;

DEVICE gateway_device;
CLOUD gateway_cloud;

char * read_config_file(char * filename,char * mode)
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
	return data_buf;

}

int parse_json_file(const char * jsonfile)
{
	char * data;
	cJSON * cloud = NULL;
	cJSON * aws_url = NULL;
	cJSON * pub_topic_telemetry = NULL;
	cJSON * pub_topic_status = NULL;
	cJSON * sub_topic_request = NULL;

	cJSON * device = NULL;
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

	device 	= cJSON_GetObjectItemCaseSensitive(gateway,"device");
	serial_ID = cJSON_GetObjectItemCaseSensitive(device,"SERIAL_ID");
	strcpy(gateway_device.device_ID,cJSON_Print(serial_ID));


	cloud 	= cJSON_GetObjectItemCaseSensitive(gateway,"cloud");
	aws_url = cJSON_GetObjectItemCaseSensitive(cloud,"HOST");
	strcpy(gateway_cloud.aws_url,cJSON_Print(aws_url));

	aws_url = cJSON_GetObjectItemCaseSensitive(cloud,"HOST");
	strcpy(gateway_cloud.aws_url,cJSON_Print(aws_url));

	pub_topic_telemetry = cJSON_GetObjectItemCaseSensitive(cloud,"TELEMETRY_PUBLISH_TOPIC");
	strcpy(gateway_cloud.pub_topic_telemetry,cJSON_Print(pub_topic_telemetry));

	pub_topic_status = cJSON_GetObjectItemCaseSensitive(cloud,"STATUS_PUBLISH_TOPIC");
	strcpy(gateway_cloud.pub_topic_status,cJSON_Print(pub_topic_status));

	sub_topic_request = cJSON_GetObjectItemCaseSensitive(cloud,"JOB_TOPIC");
	strcpy(gateway_cloud.sub_topic_request,cJSON_Print(sub_topic_request));



	end:
		cJSON_Delete(gateway);

		return 0;


}


char * create_register_thing_payload(struct DEVICE device)
{

	/*Creating a json object*/
	cJSON * jobj = cJSON_CreateObject();
	
	/*Creating a json string*/
	cJSON * jcertownertoken = cJSON_CreateString(device.certOwnership_token);
	cJSON_AddItemToObject(jobj,"certificateOwnershipToken", jcertownertoken);

	cJSON * jparameters = cJSON_CreateObject();
	cJSON_AddItemToObject(jobj, "parameters", jparameters);

	cJSON *jdevice_ID = cJSON_CreateString(device.device_ID);
	cJSON_AddItemToObject(jparameters,"SerialNumber", jdevice_ID);

	// printf ("%s\n",json_object_to_json_string(jobj));
	// ppayload[0]='\0';
	//strcpy(ppayload, json_object_to_json_string(jobj));

	strcpy(data_buf,cJSON_Print(jobj));

	cJSON_Delete(jobj);
	

	return data_buf;
}

int main()
{
	data_buf = malloc(4000);
	if(data_buf == NULL)
	{
		printf("error allocating memory for data_buf");
	}

	read_config_file("/etc/gateway/gateway.conf","r");

	parse_json_file(data_buf);
	

	return 0;
}