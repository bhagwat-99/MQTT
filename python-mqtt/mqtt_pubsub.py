import paho.mqtt.client as mqtt
from random import uniform
import time as t


data = 256
# Basic Connection Configuration
mqttBroker = "a33enhgkqb6z8i-ats.iot.us-west-2.amazonaws.com"
client = mqtt.Client("gateway_test_device")
port = 8883
# MQTT_KEEPALIVE_INTERVAL = 45
MQTT_TOPIC_PUB = "gateway_pub"
MQTT_TOPIC_SUB = "gateway_sub"
client.tls_set("/test_certificates/AmazonRootCA1.pem","/test_certificates/device_certificate.crt","/test_certificates/device_private.key")

def on_publish(client, userdata, mid):
    print ("Message Published...")

def on_connect(client, userdata, flags, rc):
	if rc == 0:
		print("Connected OK")
	else:
		print("Bad connection")
	client.subscribe(MQTT_TOPIC_PUB)
	client.publish(MQTT_TOPIC_SUB, 123)

def on_message(client, userdata, msg):
    print(msg.topic)
    print(msg.payload) # <- do you mean this payload = {...} ?
    # payload = json.loads(msg.payload) # you can use json.loads to convert string to json
    # print(payload['sepalWidth']) # then you can check the value
    client.disconnect() # Got message then disconnect

# Initiate MQTT Client
mqttc = mqtt.Client()

# Register publish callback function
mqttc.on_publish = on_publish
mqttc.on_connect = on_connect
mqttc.on_message = on_message

# Connect with MQTT Broker
mqttc.connect(mqttBroker, port)

# Loop forever
mqttc.loop_forever()