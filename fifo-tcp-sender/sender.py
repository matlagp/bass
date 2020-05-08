#!/usr/bin/env python

import socket
import time

try:
    with open('/tmp/bassfifo', 'rb', 0) as file:
        s = socket.socket()
        s.connect(('localhost', 2137))

        start_time = time.monotonic()
        data = 0

        while True:
            d = file.read(1)
            data += len(d)
            s.send(d)

            if (time.monotonic() - start_time) >= 1:
                start_time = time.monotonic()
                print(data)
                data = 0

finally:
    file.close()
