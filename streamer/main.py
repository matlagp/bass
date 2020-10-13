from threading import Thread
import gi
import os

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

Gst.init(None)

class Pipeline(object):
    def __init__(self):
        self.pipeline = Gst.Pipeline()

        self.alsasrc = Gst.ElementFactory.make('alsasrc')
        self.audioconvert = Gst.ElementFactory.make('audioconvert')
        self.equalizer = Gst.ElementFactory.make('equalizer-3bands')
        self.autoaudiosink = Gst.ElementFactory.make('autoaudiosink')

        self.pipeline.add(self.alsasrc)
        self.pipeline.add(self.audioconvert)
        self.pipeline.add(self.equalizer)
        self.pipeline.add(self.autoaudiosink)

        self.alsasrc.link(self.audioconvert)
        self.audioconvert.link(self.equalizer)
        self.equalizer.link(self.autoaudiosink)

p = Pipeline()
p.alsasrc.set_property('device', 'hw:1,1,0')
p.pipeline.set_state(Gst.State.PLAYING)

main_loop = GLib.MainLoop()
thread = Thread(target=main_loop.run)

thread.start()

def equalize():
    import time

    while True:
        print('bass')
        p.equalizer.set_property('band0', 12)
        p.equalizer.set_property('band1', 0)
        p.equalizer.set_property('band2', 0)
        time.sleep(1)

        print('mid')
        p.equalizer.set_property('band0', 0)
        p.equalizer.set_property('band1', 12)
        p.equalizer.set_property('band2', 0)
        time.sleep(1)

        print('trebble')
        p.equalizer.set_property('band0', 0)
        p.equalizer.set_property('band1', 0)
        p.equalizer.set_property('band2', 12)
        time.sleep(1)

thread2 = Thread(target=equalize)

thread2.start()
