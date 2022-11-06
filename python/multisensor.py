import dht
import onewire, ds18x20
import network
import socket
import time
import machine
from machine import ADC
from settings import *

def measure_battery():
    my_bat = ADC(0)
    # using voltage divider to reduce 5v to 2.5v
    # also, we're getting an int, not a voltage
    # so adjust the result
    # also, seems that ADC is not linear.
    # 2.5V -> 166
    # 5.0V -> 164
    # 7.5V -> 136
    # though the most that should be measured is 3.3V*2
    my_voltage = my_bat.read() / 164
    return my_voltage

def send_data(addr, socket, my_id_s, my_type, my_value):
    s = socket.socket()
    s.connect(addr)
    s.send(bytes('GET %s?id=%s&type=%s&val=%s HTTP/1.0\r\nHost: %s\r\n\r\n' % (path, my_id_s, my_type, my_value, host), 'utf8'))
    while True:
        data = s.recv(100)
        if data:
            #print(str(data, 'utf8'), end='')
            pass
        else:
            break
    s.close()

def collect_data():

    my_led = machine.Pin(2, machine.Pin.OUT)
    my_led.value(1)

    # make sure access point is off
    ap_if = network.WLAN(network.AP_IF)
    ap_if.active(False)

    # the device is on D6
    my_dht = dht.DHT22(machine.Pin(12, machine.Pin.PULL_UP))

    # the device is on D7
    dat = machine.Pin(13, machine.Pin.PULL_UP)

    # create the onewire object
    ds = ds18x20.DS18X20(onewire.OneWire(dat))

    # scan for devices on the bus
    sensors = ds.scan()
    #print('found devices:', sensors)

    sta_if = network.WLAN(network.STA_IF)
    sta_if.active(True)
    sta_if.connect(ssid, pw)
    #time.sleep_ms(1000)
    i=4
    while not sta_if.isconnected():
        time.sleep_ms(100)
        --i
        if i==0:
            print('Failed to connect')
            break
    # need to do this to get IP when cold boot
    addr = socket.getaddrinfo(host, 80)[0][-1]
    my_ip=sta_if.ifconfig()[0]
    _,_,_,my_id=my_ip.split('.',4)

    while True:
        # turn on led
        my_led.value(0)

        # do this ahead of the time because it takes 750ms for conversion to finish
        ds.convert_temp()

        # meanwhile measure battery voltage
        # tie it to sensor 0 for now
        my_id_s=str(my_id) + '0'
        my_bat=measure_battery()
        print('battery:',my_bat)
        send_data(addr, socket, my_id_s, "b", my_bat)

        # and process DHT sensor
        my_dht.measure()
        my_temp=my_dht.temperature()
        print('temperature:',my_temp)
        my_humid=my_dht.humidity()
        print('humidity:',my_humid)
        my_id_s=str(my_id) + '0'
        send_data(addr, socket, my_id_s, "t", my_temp)
        send_data(addr, socket, my_id_s, "h", my_humid)

        # shouldn't need this now
        #time.sleep_ms(1000)
        i=1
        for sensor in sensors:
            my_temp=ds.read_temp(sensor)
            print('temperature:',my_temp)
            my_id_s=str(my_id) + str(i)
            send_data(addr, socket, my_id_s, "t", my_temp)
            ++i
        # turn off led before going to sleep
        my_led.value(1)
        time.sleep_ms(60*1000-1000)

if __name__ == '__main__':
    collect_data()


