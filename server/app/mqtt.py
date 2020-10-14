import os
import threading
import paho.mqtt.client as mqtt
from .models import Node
from .repositories import NodeRepository


class MQTTClient(threading.Thread):
    node_repository = NodeRepository()

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
        client.subscribe("/nodes/#")

    def _on_message(client, userdata, message):
        try:
            print(f"MESSAGE({message.topic}): {message.payload}")
            topic = message.topic.split('/')[1:]
            node_id = int(topic[1], 16)
            if topic[2] == 'state':
                node = Node(node_id, message.payload.decode('ascii'))
                __class__.node_repository.create(node)

            if topic[2] == 'settings' and topic[3] == 'volume':
                volume = int(message.payload.decode('ascii'))
                node = __class__.node_repository.find(node_id)
                if volume < 0 or volume > 100:
                    raise ValueError("Volume not between 0 and 100")
                node.volume = volume
                __class__.node_repository.update(node)

            if topic[2] == 'settings' and topic[3] == 'bass':
                bass = float(message.payload.decode('ascii'))
                node = __class__.node_repository.find(node_id)
                if bass < -24 or bass > 12:
                    raise ValueError("Bass not between -24 and 12")
                node.bass = bass
                __class__.node_repository.update(node)

            if topic[2] == 'settings' and topic[3] == 'mid':
                mid = float(message.payload.decode('ascii'))
                node = __class__.node_repository.find(node_id)
                if mid < -24 or mid > 12:
                    raise ValueError("Mid not between -24 and 12")
                node.mid = mid
                __class__.node_repository.update(node)

            if topic[2] == 'settings' and topic[3] == 'trebble':
                trebble = float(message.payload.decode('ascii'))
                node = __class__.node_repository.find(node_id)
                if trebble < -24 or trebble > 12:
                    raise ValueError("Trebble not between -24 and 12")
                node.trebble = trebble
                __class__.node_repository.update(node)

        except Exception as e:
            print(f"MQTT MSG ERROR: {e}")
