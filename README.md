# ESP STP AP

In this project I used ESP8266 as access point and as a station. Once ESP is powered on it will start working in access point mode so the user can connect to the ESP network. Then, the user will go to a WiFi interface where he is asked to write the SSID (name) and password of the router, then the ESP will store those values in the FLASH memory and will use it to connect to the router and switch to station mode.
<br>

# Picture of the interface
<br>

<p align="center">
<img src="https://user-images.githubusercontent.com/85786699/128621536-988ddaf1-7875-4825-a48f-b7d0b5d885d1.png"></p>

<br>

# Sending Data in AP Mode

I used javascript to generate a JSON file and send it to the ESP which will receive, decode, and store the values sent by the user. 

<br>

# Storing SSID and Password

Storing WiFi settings in flash memory of the ESP has some advantages and disadvantages:
- The ESP will remember and try to connect to the network automatically after restarting the ESP and will run in AP mode only if the network is not available.
- It will slower the ESP8266.
- It will be remebered even if you reset the ESP.
<br>

# Working in STA mode

After connecting to the router, the user will be able to use the robot movement interface to control the movement of the robot.

<br>

# Video


https://user-images.githubusercontent.com/85786699/128621680-bc8c38fe-60d9-42ba-a0f1-4cc4921435d1.mp4

<br>

**Note: I had a problem in my local server in configuration file of PHP, thats why the data has been received by GET method but the output did not appear probably, you can see the output in my previous repositry: https://github.com/Yaser-alshoibi/esp_connected_to_Database_GET**
