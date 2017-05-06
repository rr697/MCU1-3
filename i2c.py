import smbus
import time

bus = smbus.SMBUS(1)
DEVICE_ADDRESS = 0x1C
pin = 0x80

def writeNumber(DEVICE_ADDRESS, pin, address):
	bus.write_byte_data(DEVICE_ADDRESS, pin, address)
	return -1

while True:
	address = input("Enter Address:")
	writeNumber(DEVICE_ADDRESS, pin, address)
time.sleep(0.1)