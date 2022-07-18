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

// char * test_url = "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883";
// char * test_client_ID = "gateway";

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

int create_register_thing_payload(struct DEVICE *device)
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

	strcpy(data_buf,cJSON_Print(jobj));

	cJSON_Delete(jobj);
	

	return 0;
}

int create_certificates(struct DEVICE *device )
{
	cJSON * parsed_json;
	cJSON * certificateID;
	cJSON * certificatePem;
	cJSON * privateKey;
	cJSON * certificateOwnershipToken;

	parsed_json = cJSON_Parse(data_buf);

	if(parsed_json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
		fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		goto end;
	}

	certificateID = cJSON_GetObjectItemCaseSensitive(parsed_json,"certificateId");
	certificatePem = cJSON_GetObjectItemCaseSensitive(parsed_json,"certificatePem");
	privateKey = cJSON_GetObjectItemCaseSensitive(parsed_json,"privateKey");
	certificateOwnershipToken = cJSON_GetObjectItemCaseSensitive(parsed_json,"certificateOwnershipToken");

	//creating certificate files

	FILE * fptr;
	//writing test_device.pem
	fptr = fopen("/etc/gateway/certificates/device_certificate.crt","w");
	if(fptr == NULL)
	{
		printf("ERROR opening the file device_certificate.crt\n");
		exit(1);
	}

	fprintf(fptr,"%s", cJSON_Print(certificatePem));

	fclose(fptr);


	// writing private key to file
	//FILE * fptr;
	fptr = fopen("/etc/gateway/certificates/device_private.key","w");
	if(fptr == NULL)
	{
		printf("ERROR opening the file device_private.key\n");
		exit(1);
	}

	fprintf(fptr,"%s", cJSON_Print(privateKey));

	fclose(fptr);

	printf("certificates created successfully\n");

	//strore certificate owenership token in pcertdata temporary for usign in payload formation
	//strcpy(device->certOwnership_token, cJSON_Print(certificateOwnershipToken));
	device->certOwnership_token = malloc(strlen(cJSON_Print(certificateOwnershipToken)+1));
	if(device->certOwnership_token == NULL) fail();
	device->certOwnership_token = cJSON_Print(certificateOwnershipToken);
	//printf("certificateOwenershipToken : %s\n",pcert_data);


	//resetting msg_arrvd_flag flag
	msg_arrvd_flag = 0;


	end:
		cJSON_Delete(parsed_json);

		return 0;



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

int subscribe_to_reserved_topic(MQTTClient client, DEVICE * device)
{
	//subscribing to SUB_TOPIC_THING_ACCEPT 
	if ((rc = MQTTClient_subscribe(client, SUB_TOPIC_THING_ACCEPT , QOS)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to subscribe, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("Subscribed to topic %s for client %s using QoS%d \n", SUB_TOPIC_THING_ACCEPT , device->client_ID, QOS);


	//subscribing to SUB_TOPIC_THING_REJECT 
	if ((rc = MQTTClient_subscribe(client, SUB_TOPIC_THING_REJECT , QOS)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to subscribe, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("Subscribed to topic %s for client %s using QoS%d \n", SUB_TOPIC_THING_ACCEPT , device->client_ID, QOS);


	//subscribing to SUB_TOPIC_CERT_ACCEPT 
	if ((rc = MQTTClient_subscribe(client, SUB_TOPIC_THING_ACCEPT , QOS)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to subscribe, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("Subscribed to topic %s for client %s using QoS%d \n", SUB_TOPIC_THING_ACCEPT , device->client_ID, QOS);


	//subscribing to SUB_TOPIC_CERT_REJECT 
	if ((rc = MQTTClient_subscribe(client, SUB_TOPIC_THING_REJECT , QOS)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to subscribe, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("Subscribed to topic %s for client %s using QoS%d \n", SUB_TOPIC_THING_ACCEPT , device->client_ID, QOS);


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

	device->device_ID = malloc(strlen(cJSON_Print(jserial_ID) + 1));
	if (device->device_ID == NULL) fail();
	//strcpy(device->device_ID,cJSON_Print(jserial_ID));
	device->device_ID = cJSON_Print(jserial_ID);

	jclient_ID = cJSON_GetObjectItemCaseSensitive(jdevice,"NAME");
	device->client_ID = malloc(strlen(cJSON_Print(jclient_ID) + 1));
	if (device->client_ID == NULL) fail();
	//strcpy(device->client_ID,cJSON_Print(jclient_ID));
	device->client_ID = cJSON_Print(jclient_ID);


	jcloud 	= cJSON_GetObjectItemCaseSensitive(jgateway,"cloud");

	jaws_url = cJSON_GetObjectItemCaseSensitive(jcloud,"HOST");
	cloud->aws_url = malloc(strlen(cJSON_Print(jaws_url) + 1));
	if (cloud->aws_url == NULL) fail();
	//strcpy(cloud->aws_url,cJSON_Print(jaws_url));
	cloud->aws_url = cJSON_Print(jaws_url);

	jpub_topic_telemetry = cJSON_GetObjectItemCaseSensitive(jcloud,"TELEMETRY_PUBLISH_TOPIC");
	cloud->pub_topic_telemetry = malloc(strlen(cJSON_Print(jpub_topic_telemetry)+1));
	if(cloud->pub_topic_telemetry == NULL) fail();
	//strcpy(cloud->pub_topic_telemetry,cJSON_Print(jpub_topic_telemetry));
	cloud->pub_topic_telemetry = cJSON_Print(jpub_topic_telemetry);


	jpub_topic_status = cJSON_GetObjectItemCaseSensitive(jcloud,"STATUS_PUBLISH_TOPIC");
	cloud->pub_topic_status = malloc(strlen(cJSON_Print(jpub_topic_status)+1));
	if(cloud->pub_topic_status == NULL) fail();
	//strcpy(cloud->pub_topic_status,cJSON_Print(jpub_topic_status));
	cloud->pub_topic_status = cJSON_Print(jpub_topic_status);

	jsub_topic_request = cJSON_GetObjectItemCaseSensitive(jcloud,"JOB_TOPIC");
	cloud->sub_topic_request = malloc(strlen(cJSON_Print(jsub_topic_request)+1));
	if(cloud->sub_topic_request == NULL) fail();
	//strcpy(cloud->sub_topic_request,cJSON_Print(sub_topic_request));
	cloud->sub_topic_request = cJSON_Print(jsub_topic_request);

	end:
		cJSON_Delete(jgateway);

		return 0;


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


int main()
{
	data_buf = malloc(4000 * sizeof(char));
	if (data_buf == NULL) fail();

	printf("debug1\n");

	DEVICE gateway_device;
	CLOUD gateway_cloud;

	printf("debug2\n");

	

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

	printf("debug3\n");

	read_config_file(CONF_FILE_PATH,"r");

	printf("read_config_file_done\n");

	//printf("%s\n",data_buf);


	//parse tha json file and store the device and cloud data in respective struct
	parse_json_file(&gateway_device,&gateway_cloud);
	printf("parsed json file\n");
	free(data_buf);

	//printf("%s\n",gateway_device.client_ID);

	//printf("%s\n",gateway_cloud.aws_url);




	//MQTTClient_create(&client, gateway_cloud.aws_url, gateway_device.client_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	MQTTClient_create(&client, gateway_cloud.aws_url, gateway_device.client_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
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



	

	data_buf = malloc(4000 * sizeof(char));
	if (data_buf == NULL) fail();

	//publish on reserved topic with empty payload for create certificate
	strcpy(data_buf,"");
	publish(client,pubmsg,token,PUB_TOPIC_CERT,data_buf);

	//wait for response
	while(msg_arrvd_flag == 0);

	//create certificate files from string
	create_certificates(&gateway_device);

	create_register_thing_payload(&gateway_device);

	printf("payload  %s\n",data_buf);

	// //publish on reserved topic for register thing
	publish(client,pubmsg,token,PUB_TOPIC_THING,data_buf);

	printf("publish thing done \n");

	while(msg_arrvd_flag == 0);

	// //wait for response

	//while(msg_arrvd_flag == 0);
	printf("\nhello\n");

	//printf("\n%s\n",pcert_data);


	sleep(100);

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	return 0;
}