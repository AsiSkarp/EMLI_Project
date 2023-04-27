# Embedded Linux (EMLI)
# University of Southern Denmark
# Raspberry Pico plant watering controller
# Copyright (c) Kjeld Jensen <kjen@sdu.dk> <kj@kjen.dk>
# 2023-04-19, KJ, First version

from machine import Pin, ADC, UART
import utime
from sys import stdin
import uselect

pump_control = Pin(16, Pin.OUT)
pump_water_alarm = Pin(13, Pin.IN)
plant_water_alarm = Pin(9, Pin.IN)

moisture_sensor_pin = Pin(26, mode=Pin.IN)
moisture_sensor = ADC(moisture_sensor_pin)

photo_resistor_pin = Pin(27, mode=Pin.IN)
light_sensor = ADC(photo_resistor_pin)

led_builtin = machine.Pin(25, machine.Pin.OUT)
uart = machine.UART(0, 115200)

init_time = utime.time()
hour_timer = utime.time()
isWatered = False


def moisture():
    return moisture_sensor.read_u16() / 655.36


def light():
    return light_sensor.read_u16() / 655.36


def pump_request():
    result = False
    select_result = uselect.select([stdin], [], [], 0)
    while select_result[0]:
        ch = stdin.read(1)
        if ch == "p":
            result = True
        select_result = uselect.select([stdin], [], [], 0)
    return result


def alarm_guard():
    return plant_water_alarm.value() == 0 and pump_water_alarm.value() == 1


def run_pump():
    global isWatered

    pump_control.high()
    utime.sleep(3)
    pump_control.low()
    isWatered = True


def reset_hour_timer():
    global hour_timer
    hour_timer = utime.time()


def watering_interval():
    interval = utime.time()
    global init_time
    diff = interval - init_time
    hour_diff = interval - hour_timer
    hour = 3600  # 3600 seconds
    day = 43200  # 43200 seconds

    if hour_diff > hour:
        isWatered = False

    if hour_diff > hour and moisture() < 100 and alarm_guard():
        run_pump()
        reset_hour_timer()

    if diff >= day and alarm_guard():
        run_pump()
        init_time = utime.time()
        reset_hour_timer()


while True:
    pump_control.low()
    led_builtin.toggle()
    watering_interval()
    utime.sleep(1)
    if pump_request() and not isWatered and alarm_guard():
        run_pump()
        reset_hour_timer()
    else:
        utime.sleep(1)
    print(
        "%d,%d,%.0f,%.0f"
        % (plant_water_alarm.value(), pump_water_alarm.value(), moisture(), light())
    )
