import paho.mqtt.client as mqtt
from random import uniform
import time as t


mqttBroker = "a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com"
client = mqtt.Client("gateway_G001")
port = 8883
client.tls_set("/etc/gateway/certificates/AmazonRootCA1.pem","/etc/gateway/certificates/device_certificate.crt","/etc/gateway/certificates/device_private.key")
MQTT_KEEPALIVE_INTERVAL = 45
topic = "gateway/request"

# Callback Functions
def on_log(client, userdata, level, buf):
	print("Log: "+buf)

def on_connect(client, userdata, flags, response_code):
	# Checking for established connection.
    if response_code == 0:
        conFlag = True
        print("Connected with status: {0}".format(response_code))	
        client.subscribe(topic, 1)
    else:
        print("Bad Connection", response_code)

def on_message(client, userdata, message):
	print("Recieved Message: ", str(message.payload.decode('utf-8')))

# Attempting Connection
client.on_connect = on_connect
client.on_message = on_message

# try:
client.connect(mqttBroker, port, MQTT_KEEPALIVE_INTERVAL)
# except:
# 	print("Did not connect")
client.loop_forever()
