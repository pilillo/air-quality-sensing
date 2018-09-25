#!/usr/bin/python
# -*- coding: UTF-8 -*-

import serial, time, struct

ser = serial.Serial()
ser.port = "/dev/cu.wchusbserial14210" 
ser.baudrate = 9600

ser.open()
ser.flushInput()

byte, lastbyte = "\x00", "\x00"


def process_reading(reading):
    # Decode the packet - big endian, 2 shorts for pm2.5 and pm10, 2 reserved bytes, checksum, message tail
    #values = struct.unpack('>hhxxcc', sentence) 
    # decode as little endian
    # http://ecksteinimg.de/Datasheet/SDS011%20laser%20PM2.5%20sensor%20specification-V1.3.pdf
    # https://www.snip2code.com/Snippet/1048974/SDS011-dust-sensor-reading
    #values = struct.unpack("<hhxxcc", sentence) 
    values = struct.unpack('<HHxxBBB', reading[2:])
    return values[0]/10.0, values[1]/10.0

def collect_measurement():
    byte = 0
    while byte != "\xaa":
        byte = ser.read(size=1)
    d = ser.read(size=10)
    # if we got a valid packet header
    if d[0] == "\xc0":
        return process_reading(byte + d)

def collect(num_measurements):
    for i in range(num_measurements):
        pm_25, pm_10 = collect_measurement()
        print "PM 2.5:", pm_25, "μg/m^3  PM 10:", pm_10, "μg/m^3"

collect(1)