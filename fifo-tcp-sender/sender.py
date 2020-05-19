#!/usr/bin/env python

import socket
import time

try:
    with open('/tmp/bassfifo', 'rb') as file:
        s = socket.socket()
        s.connect(('192.168.11.113', 2137))
        #s.connect(('localhost', 2137))

        start_time = time.monotonic()

        while time.monotonic() - start_time < 1:
            file.read(1)

        start_time = time.monotonic()
        data = 0

        while True:
            d = file.read(128)
            data += len(d)
            s.sendall(d)

            if (time.monotonic() - start_time) >= 1:
                start_time = time.monotonic()
                print(data)
                data = 0

finally:
    file.close()
