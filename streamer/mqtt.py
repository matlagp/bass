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

            if topic[2] == 'settings' and topic[3] == 'volume':
                self._on_volume(node_id, int(message.payload.decode('ascii')))

            if topic[2] == 'settings' and topic[3] == 'bass':
                self._on_bass(node_id, float(message.payload.decode('ascii')))

            if topic[2] == 'settings' and topic[3] == 'mid':
                self._on_mid(node_id, float(message.payload.decode('ascii')))

            if topic[2] == 'settings' and topic[3] == 'trebble':
                self._on_trebble(node_id, float(message.payload.decode('ascii')))

        except Exception as e:
            print(f"MQTT message handling error: {e}")

    def _on_state(self, node_id, message):
        if message == '0':
            self.pipeline.remove_node(node_id)
        else:
            self.pipeline.add_node(node_id, message)

    def _on_volume(self, node_id, volume):
        if volume < 0 and volume > 100:
            raise ValueError("Volume not between 0 and 100")
        self.pipeline.set_volume(node_id, volume)

    def _on_bass(self, node_id, bass):
        if bass < -24 and bass > 12:
            raise ValueError("Bass not between -24 and 12")
        self.pipeline.set_bass(node_id, bass)

    def _on_mid(self, node_id, mid):
        if mid < -24 and mid > 12:
            raise ValueError("Mid not between -24 and 12")
        self.pipeline.set_mid(node_id, mid)

    def _on_trebble(self, node_id, trebble):
        if trebble < -24 and trebble > 12:
            raise ValueError("Trebble not between -24 and 12")
        self.pipeline.set_trebble(node_id, trebble)
