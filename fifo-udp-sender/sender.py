#!/usr/bin/env python

import socket
import time

IP = '192.168.1.31'
PORT = 2137

try:
    with open('/tmp/bassfifo', 'rb') as file:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # s.connect(('192.168.11.31', 2137))
        # s.connect(('localhost', 2137))

        start_time = time.monotonic()

        while time.monotonic() - start_time < 1:
            file.read(1)

        start_time = time.monotonic()
        data = 0

        while True:
            d = file.read(128)
            data += len(d)

            if len(d) > 0:
                s.sendto(d, (IP, PORT))

            if (time.monotonic() - start_time) >= 1:
                start_time = time.monotonic()
                print(data)
                data = 0

finally:
    file.close()
