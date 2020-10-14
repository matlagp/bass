import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

class Node(object):
    def __init__(self, pipeline, host, port):
        self.pipeline = pipeline
        self.latency = Gst.ElementFactory.make('queue')
        self.equalizer = Gst.ElementFactory.make('equalizer-3bands')
        self.buffersize = Gst.ElementFactory.make('rndbuffersize')
        self.sink = Gst.ElementFactory.make('udpsink')

        self.buffersize.set_property('min', 512)
        self.buffersize.set_property('max', 512)

        self.sink.set_property('host', host)
        self.sink.set_property('port', port)

        self.pipeline.pipeline.add(self.latency)
        self.pipeline.pipeline.add(self.equalizer)
        self.pipeline.pipeline.add(self.buffersize)
        self.pipeline.pipeline.add(self.sink)

        self.latency.link(self.equalizer)
        self.equalizer.link(self.buffersize)
        self.buffersize.link(self.sink)

        self.pipeline.pause()

        self.tee = pipeline.tee.get_request_pad('src_%u')
        self.tee.link(self.latency.get_static_pad('sink'))

        self.pipeline.play()

        print("__init__")

    def detach(self):
        self.pipeline.pause()

        self.tee.unlink(self.latency.get_static_pad('sink'))

        self.pipeline.tee.release_request_pad(self.tee)

        self.pipeline.play()

        self.pipeline.pipeline.remove(self.latency)
        self.pipeline.pipeline.remove(self.equalizer)
        self.pipeline.pipeline.remove(self.sink)

        self.latency.unlink(self.equalizer)
        self.equalizer.unlink(self.sink)

        self.latency.set_state(Gst.State.NULL)
        self.equalizer.set_state(Gst.State.NULL)
        self.sink.set_state(Gst.State.NULL)

        self.latency.unref()
        self.equalizer.unref()
        self.sink.unref()

        self.pipeline = None
        self.latency = None
        self.equalizer = None
        self.sink = None
        self.tee = None

        print("detach")
