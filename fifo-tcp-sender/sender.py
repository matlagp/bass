#!/usr/bin/env python

import socket
import time

HOST = '192.168.1.31'
PORT = 2137

try:
    with open('/tmp/bassfifo', 'rb') as file:
        s = socket.socket()
        s.connect((HOST, PORT))
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
