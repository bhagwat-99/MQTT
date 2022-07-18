#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <json-c/json.h>
#include "MQTTClient.h"

#define ADDRESS                     "ssl://a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com:8883"
#define CLIENTID                    "gateway_G001"
#define PUB_TOPIC_CERT              "$aws/certificates/create/json"
#define PUB_TOPIC_THING             "$aws/provisioning-templates/gatewayTemplate/provision/json"
#define SUB_TOPIC_CERT_ACCEPT       "$aws/certificates/create/json/accepted"
#define SUB_TOPIC_CERT_REJECT       "$aws/certificates/create/json/rejected"

#define SUB_TOPIC_THING_ACCEPT       "$aws/provisioning-templates/gatewayTemplate/provision/json/accepted"
#define SUB_TOPIC_THING_REJECT       "$aws/provisioning-templates/gatewayTemplate/provision/json/rejected"

#define QOS                         1
#define TIMEOUT                     10000L


//provision certificate path
char * cafile = "/etc/gateway/provision_certificates/AmazonRootCA1.pem";
char * cert = "/etc/gateway/provision_certificates/device_certificate.crt";
char * key = "/etc/gateway/provision_certificates/device_private.key";


char pcert_data[4000]; //character pointer to store certificate data string
// char * ack_topic;
// char * ack_payload;
char * ppayload;
char SerialNumber[]="G001";

int rc; //return value variable
int msg_arrvd_flag = 0; //flag to check msg arrived



//volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;




void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("message delivered \n");
}

int create_certificates()
{

    //printf("%s\n",pcert_data);

    struct json_object *parsed_json;
	struct json_object *certificateId;
	struct json_object *certificatePem;
    struct json_object *privateKey;
	struct json_object *certificateOwnershipToken;
    

    parsed_json = json_tokener_parse(pcert_data);

    json_object_object_get_ex(parsed_json, "certificateId", &certificateId);
	json_object_object_get_ex(parsed_json, "certificatePem", &certificatePem);
	json_object_object_get_ex(parsed_json, "privateKey", &privateKey);
    json_object_object_get_ex(parsed_json, "certificateOwnershipToken", &certificateOwnershipToken);

    // printf("certificate ID : %s\n",json_object_get_string(certificateId));
    // printf("certificatePem : %s\n",json_object_get_string(certificatePem));
    // printf("privateKey : %s\n",json_object_get_string(privateKey));
    // printf("certificateOwnershiptoken : %s\n",json_object_get_string(certificateOwnershipToken));



    
    FILE * fptr;
    //writing test_device.pem
    fptr = fopen("/etc/gateway/certificates/device_certificate.crt","w");
    if(fptr == NULL)
    {
            printf("ERROR opening the file device_certificate.crt\n");
            exit(1);
    }

    fprintf(fptr,"%s", json_object_get_string(certificatePem));

    fclose(fptr);


    // writing private key to file
    //FILE * fptr;
    fptr = fopen("/etc/gateway/certificates/device_private.key","w");
    if(fptr == NULL)
    {
            printf("ERROR opening the file device_private.key\n");
            exit(1);
    }

    fprintf(fptr,"%s", json_object_get_string(privateKey));

    fclose(fptr);

    printf("certificates created successfully\n");

    //strore certificate owenership token in pcertdata temporary for usign in payload formation
    strcpy(pcert_data,(char *)json_object_get_string(certificateOwnershipToken));
    //printf("certificateOwenershipToken : %s\n",pcert_data);


    //resetting msg_arrvd_flag flag
    msg_arrvd_flag = 0;

    return 0;

    
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    //printf("enter msgarrvd\n");
    msg_arrvd_flag = 1;
    
    printf("\nMessage arrived on ");
    printf("topic: %s\n", topicName);
    //printf("message: \n%s\n\n",(char *)message->payload);
    //printf("hello\n");
    //printf("payload length %d\n",message->payloadlen);
    //strcpy(pcert_data,message->payload);
    //printf("%s\n",pcert_data);

    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    //printf("exit msgarrvd\n");

    
    return 1;
}


void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void publish(char * publish_topic)
{
        pubmsg.payload = ppayload;
        pubmsg.payloadlen = (int)strlen(ppayload);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        //printf("in publish\n");
        if ((rc = MQTTClient_publishMessage(client, publish_topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to publish message, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }
        //printf("in publish 2\n");

}

int subscribe_to_reserved_topic()
{
    //subscribing to SUB_TOPIC_THING_ACCEPT 
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC_THING_ACCEPT , CLIENTID, QOS);
    if ((rc = MQTTClient_subscribe(client, SUB_TOPIC_THING_ACCEPT , QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	exit(EXIT_FAILURE);

    }

    //subscribing to SUB_TOPIC_THING_REJECT 
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC_THING_REJECT , CLIENTID, QOS);
    if ((rc = MQTTClient_subscribe(client, SUB_TOPIC_THING_REJECT , QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	exit(EXIT_FAILURE);

    }

    //subscribing to SUB_TOPIC_CERT_ACCEPT 
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC_CERT_ACCEPT , CLIENTID, QOS);
    if ((rc = MQTTClient_subscribe(client, SUB_TOPIC_THING_ACCEPT , QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	exit(EXIT_FAILURE);

    }

    //subscribing to SUB_TOPIC_CERT_REJECT 
    printf("Subscribing to topic %s for client %s using QoS%d \n", SUB_TOPIC_CERT_REJECT , CLIENTID, QOS);
    if ((rc = MQTTClient_subscribe(client, SUB_TOPIC_THING_REJECT , QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	exit(EXIT_FAILURE);

    }

    return 0;

}

int create_register_thing_payload()
{
   
    /*Creating a json object*/
    json_object * jobj = json_object_new_object();
    json_object * jparameters = json_object_new_object();

    /*Creating a json string*/
    json_object *jcertownertoken = json_object_new_string(pcert_data);

    json_object *jserial_ID = json_object_new_string(SerialNumber);

    json_object_object_add(jparameters,"SerialNumber",jserial_ID);

    json_object_object_add(jobj,"certificateOwnershipToken",jcertownertoken);
    json_object_object_add(jobj,"parameters",jparameters);
    //printf ("%s\n",json_object_to_json_string(jobj));
    //ppayload[0]='\0';
    strcpy(ppayload,json_object_to_json_string(jobj));
    return 0;

}

int main(int argc, char* argv[])
{

    
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    ssl_opts.keyStore = cert;
    ssl_opts.trustStore = cafile;
    ssl_opts.privateKey = key;

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.ssl = &ssl_opts;


    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("connected\n");

    subscribe_to_reserved_topic();
    
    

    //allocating 500 bytes of memory for ppayload . size depend on payload
    ppayload = malloc(4000);
    if(ppayload == NULL)
    {
        printf("Error allocating memory for payload\n");
        exit(1);
    }
    ppayload[0] = '\0';   // ensures the memory is an empty string

    //need empty payload for create certificate
    strcpy(ppayload,"");

    //publish on reserved topic with empty payload for create certificate
    publish(PUB_TOPIC_CERT);
    
    //wait for response
    while(msg_arrvd_flag == 0);

    //create certificate files from string
    create_certificates();

    // //payload for register thing
    create_register_thing_payload();

    

    printf("payload  %s\n",ppayload);
    //printf("flag : %d\n",msg_arrvd_flag);


    // //publish on reserved topic for register thing
    publish(PUB_TOPIC_THING);

    //publish(PUB_TOPIC_CERT);


    printf("publish thing done \n");

    while(msg_arrvd_flag == 0);

    // //wait for response
    
    //while(msg_arrvd_flag == 0);
    printf("\nhello\n");

    printf("\n%s\n",pcert_data);
    

    // msg_arrvd_flag = 0;



    sleep(100);

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}