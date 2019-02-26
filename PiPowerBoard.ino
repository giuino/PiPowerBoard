/*
PiPowerboard Attiny Controller

This sketch runs on an Attiny85 to check and actively control
a Raspberry car power module

Monitors:
Battery charge (Voltage)
Raspberry status (Off/On/StdBy/FirstStart)(0/1/2/3)
Ignition key status

Do:
Fire up Power board (and Raspberry) if Off on KeyOn 
Shut down Power Board after Raspberry as shut down to save juice

Connections:
P0 SDA
P1 Enable/disable Buck Board
P2 SCL
P3 Input from Key on
P5 Battery charge (analog 0)


*/
#include <TinyWireS.h>


#define I2C_SLAVE_ADDRESS 0x26          // the Attiny 7-bit address
const long poolTime = 500;              //Interval between sensors readings
const long buckDelay =120000;           //Buck converter shutdown delay from Raspberry timer "shutdown command" (in milliseconds)


int battCharge=0;                       //Battery raw adc start value
int PiStatus=0;                         //Pi Status start value
int ignStatus=0;                        //Ignition start value
int buckEnable=false;                   //Buck converter start status

unsigned long previousBuckMillis = 0;
unsigned long previousPoolMillis = 0;


namespace VARS
{
  /*
   * Register map:
   * 0x00 - ignStatus
   * 0x01 - battChargeL
   * 0x02 - battChargeH
   * 0x03 - PiStatus
   *
   * Total size: 4
   */
  const byte reg_size = 4;
  volatile uint8_t i2c_regs[reg_size];
  
  volatile byte reg_position;
}


namespace I2C{
  
  void requestEvent()
  {
    //Send the value on the current register position
    TinyWireS.send(VARS::i2c_regs[VARS::reg_position]);
    // Increment the reg position on each read, and loop back to zero
    VARS::reg_position++;
    if (VARS::reg_position >= VARS::reg_size)
    {
        VARS::reg_position = 0;
    }
  }


  void receiveEvent(uint8_t howMany)
  {
    if (howMany < 1)
    {
        return;// Sanity-check
    }

    VARS::reg_position = TinyWireS.receive();
    howMany--;
    if (!howMany)
    {
        return;// This write was only to set the buffer for next read
    }
    
    while(howMany--)
    {
        //Store the received data in the currently selected register
        VARS::i2c_regs[VARS::reg_position] = TinyWireS.receive();
        
        //Proceed to the next register
        VARS::reg_position++;
        if (VARS::reg_position >= VARS::reg_size)
        {
            VARS::reg_position = 0;
        }
    }

    
  }
}

void setup() {
  
    pinMode(3, INPUT_PULLUP);           //Ignition Input
    pinMode(1, OUTPUT);                 //Buck Converter Enabling pin

    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onReceive(I2C::receiveEvent);
    TinyWireS.onRequest(I2C::requestEvent);

    
    
}

void loop() {
  
   unsigned long currentMillis = millis();

   //routine to read sensors (battery voltage and ignition pin) 
      if (currentMillis - previousPoolMillis >= poolTime) {
            previousPoolMillis = currentMillis;
            battCharge = ((analogRead(A2)+battCharge)/2);     //Read Analog pin 2, sums the previous value and divide by 2
            VARS::i2c_regs[0x01] = lowByte(battCharge);       //Write Battery voltage in memory (LowByte)
            VARS::i2c_regs[0x02] = highByte(battCharge);      //Write Battery voltage in memory (HighByte)
            if (digitalRead(3)==true){                        //Read Ignition
            VARS::i2c_regs[0x00] = 0x00;                      //Write Ignition status in memory 
            ignStatus=0;
            }
            else {
            VARS::i2c_regs[0x00] = 0x01;
            ignStatus=1;
            }
        }

        PiStatus=VARS::i2c_regs[0x03];                        //Read Pi Status (from raspberry)


    //Cases and combinations:

    if (ignStatus==1 && PiStatus==0){         //Restart from regular shutdown
              previousBuckMillis=currentMillis;          
              buckEnable=true;
    }
    
    else if (ignStatus==0 && PiStatus==0){    // Commanded shutdown via Ignition pin
            if (currentMillis - previousBuckMillis >= buckDelay) {
            buckEnable=false;
            }
        }
        else {                                //Every other case, buck enabled
              previousBuckMillis=currentMillis;          
            buckEnable=true;

        }

    digitalWrite(1,buckEnable);               //Switch the Buck converter on or off

      TinyWireS_stop_check();

}




