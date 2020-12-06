# Streaming module

Streams audio from an ALSA loopback device to multiple udp clients.

New nodes connect via MQTT (the same mechanism is present in the server module).

## Usage

1. Create an alsa loopback device

```sh
sudo modprobe snd-aloop
```

1. Send some audio to the loopback device (hw:1,0,0)

  - mopidy (mopidy.conf)

  ```
  [audio]
  output = audioresample ! audioconvert ! audio/x-raw,rate=44100,channels=2,format=S16LE ! alsasink device=hw:1,0,0
  ```

   - gstreamer test audio

  ```sh
  gst-launch-1.0 -v audiotestsrc ! audioresample ! audioconvert ! audio/x-raw,rate=44100,channels=2,format=S16LE ! alsasink device=hw:1,0,0
  ```

1. Run an MQTT server

Described in the server README

1. Run the server module

1. Install dependencies (gstreamer and paho-mqtt)

``` sh
pip install paho-mqtt
```

1. Run this module

``` sh
python main.py
```

1. (optional) Add localhost as a node

Run in the MQTT docker container

``` sh
mosquitto_pub -m '127.0.0.1' -t /nodes/00000000/state -q 1 -r
```

1. (optional) Redirect the udp stream to autoaudiosink

``` sh
gst-launch-1.0 -v udpsrc port=2137 ! rawaudioparse use-sink-caps=false format=pcm pcm-format=s16le sample-rate=44100 num-channels=2 ! queue ! audioconvert ! audioresample ! autoaudiosink
```
