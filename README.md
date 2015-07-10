RaspberryPi-AS3935-BMP085-LCD 4D Sytems Visi-Genie
==================================================
This is a derivative work based on:
   Jim Lindblom
   SparkFun Electronics
   date: 1/18/11
   [Source:](http://www.sparkfun.com/tutorial/Barometric/BMP085_Example_Code.pde)
   & John Burns (www.john.geek.nz)
   & Gordon Henderson for BMP code
   & Folkert van Heusden for AS3935 code

A C program for interacting with the
[AMS Franklin Lightning Sensor](http://www.ams.com/eng/Products/RF-Products/Lightning-Sensor/AS3935).
[BMP085 Spark Fun Barometric Pressure Sensor](https://www.sparkfun.com/products/9694).
[LCD 4D Sytems Visi-Genie communication](http://www.4dsystems.com.au/product/uLCD_32W_PTU_AR/)

This script will only work if the correct kernel modules are loaded
on your Pi.  Adafruit has a nice [tutorial](http://learn.adafruit.com/adafruits-raspberry-pi-lesson-4-gpio-setup/configuring-i2c)
set up, though depending on the breakout board that you use, you may 
not see anything show up when you run `i2cdetect`.

## Installation

You have to install libgeniePi
```
git clone https://github.com/4dsystems/ViSi-Genie-RaspPi-Library.git
cd ViSi-Genie-RaspPi-Library/
make
sudo make install
```

You have to install WiringPi
```
git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi
./build
```

## Breakout Board

I purchased a (v4) breakout board from [Embedded Adventures](http://www.embeddedadventures.com/as3935_lightning_sensor_module_mod-1016.html).


## Connecting the AS3935

| AS3935 Pin | Raspberry Pi Pin |
| ---------: | :--------------- |
| 4 (GND)    | 25 (Ground)      |
| 5 (VDD)    | 1 (3v3 Power)    |
| 10 (IRQ)   | 11 (GPIO 21)     |
| 11 (I2CL)  | 5 (SCL)          |
| 13 (I2CD)  | 3 (SDA)          |

## Connecting the BMP085

| BMP085 Pin | Raspberry Pi Pin |
| ---------: | :--------------- |
| GND        | 25 (Ground)      |
| VDD        | 1 (3v3 Power)    |
| SCL        | 5 (SCL)          |
| SDA        | 3 (SDA)          |

## Connecting the LCD

| LCD Pin    | Raspberry Pi Pin |
| ---------: | :--------------- |
| GND        | 25 (Ground)      |
| VDD        | 2 (5v  Power)    |
| TX         | 10 (UART_RXD)    |
| RX         | 8 (UART_TXD      |

## Known Issues

