The Pi Power Board is a tiny add-on board for the Raspberry Pi  

It consists of a DS3231 RTC, an Attiny85, a 3A Buck converter and a fan  
It permits to power on and off the Raspberry via an extra 12v wire, let alone the 12v an GND wires  

it's born to ease the install of a Raspberry Pi in a car (dashboard, media player, carPc)  

To install and have a fully working board we have to fulfill 4 steps:  
Enable I2C communications  
Check the I2C, Rtc and Attiny are working  
Enable the Real Time Clock  
Install the PiPowerBoardScript as an autostart script  

Let's start from the I2C communications:  
Open the Menu->Preferences->Raspberry Pi Configuration  
Go to the Interfaces tab and enable I2C, "OK" and reboot  
on Reboot open a terminal and install:  
sudo apt-get install -y python-smbus i2c-tools

after install try (on terminal):  
i2cdetect -y 1

You Raspberry should show a matrix with two numbers, 68 and 26  
If they show we're on track!  
Now, for the RTC config I found a very well written page I should point:  
https://www.raspberrypi-spy.co.uk/2015/05/adding-a-ds3231-real-time-clock-to-the-raspberry-pi/


PiPowerboard Script install:  
download the script "PiPowerBoardScript.py" and save it in your "home/pi" folder  
open a terminal and edit your "/etc/xdg/lxsession/LXDE-pi/autostart":  
e.g. sudo nano /etc/xdg/lxsession/LXDE-pi/autostart  
paste this line "/usr/bin/python /home/pi/PiPowerBoardScript.py"  
after the "@pcmanfm..." line  

save and reboot

Now try a commanded power on and shutdown, everything should be ok!
