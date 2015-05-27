/*

Raspberry Pi BMP180 sensor & LCD 4D Sytems Visi-Genie communication code 
by:	Roy Trenneman
date 	May 2015

This is a derivative work based on:

   Jim Lindblom
   SparkFun Electronics
   date: 1/18/11
   Source: http://www.sparkfun.com/tutorial/Barometric/BMP085_Example_Code.pde

   & John Burns (www.john.geek.nz)

   & Gordon Henderson for BMP code

Circuit detail:
   Using a Spark Fun Barometric Pressure Sensor - BMP085 breakout board
   link: https://www.sparkfun.com/products/9694
   This comes with pull up resistors already on the i2c lines.
   BMP085 pins below are as marked on the Sparkfun BMP085 Breakout board

   SDA   -    P1-03 / IC20-SDA
   SCL   -    P1-05 / IC20_SCL
   XCLR   -    Not Connected
   EOC   -   Not Connected
   GND   -   P1-06 / GND
   VCC   -    P1-01 / 3.3V
   
   Note: Make sure you use P1-01 / 3.3V NOT the 5V pin.

Use make to compil.
Depends on libgeniePi https://github.com/4dsystems/ViSi-Genie-RaspPi-Library
Depends on GENIE_OBJ_THERMOMETER objet 
Depends on GENIE_OBJ_COOL_GAUGE objet
TODO:

Manage Genie Event
Make a bar graph history 
 ***********************************************************************
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <math.h>

#include <geniePi.h>

#include "bmp.h"

#ifndef	TRUE
#  define TRUE  (1==1)
#  define FALSE (1==2)
#endif

// GAUGE Offsets

#define	TEMP_BASE	 0
#define	TEMP_BASE_MIN	 7
#define	TEMP_BASE_MAX	14
#define	PRESSURE_BASE	21
#define TAILLE_MAX 10 // Tableau de taille 1000
// Globals

int temps [8], minTemps [8], maxTemps [8] ;
int currentTemp, minTemp, maxTemp ;



/*
 * updateTemp:
 *	This sends the relevant data to the current temperature
 *	guagues on the display.
 *
 *	This page has 7 days of history and a live temperature gauge.
 *********************************************************************************
 */

static void updateTemp (int value)
{
 int v ;
 v = value + 10 ;
 genieWriteObj (GENIE_OBJ_THERMOMETER, 0, v) ;
}

static void updatePressure (int live)
{
 int v ;
 v = live - 940 ;
 if (v <   0) v =   0 ;
 if (v > 120) v = 120 ;
 genieWriteObj (GENIE_OBJ_COOL_GAUGE, 0, v) ;

}


/*
 * 
 * HandleGenie Event: not used right now...
*/

void handleGenieEvent (struct genieReplyStruct *reply)
{
  if (reply->cmd != GENIE_REPORT_EVENT)
  {
    printf ("Invalid event from the display: 0x%02X\r\n", reply->cmd) ;
    return ;
  }

  /**/ if (reply->object == GENIE_OBJ_WINBUTTON)
  {
    /**/ if (reply->index == 2) // Button 2 -> Reset Min
    {
      minTemp = currentTemp ;
    //  updateTemp (minTemps, minTemp, TEMP_BASE_MIN, 1) ;
    }
    else if (reply->index == 6) // Button 6 -> Reset Max
    {
      maxTemp = currentTemp ;
      //updateTemp (maxTemps, maxTemp, TEMP_BASE_MAX, 2) ;
    }
    else
      printf ("Unknown button: %d\n", reply->index) ;
  }
  else
    printf ("Unhandled Event: object: %2d, index: %d data: %d [%02X %02X %04X]\r\n",
      reply->object, reply->index, reply->data, reply->object, reply->index, reply->data) ;
}


/*
 * handleTemperature:
 *	This is a thread that runs in a loop, polling the temperature
 *	sensor and updating the display as required.
 *********************************************************************************
 */

static void *handleTemperaturePressure (void *data)
{
double temperature;
double pressure;
        for (;;)
	{
	bmp085_Calibration();
        temperature = bmp085_GetTemperature(bmp085_ReadUT());
	pressure = bmp085_GetPressure(bmp085_ReadUP());
        int press = (pressure/100);
	int temp = round(temperature/10);
	updateTemp (temp) ;
	updatePressure (press) ;
 	sleep(10) ; //Wait 10s to avoid concurrent access
}
    return 0;
}



/*
 *********************************************************************************
 * main:
 *********************************************************************************
 */

int main ()
{
  pthread_t myThread ;
  struct genieReplyStruct reply ;

// Genie display setup
//	Using the Raspberry Pi's on-board serial port.

  if (genieSetup ("/dev/ttyAMA0", 115200) < 0)
  {
    fprintf (stderr, "rgb: Can't initialise Genie Display: %s\n", strerror (errno)) ;
    return 1 ;
  }

// Select form 0 (the temperature)

  genieWriteObj (GENIE_OBJ_FORM, 0, 0) ;

// Start the temperature and pressure sensor reading threads

  (void)pthread_create (&myThread, NULL, handleTemperaturePressure, NULL) ;

// Big loop - just wait for events from the display now

  for (;;)
  {
    while (genieReplyAvail ())
    {
      genieGetReply    (&reply) ;
      handleGenieEvent (&reply) ;
    }
    usleep (10000) ; // 10mS - Don't hog the CPU in-case anything else is happening...
  }

  return 0 ;
}
