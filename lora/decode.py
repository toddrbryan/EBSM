#!/usr/bin/env python3

with open('test.bin', 'rb') as data:
	buf = data.read();

	lon = int.from_bytes(buf[0:3], byteorder='big')
	lat = int.from_bytes(buf[3:6], byteorder='big')

	print(lon, lat);
