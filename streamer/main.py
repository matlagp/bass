from threading import Thread
import gi
import os

from mqtt import MQTTClient
from pipeline import Pipeline
from node import Node

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

Gst.init(None)
p = Pipeline()
p.src.set_property('device', 'hw:1,1,0')
p.play()

Gst.debug_bin_to_dot_file(p.pipeline, Gst.DebugGraphDetails.ALL, "pipeline")

main_loop = GLib.MainLoop()
thread = Thread(target=main_loop.run)

thread.start()

def manipulate():
    node = Node(p, '127.0.0.1', 10000)

thread2 = Thread(target=manipulate)
thread2.start()

mqtt_client = MQTTClient()
mqtt_client.start()
