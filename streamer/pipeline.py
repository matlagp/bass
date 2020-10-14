import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst

BUFFER_SIZE = 512

class Pipeline(object):
    def __init__(self):
        self.nodes = {}
        self.pipeline = Gst.Pipeline()

        self.src = Gst.ElementFactory.make('alsasrc')
        self.tee = Gst.ElementFactory.make('tee')
        self.fakequeue = Gst.ElementFactory.make('queue')
        self.fakesink = Gst.ElementFactory.make('fakesink')

        self.pipeline.add(self.src)
        self.pipeline.add(self.tee)
        self.pipeline.add(self.fakequeue)
        self.pipeline.add(self.fakesink)

        self.src.link(self.tee)
        self.tee.get_request_pad('src_%u').link(self.fakequeue.get_static_pad('sink'))
        self.fakequeue.link(self.fakesink)

        self.fakesink.set_property('sync', False)

    def pause(self):
        return self.pipeline.set_state(Gst.State.PAUSED)

    def play(self):
        return self.pipeline.set_state(Gst.State.PLAYING)

    def add_node(self, node_id, host):
        if node_id not in self.nodes:
            self.nodes[node_id] = Node(self, node_id)
        node = self.nodes[node_id]
        node.set_host(host)
        node.attach()
        print(self.nodes)

    def remove_node(self, node_id):
        if node_id not in self.nodes:
            self.nodes[node_id] = Node(self, node_id)
        node = self.nodes[node_id]
        node.detach()
        print(self.nodes)

    def set_volume(self, node_id, volume):
        if node_id not in self.nodes:
            self.nodes[node_id] = Node(self, node_id)
        node = self.nodes[node_id]
        node.set_volume(volume)
        print(self.nodes)

    def set_bass(self, node_id, bass):
        if node_id not in self.nodes:
            self.nodes[node_id] = Node(self, node_id)
        node = self.nodes[node_id]
        node.set_bass(bass)
        print(self.nodes)

    def set_mid(self, node_id, mid):
        if node_id not in self.nodes:
            self.nodes[node_id] = Node(self, node_id)
        node = self.nodes[node_id]
        node.set_mid(mid)
        print(self.nodes)

    def set_trebble(self, node_id, trebble):
        if node_id not in self.nodes:
            self.nodes[node_id] = Node(self, node_id)
        node = self.nodes[node_id]
        node.set_trebble(trebble)
        print(self.nodes)


class Node(object):
    def __init__(self, pipeline, node_id):
        self.pipeline = pipeline
        self.node_id = node_id
        self.port = 2137
        self.attached = False

        self.vol = 100

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

        self.latency = Gst.ElementFactory.make('queue')
        self.equalizer = Gst.ElementFactory.make('equalizer-3bands')
        self.volume = Gst.ElementFactory.make('volume')
        self.buffersize = Gst.ElementFactory.make('rndbuffersize')
        self.sink = Gst.ElementFactory.make('udpsink')

        self.buffersize.set_property('min', BUFFER_SIZE)
        self.buffersize.set_property('max', BUFFER_SIZE)

        self.sink.set_property('host', self.host)
        self.sink.set_property('port', self.port)

        self.pipeline.pipeline.add(self.latency)
        self.pipeline.pipeline.add(self.equalizer)
        self.pipeline.pipeline.add(self.volume)
        self.pipeline.pipeline.add(self.buffersize)
        self.pipeline.pipeline.add(self.sink)

        self.latency.link(self.equalizer)
        self.equalizer.link(self.volume)
        self.volume.link(self.buffersize)
        self.buffersize.link(self.sink)

        self.pipeline.pause()

        self.tee = self.pipeline.tee.get_request_pad('src_%u')
        self.tee.link(self.latency.get_static_pad('sink'))

        self.pipeline.play()

        self.attached = True

        self.set_volume(self.vol)
        self.set_bass(self.bass)
        self.set_mid(self.mid)
        self.set_trebble(self.trebble)

    def detach(self):
        if not self.attached: return

        self.pipeline.pause()

        self.tee.unlink(self.latency.get_static_pad('sink'))

        self.pipeline.tee.release_request_pad(self.tee)

        self.pipeline.play()

        self.latency.unlink(self.equalizer)
        self.equalizer.unlink(self.volume)
        self.volume.unlink(self.buffersize)
        self.buffersize.unlink(self.sink)

        self.pipeline.pipeline.remove(self.latency)
        self.pipeline.pipeline.remove(self.equalizer)
        self.pipeline.pipeline.remove(self.volume)
        self.pipeline.pipeline.remove(self.buffersize)
        self.pipeline.pipeline.remove(self.sink)

        self.latency.set_state(Gst.State.NULL)
        self.equalizer.set_state(Gst.State.NULL)
        self.volume.set_state(Gst.State.NULL)
        self.buffersize.set_state(Gst.State.NULL)
        self.sink.set_state(Gst.State.NULL)

        self.latency.unref()
        self.equalizer.unref()
        self.volume.unref()
        self.buffersize.unref()
        self.sink.unref()

        self.latency = None
        self.equalizer = None
        self.sink = None
        self.tee = None

        self.attached = False
