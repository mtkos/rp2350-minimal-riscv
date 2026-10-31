#!/usr/bin/python3

from serial import Serial

with Serial('/dev/ttyUSB0', 921600) as s:
    for n in range(100):
        l = s.read(8)
        for i in range(0, 8, 8):
            print(l[i:i + 8].hex())
