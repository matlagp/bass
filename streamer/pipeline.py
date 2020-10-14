import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

class Pipeline(object):
    def __init__(self):
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
