import serial
import argparse
import pendulum

parser = argparse.ArgumentParser(
                    prog='ledclkctrl',
                    description='Configure/set the clock')

parser.add_argument('-s', '--set', nargs='?', type=int, default=-1, help="Set the current time to the unix time in seconds. Defaults to system clock")
parser.add_argument('-b', '--brightness', type=int, help="Set the max brightness, between 0 and 255 inclusive.")
parser.add_argument('-d', '--device', default='/dev/tty.usbmodem10832401')

args = parser.parse_args()

if args.set == None:
    now = pendulum.now()
    args.set = round(now.int_timestamp + now.utcoffset().total_seconds())

with serial.Serial(args.device) as ser:
    if args.set != -1:
        print("Setting time")
        ser.write(f"T{args.set}\n".encode('utf8'))
        print(ser.readline().decode())
    if args.brightness != None:
        print("Setting brightness")
        ser.write(f"B{args.brightness}\n".encode('utf8'))
        print(ser.readline().decode())
    if args.set == -1 and args.brightness == None:
        print("Set time:")
        ser.write(b"G")
        print(ser.readline().decode())
