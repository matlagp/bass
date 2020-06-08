#!/usr/bin/env python

import socket
import time

s = socket.socket()
s.bind(('localhost', 2137))
s.listen(5)

conn, address = s.accept()  # Establish connection with client.
try:
    start_time = time.monotonic()
    data = 0
    while True:
        data += len(conn.recv(1024))
        if (time.monotonic() - start_time) >= 1:
            start_time = time.monotonic()
            print(data)
            data = 0

finally:
    conn.close()
