from threading import Thread
import paho.mqtt.client as mqtt

class MQTTClient(Thread):
    def __init__(self, pipeline):
        Thread.__init__(self)

        self.pipeline = pipeline
        self.daemon = True
        self.client = mqtt.Client()
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message

    def run(self):
        self.client.connect('127.0.0.1', 1883, 60)
        self.client.loop_forever()

    def _on_connect(self, client, userdata, flags, rc):
        client.subscribe("/nodes/#")

    def _on_message(self, client, userdata, message):
        try:
            print(f"MESSAGE({message.topic}): {message.payload}")
            topic = message.topic.split('/')[1:]
            node_id = int(topic[1], 16)
            if topic[2] == 'state':
               self._on_state(node_id, message.payload.decode('ascii'))

        except Exception as e:
            print(f"MQTT MSG ERROR: {e}")

    def _on_state(self, node_id, message):
        print(node_id, message)
        if message == '0':
            self.pipeline.remove_node(node_id)
        else:
            self.pipeline.add_node(node_id, message)
