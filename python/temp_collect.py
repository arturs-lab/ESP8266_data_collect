import network
import socket
import time
import machine
import onewire, ds18x20
from settings import *

def collect_data():

    # make sure access point is off
    ap_if = network.WLAN(network.AP_IF)
    ap_if.active(False)

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
        ds.convert_temp()
        time.sleep_ms(1000)
        addr = socket.getaddrinfo(host, 80)[0][-1]
        s = socket.socket()
        s.connect(addr)
        i=1
        for sensor in sensors:
            my_temp=ds.read_temp(sensor)
            #print('temperature:',my_temp)
            my_id_s=str(my_id) + str(i)
            s.send(bytes('GET %s?id=%s&type=t&val=%s HTTP/1.0\r\nHost: %s\r\n\r\n' % (path, my_id_s, my_temp, host), 'utf8'))
            ++i
            while True:
                data = s.recv(100)
                if data:
                    print(str(data, 'utf8'), end='')
                else:
                    break
        s.close()
        time.sleep_ms(60*1000)

if __name__ == '__main__':
    collect_data()