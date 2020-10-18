import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst
from utils import *

BUFFER_SIZE = 512


class Pipeline(object):
    def __init__(self):
        self.nodes = {}
        self.pipeline = Gst.Pipeline()

        self.src = make_element('alsasrc')
        self.tee = make_element('tee')
        self.fakequeue = make_element('queue')
        self.fakesink = make_element('fakesink')

        add_to_pipeline(
            self.pipeline,
            [self.src, self.tee, self.fakequeue, self.fakesink]
        )

        link([self.src, self.tee])
        link_tee(self.tee, self.fakequeue)
        link([self.fakequeue, self.fakesink])

        self.fakesink.set_property('sync', False)

    def get_node(self, node_id):
        if node_id not in self.nodes:
            self.nodes[node_id] = Node(self, node_id)
        return self.nodes[node_id]

    def add_node(self, node_id, host):
        node = self.get_node(node_id)
        node.set_host(host)
        node.attach()
        print(self.nodes)

    def remove_node(self, node_id):
        node = self.get_node(node_id)
        node.detach()
        print(self.nodes)

    def set_volume(self, node_id, volume):
        node = self.get_node(node_id)
        node.set_volume(volume)
        print(self.nodes)

    def set_bass(self, node_id, bass):
        node = self.get_node(node_id)
        node.set_bass(bass)
        print(self.nodes)

    def set_mid(self, node_id, mid):
        node = self.get_node(node_id)
        node.set_mid(mid)
        print(self.nodes)

    def set_trebble(self, node_id, trebble):
        node = self.get_node(node_id)
        node.set_trebble(trebble)
        print(self.nodes)

    def pause(self):
        return self.pipeline.set_state(Gst.State.PAUSED)

    def play(self):
        return self.pipeline.set_state(Gst.State.PLAYING)


class Node(object):
    def __init__(self, pipeline, node_id):
        self.pipeline = pipeline
        self.node_id = node_id
        self.port = 2137
        self.attached = False

        self.vol = 100
        self.bass = 0.0
        self.mid = 0.0
        self.trebble = 0.0

    def __repr__(self):
        return f"Node#{self.node_id}"\
            f"(attached: {self.attached}, "\
            f"host: {self.host}, "\
            f"vol: {self.vol}, "\
            f"bass: {self.bass}, "\
            f"mid: {self.mid}, "\
            f"trebble: {self.trebble})"

    def set_volume(self, volume):
        self.vol = volume
        if self.attached:
            self.volume.set_property('volume', self.vol / 100)

    def set_host(self, host):
        self.host = host
        if self.attached:
            self.sink.set_property('host', host)

    def set_bass(self, bass):
        self.bass = bass
        if self.attached:
            self.equalizer.set_property('band0', self.bass)

    def set_mid(self, mid):
        self.mid = mid
        if self.attached:
            self.equalizer.set_property('band1', self.mid)

    def set_trebble(self, bass):
        self.trebble = bass
        if self.attached:
            self.equalizer.set_property('band2', self.trebble)

    def attach(self):
        if self.attached: return

        self.latency = make_element('queue')
        self.equalizer = make_element('equalizer-3bands')
        self.volume = make_element('volume')
        self.buffersize = make_element('rndbuffersize')
        self.sink = make_element('udpsink')

        self.buffersize.set_property('min', BUFFER_SIZE)
        self.buffersize.set_property('max', BUFFER_SIZE)

        self.sink.set_property('host', self.host)
        self.sink.set_property('port', self.port)

        elements = [self.latency, self.equalizer, self.volume, self.buffersize, self.sink]

        add_to_pipeline(self.pipeline.pipeline, elements)
        link(elements)

        self.pipeline.pause()

        self.tee = link_tee(self.pipeline.tee, self.latency)

        self.attached = True

        self.set_volume(self.vol)
        self.set_bass(self.bass)
        self.set_mid(self.mid)
        self.set_trebble(self.trebble)

        self.pipeline.play()

    def detach(self):
        if not self.attached:
            return

        self.pipeline.pause()
        unlink_tee(self.pipeline.tee, self.tee, self.latency)
        self.pipeline.play()

        elements = [self.latency, self.equalizer, self.volume, self.buffersize, self.sink]
        unlink(elements)
        remove_from_pipeline(self.pipeline.pipeline, elements)
        set_null_state(elements)
        unref(elements)
        self.latency, self.equalizer, self.volume, self.sink, self.tee = [ None for _ in elements ]

        self.attached = False
