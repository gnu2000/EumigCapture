#!/usr/bin/env python3
import serial
import time
from picamera2 import Picamera2, Preview
from libcamera import Transform

picam2 = Picamera2()
controls = {"ExposureTime": 10000, "AnalogueGain": 1.0}
camera_config = picam2.create_still_configuration(main={"size": (4056, 3040)}, lores={"size": (640, 480)}, display="lores", controls=controls, transform=Transform(vflip=1))
picam2.configure(camera_config)
picam2.set_controls({"AwbEnable": False, "AeEnable": False})
picam2.set_controls({'ColourGains': (2.58, 2.03)})

picam2.start_preview(Preview.QTGL)
picam2.start()

line = ""
prevline = ""
capname = input("Please enter a name for this capture: ")
filetype = input("Select capture format:    1. RAW    2. JPG: ")
framestring = input("Please enter number of frames to capture: ")
frames = int(framestring)
capture = "<capture, " + str(frames) + ">"
command = capture.encode("ascii")
print(f"Capturing {frames} frames.")
i = 1

if __name__ == '__main__':
    print(f"Connecting to Arduino...")
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=5)
    ser.reset_input_buffer()
    ser.reset_output_buffer()

#picam2.stop_preview()

# Give the arduino time to restart before sending the command
time.sleep(2)
print(f"Sending capture command")
time.sleep(1)
ser.write(command)
print(f"Sent command: {capture}")

while line != "Done!":
  time.sleep(0.05)
  if ser.in_waiting > 0:
    line = ser.readline().decode('utf-8').rstrip()
  if line == prevline:
    line = ""
  if line !="":
    print(f"Received: {line}")
  if line == "trig":
    print(f"Getting frame {i}...")
    if filetype == "1":
      picam2.capture_file(f"frames/{capname}_{i : 07d}.dng", 'raw') # Gets a RAW
    elif filetype == "2": 
      picam2.capture_file(f"frames/{capname}_{i : 07d}.jpg") # Gets a jpg
    i += 1
  prevline = line
i = i - 1
print(f"Received stop command, {i} frames captured.")
ser.write(b"c\n")
