from threading import Thread
import gi
import os

from mqtt import MQTTClient
from pipeline import Pipeline

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

Gst.init(None)
p = Pipeline()
p.src.set_property('device', 'hw:1,1,0')
p.play()

main_loop = GLib.MainLoop()
thread = Thread(target=main_loop.run)

thread.start()

mqtt_client = MQTTClient(p)
mqtt_client.start()
