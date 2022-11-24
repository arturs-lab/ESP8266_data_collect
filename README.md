# ESP8266_data_collect
Data collector based on ESP8266 and Micropython

Save contents of php folder on your web server.<br>
Adjust settings in settings.py and save contents of python folder on ESP8266 flash.<br>
Reboot ESP8266.<br>

Also, added code in Arduino for ESP8266-01. This board is so minimal that it does not seem to be able to run micropython. Perhaps it needs a custom compile. Instead, functionality was implemented in Arduino. It includes ability to calibrate individual sensors by sending calibration data in http response. This is preferred over hard-coding calibration for each sensor.
It may be worth researching saving this calibration data in flash so that it is used upon reboot. Currently, calibration is fetched after sending first data set.

# Get averages from httpd logs

script "calculate_average.bash" can be used to get sensor raw reading averages from httpd log entries
