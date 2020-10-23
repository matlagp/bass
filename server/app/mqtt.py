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
            print(f"MQTT message: ({message.topic}): {message.payload}")
            topic = message.topic.split('/')[1:]
            node_id = int(topic[1], 16)
            msg = message.payload.decode('ascii')

            if topic[2] == 'state':
                node = Node(node_id, message.payload.decode('ascii'))
                __class__.node_repository.create(node)

            elif topic[2] == 'settings':
                node = __class__.node_repository.find(node_id)

                if topic[3] == 'volume':
                    volume_diff = int(msg)
                    new_volume = node.volume + volume_diff
                    __class__._check_range(new_volume, 0, 100, "Volume")
                    node.volume = new_volume

                elif topic[3] == 'bass':
                    bass_diff = float(msg)
                    new_bass = node.bass + bass_diff
                    __class__._check_range(new_bass, -24, 12, "Bass")
                    node.bass = new_bass

                elif topic[3] == 'mid':
                    mid_diff = float(msg)
                    new_mid = node.mid + mid_diff
                    __class__._check_range(new_mid, -24, 12, "Mid")
                    node.mid = new_mid

                elif topic[3] == 'trebble':
                    trebble_diff = float(msg)
                    new_trebble = node.trebble + trebble_diff
                    __class__._check_range(new_trebble, -24, 12, "Trebble")
                    node.trebble = new_trebble

                __class__.node_repository.update(node)

        except Exception as e:
            print(f"MQTT message handling error: {e}")

    def _check_range(value, min_value, max_value, comment):
        if value < min_value or value > max_value:
            raise ValueError(f"{comment}({value}) not between {min_value} and {max_value}")
