import cv2
import mediapipe as mp
import time
import math as math
from hand_detect import HandTrackingDynamic
# from smbus import SMBus
import serial
from time import sleep
import RPi.GPIO as GPIO
import neopixel
import board

neopixel_pin = board.D18
num_pixels = 256

nps = neopixel.NeoPixel(neopixel_pin, num_pixels)

ctime=0
ptime=0

cap = cv2.VideoCapture(0)
detector = HandTrackingDynamic()

camera_xmax = 640
camera_ymax = 480
crop_amount = 100

cap.set(cv2.CAP_PROP_FRAME_WIDTH, camera_xmax)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, camera_ymax)

# i2c = SMBus(1)
# position_address = 0x42
uart = serial.Serial("/dev/ttyUSB0", 115200)

if not cap.isOpened():
    print("Cannot open camera")
    exit()

claw_queue = []
queue_length = 5

neopixel_id = 0
while True:

    # if uart.in_waiting > 0:
    #     print(f"Incoming: {uart.read(10)}")

    ret, frame = cap.read()

    cropped_frame = frame[crop_amount:camera_ymax-crop_amount, crop_amount:camera_xmax-crop_amount]

    frame = detector.findFingers(frame, draw=False)
    lmsList, bbox = detector.findPosition(frame, draw=False)
    nps[neopixel_id] = (0, 0, 0)

    # 0000 0 claw_down claw_up
    claw_command = 0x00
    fingers = detector.findFingerUp()
    if fingers == [0, 0, 0, 0]:
        # If all fingers are down
        claw_command += 0x01
    # elif fingers == [1, 1, 1, 1]:
    #     # If all fingers are up
    #     claw_command += 0x02
    # elif fingers == [1, 0, 0, 0]:
    #     # Pointing up
    #     claw_command += 0x04
    # elif fingers == [0, 1, 1, 1]:
    #     # Pointing down
    #     claw_command += 0x08

    claw_queue.append(claw_command)
    if len(claw_queue) > queue_length:
        claw_queue = claw_queue[1:]
    real_claw_command = int(claw_queue==[1]*queue_length)

    x_mid = 0x7F
    y_mid = 0x7F
    if len(bbox) > 0:
        x_mid = int(255*((bbox[0]+bbox[2])/(2*(camera_xmax-crop_amount))))
        y_mid = int(255*((bbox[1]+bbox[3])/(2*(camera_ymax-crop_amount))))
        neopixel_id = (15-round(x_mid/16))*16 + (round(y_mid/16) if (15-round(x_mid/16))%2==0 else 16-round(y_mid/16))
        # print(f"Sending position: {(x_mid,y_mid)} => (Pixel @ {(int(x_mid/16), int(y_mid/16))}, index {neopixel_id}")
        if real_claw_command == 1:
            nps[neopixel_id] = (128, 0, 0)
        elif claw_command == 1:
            nps[neopixel_id] = (0, 0, 128)
        else:
            nps[neopixel_id] = (0, 128, 0)


        

    try:
        data = bytearray([0x73,int(x_mid), int(y_mid), int(real_claw_command),0x65])
        # i2c.write_i2c_block_data(position_address, 0, data)
        print(f"Sending message: x_pos: {data[1]}, y_pos: {data[2]}, claw: {data[3]}")
        uart.write(data)
    except Exception as e:
        print(e)


    ctime = time.time()
    fps = 1/(ctime-ptime)
    ptime = ctime

    # cv2.putText(frame, str(int(fps)), (10,70), cv2.FONT_HERSHEY_PLAIN,3,(255,0,255),3)

    #gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    # cv2.imshow('frame', frame)
    cv2.waitKey(1)
