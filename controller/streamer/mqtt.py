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
        client.subscribe("/nodes/+/state")
        client.subscribe("/nodes/+/get/#")

    def _on_message(self, client, userdata, message):
        try:
            print(f"MQTT message: ({message.topic}): {message.payload}")
            topic = message.topic.split('/')[1:]
            node_id = int(topic[1], 16)
            msg = message.payload.decode('ascii')

            if topic[2] == 'state':
               self._on_state(node_id, msg)

            elif topic[2] == 'get':
                if topic[3] == 'volume':
                    self._on_volume(node_id, int(msg))

                elif topic[3] == 'bass':
                    self._on_bass(node_id, float(msg))

                elif topic[3] == 'mid':
                    self._on_mid(node_id, float(msg))

                elif topic[3] == 'trebble':
                    self._on_trebble(node_id, float(msg))

        except Exception as e:
            print(f"MQTT message handling error: {e}")

    def _on_state(self, node_id, ip):
        if len(ip) < 2: # '0' or '' or something else that's invalid
            self.pipeline.remove_node(node_id)
        else:
            self.pipeline.add_node(node_id, ip)

    def _on_volume(self, node_id, volume):
        __class__._check_range(volume, 0, 100, "Volume")
        self.pipeline.set_volume(node_id, volume)

    def _on_bass(self, node_id, bass):
        __class__._check_range(bass, -24, 12, "Bass")
        self.pipeline.set_bass(node_id, bass)

    def _on_mid(self, node_id, mid):
        __class__._check_range(mid, -24, 12, "Mid")
        self.pipeline.set_mid(node_id, mid)

    def _on_trebble(self, node_id, trebble):
        __class__._check_range(trebble, -24, 12, "Trebble")
        self.pipeline.set_trebble(node_id, trebble)

    def _check_range(value, min_value, max_value, comment):
        if value < min_value or value > max_value:
            raise ValueError(f"{comment}({value}) not between {min_value} and {max_value}")
