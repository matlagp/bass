#!/usr/bin/env python

import socket
import time
import os
import errno


IP = '192.168.0.113'
PORT = 2138

try:
    os.mkfifo('/tmp/bassfifo')
except OSError as oe:
    if oe.errno != errno.EEXIST:
        raise

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    with open('/tmp/bassfifo', 'rb') as file:
        # s.connect(('192.168.11.31', 2137))
        # s.connect(('localhost', 2137))

        # start_time = time.monotonic()

        # while time.monotonic() - start_time < 1:
        #     file.read(1)

        data = 0
        start_time = time.monotonic()

        while True:
            d = file.read(44100)
            data += len(d)

            if len(d) > 0:
                pass
                # print(len(d))
                s.sendto(d, (IP, PORT))
            elif time.monotonic() - start_time > 0.1:
                print(data)
                break
            # else:
                # break
                # print("empty")


            if time.monotonic() - start_time >= 1:
                start_time = time.monotonic()
                print("second: {}".format(data))
                data = 0

