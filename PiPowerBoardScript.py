import smbus
from subprocess import call
import time
import threading

DEV_ADDR = 0x26

#pistatus
#0=OFF
#1=ON
#2=STANDBY
#3=FIRST START
#
#
shutdownDelay=10.0
bus = smbus.SMBus(1)
reads = 0
errs = 0
pistatus = 3
delay = 60

def shutdownTimer():
    call('(shutdown -P +1)', shell=True)
    bus.write_byte_data(DEV_ADDR, 3,0)
    pistatus = 0

t = threading.Timer(shutdownDelay, shutdownTimer)
    
while True:
    reads += 1
    try:
        ignSwitch = bus.read_byte_data(DEV_ADDR, 0)
        battLval = bus.read_byte_data(DEV_ADDR, 1)
        battHval = bus.read_byte_data(DEV_ADDR, 2)
        piattinystatus = bus.read_byte_data(DEV_ADDR, 3)
        battval = (battHval<<8 | battLval)/29.55-1.55
        print("Ign Switch [%d]; no. of reads [%s]; no. of errors [%s]" % (ignSwitch, reads, errs))
        print("Batt Volt  [%.1f]" % (battval))
        print("Pi Status  [%d]" % (piattinystatus))
        if(ignSwitch == 1 and pistatus == 2):#standby to run
            #call('(vcgencmd display_power 1)', shell=True)#uncomment to power-on display when turning ignition on
            call('(shutdown -c)', shell=True)
            bus.write_byte_data(DEV_ADDR, 3,1)#sends pistatus to attiny
            pistatus = 1
            t.cancel()
        elif(ignSwitch == 0 and pistatus == 1):#run to standby
            #call('(vcgencmd display_power 0)', shell=True)#uncomment to power-off display when turning ignition off
            t.start()
            #call('(shutdown -P +1)', shell=True)
            bus.write_byte_data(DEV_ADDR, 3,2)#sends pistatus to attiny
            pistatus = 2
        elif(ignSwitch == 1 and pistatus == 3):#first start to run
            bus.write_byte_data(DEV_ADDR, 3,1)#sends pistatus to attiny
            pistatus = 1
    except Exception as ex:
        errs += 1
        print("Exception [%s]" % (ex))
    time.sleep(2)
