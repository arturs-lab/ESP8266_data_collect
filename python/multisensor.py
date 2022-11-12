import dht
import onewire, ds18x20
import network
import socket
import time
import machine
from machine import ADC, SoftI2C
#from machine import WDT
from settings import *
import sh1106


def measure_battery(my_adc):
    # using voltage divider to reduce 5v to 2.5v
    # also, we're getting an int, not a voltage
    # so adjust the result
    # also, seems that ADC is not linear.
    # 2.5v -> 412
    # 5.0v -> 806
    # 7.5v -> 1024 probably off the scale
    # the most that should be measured is 3.3V*2
    my_voltage = my_adc.read() / 164
    #my_voltage = ( my_adc.read() / 159.4 ) + 0.06900878294
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
    # init WDT
    # enable it with a timeout of 2s
    # alas this does not work on ESP8266
    # Can't configure timeout and the default one is very short
    #wdt = WDT()
    #wdt.feed()

    i2c = SoftI2C(
    scl=machine.Pin(16),
    sda=machine.Pin(14))
    oled_width = 128
    oled_height = 64
    oled = sh1106.SH1106_I2C(oled_width, oled_height, i2c)

    my_adc = ADC(0)

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
    if not sta_if.active():
        print("Failed to activate WLAN")
        oled.text('Failed to', 0, 0)
        oled.text('activate WLAN', 0, 10)
        oled.show()
        quit()
    sta_if.connect(ssid, pw)
    i=5
    while not sta_if.isconnected():
        time.sleep_ms(100)
        i -= 1
        if i==0:
            print('Failed to connect')
            oled.text('Failed to connect', 0, 0)
            oled.show()
            quit()

    while True:
        try:
            addr = socket.getaddrinfo(host, 80)[0][-1]
        except:
            print("Failed to get addr for %s. Will take a nap and try again" % host)
            oled.text('Failed to get addr for %s' % host, 0, 0)
            oled.text('Will take a nap and try again', 0, 10)
            oled.show()
            # According to tcpdump it may take ESP about 4 minutes to respond,
            # long after it printed error message and exited program
            time.sleep(60)
        else:
            break
    my_ip=sta_if.ifconfig()[0]
    _,_,_,my_id=my_ip.split('.',4)

    while True:
        # turn on led
        my_led.value(0)
        
        # clear oled
        oled.fill(0)

        # do this ahead of the time because it takes 750ms for conversion to finish
        ds.convert_temp()

        # meanwhile measure battery voltage
        # tie it to sensor 0 for now
        my_id_s=str(my_id) + '0'
        my_voltage=measure_battery(my_adc)
        #print('battery:',my_voltage)
        send_data(addr, socket, my_id_s, "b", my_voltage)
        oled.text('Battery:   %.2f' % my_voltage, 0, 0)

        # and process DHT sensor
        my_dht.measure()
        my_temp=my_dht.temperature()
        #print('temperature:',my_temp)
        my_humid=my_dht.humidity()
        #print('humidity:',my_humid)
        my_id_s=str(my_id) + '0'
        send_data(addr, socket, my_id_s, "t", my_temp)
        send_data(addr, socket, my_id_s, "h", my_humid)
        oled.text('Humidity: %.2f' % my_humid, 0, 10)
        oled.text('Temp:     %.2f' % my_temp, 0, 20)

        # shouldn't need this now
        #time.sleep_ms(1000)
        i=1
        for sensor in sensors:
            my_temp=ds.read_temp(sensor)
            #print('temperature:',my_temp)
            my_id_s=str(my_id) + str(i)
            send_data(addr, socket, my_id_s, "t", my_temp)
            oled.text('Temp%d:    %.2f' % (i, my_temp), 0, 20+i*10)
            ++i
        oled.show()
        # turn off led before going to sleep
        my_led.value(1)
        # and make sure WDT doesn't reset node
        #wdt.feed()
        time.sleep_ms(60*1000-1000)

if __name__ == '__main__':
    collect_data()



