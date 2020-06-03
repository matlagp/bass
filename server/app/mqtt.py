import os
import threading
import paho.mqtt.client as mqtt

class MQTTClient(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.daemon = True
        self.client = mqtt.Client()
        self.client.on_connect = __class__._on_connect
        self.client.on_message = __class__._on_message

    def run(self):
        self.client.connect('127.0.0.1', 1883, 60)
        self.client.loop_forever()

    def _on_connect(client, userdata, flags, rc):
        print(f"CONNECTED")
        client.subscribe("/nodes/#")

    def _on_message(client, userdata, message):
        print(f"MESSAGE({message.topic}): {message.payload}")
