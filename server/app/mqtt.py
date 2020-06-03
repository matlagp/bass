import os
import threading
import paho.mqtt.client as mqtt
from .models import Node
from .repositories import NodeRepository

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
        topic = message.topic.split('/')[1:]
        print(topic)
        if topic[2] == 'state':
            try:
                node = Node(int(topic[1], 16), message.payload.decode('ascii'))
                print(node)
                NodeRepository().create(node)
                print('inserting')
            except Exeption as e:
                print(e)
