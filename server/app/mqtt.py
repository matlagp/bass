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

    def publish_node_setting(self, node_hex_id, setting, message):
        __class__._publish_node_setting(self.client, node_hex_id, setting, message)

    def _on_connect(client, userdata, flags, rc):
        client.subscribe("/nodes/+/state")
        client.subscribe("/nodes/+/set/#")

    def _on_message(client, userdata, message):
        try:
            print(f"MQTT message: ({message.topic}): {message.payload}")
            topic = message.topic.split('/')[1:]
            node_id = int(topic[1], 16)
            msg = message.payload.decode('ascii')

            if topic[2] == 'state':
                node = Node(node_id, message.payload.decode('ascii'))
                __class__.node_repository.create(node)

            elif topic[2] == 'set':
                node = __class__.node_repository.find(node_id)

                if topic[3] == 'volume':
                    incoming_volume = int(msg)
                    new_volume = __class__._new_setting_value(node.volume, incoming_volume, msg)

                    __class__._check_range(new_volume, 0, 100, "Volume")
                    node.volume = new_volume
                    __class__._publish_node_setting(client, topic[1], 'volume', new_volume)

                elif topic[3] == 'bass':
                    incoming_bass = float(msg)
                    new_bass = __class__._new_setting_value(node.bass, incoming_bass, msg)

                    __class__._check_range(new_bass, -24, 12, "Bass")
                    node.bass = new_bass
                    __class__._publish_node_setting(client, topic[1], 'bass', new_bass)

                elif topic[3] == 'mid':
                    incoming_mid = float(msg)
                    new_mid = __class__._new_setting_value(node.mid, incoming_mid, msg)

                    __class__._check_range(new_mid, -24, 12, "Mid")
                    node.mid = new_mid
                    __class__._publish_node_setting(client, topic[1], 'mid', new_mid)

                elif topic[3] == 'trebble':
                    incoming_trebble = float(msg)
                    new_trebble = __class__._new_setting_value(node.trebble, incoming_trebble, msg)

                    __class__._check_range(new_trebble, -24, 12, "Trebble")
                    node.trebble = new_trebble
                    __class__._publish_node_setting(client, topic[1], 'trebble', new_trebble)

                __class__.node_repository.update(node)

        except Exception as e:
            print(f"MQTT message handling error: {e}")

    def _new_setting_value(old_value, incoming_value, value_message):
        if value_message[0] in ['+', '-']:
            return old_value + incoming_value

        return incoming_value

    def _check_range(value, min_value, max_value, comment):
        if value < min_value or value > max_value:
            raise ValueError(f"{comment}({value}) not between {min_value} and {max_value}")

    def _publish_node_setting(client, node_hex_id, setting, message):
        client.publish(f"/nodes/{node_hex_id}/get/{setting}", payload=message, qos=1, retain=True)
