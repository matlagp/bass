from threading import Thread
import gi
import os

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

Gst.init(None)

class Pipeline(object):
    def __init__(self):
        self.pipeline = Gst.Pipeline()

        self.src = Gst.ElementFactory.make('alsasrc')
        self.audioconvert = Gst.ElementFactory.make('audioconvert')
        self.tee = Gst.ElementFactory.make('tee')
        self.fakequeue = Gst.ElementFactory.make('queue')
        self.fakesink = Gst.ElementFactory.make('fakesink')

        self.pipeline.add(self.src)
        self.pipeline.add(self.audioconvert)
        self.pipeline.add(self.tee)
        self.pipeline.add(self.fakequeue)
        self.pipeline.add(self.fakesink)

        self.src.link(self.audioconvert)
        self.audioconvert.link(self.tee)
        self.tee.get_request_pad('src_%u').link(self.fakequeue.get_static_pad('sink'))
        self.fakequeue.link(self.fakesink)

        self.fakesink.set_property('sync', False)

    def pause(self):
        return self.pipeline.set_state(Gst.State.PAUSED)

    def play(self):
        return self.pipeline.set_state(Gst.State.PLAYING)


class NodeBin(object):
    def __init__(self, pipeline):
        self.pipeline = pipeline
        self.latency = Gst.ElementFactory.make('queue')
        self.equalizer = Gst.ElementFactory.make('equalizer-3bands')
        self.sink = Gst.ElementFactory.make('autoaudiosink')

        self.pipeline.pipeline.add(self.latency)
        self.pipeline.pipeline.add(self.equalizer)
        self.pipeline.pipeline.add(self.sink)

        self.latency.link(self.equalizer)
        self.equalizer.link(self.sink)

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


p = Pipeline()

p.src.set_property('device', 'hw:1,1,0')

p.play()

Gst.debug_bin_to_dot_file(p.pipeline, Gst.DebugGraphDetails.ALL, "pipeline")

main_loop = GLib.MainLoop()
thread = Thread(target=main_loop.run)

thread.start()

def manipulate():
    import time

    while True:
        node_bin = NodeBin(p)
        time.sleep(1)
        Gst.debug_bin_to_dot_file(p.pipeline, Gst.DebugGraphDetails.ALL, "pipeline_attach")

        node_bin.detach()
        node_bin = None
        time.sleep(1)
        Gst.debug_bin_to_dot_file(p.pipeline, Gst.DebugGraphDetails.ALL, "pipeline_detach")

thread2 = Thread(target=manipulate)

thread2.start()
