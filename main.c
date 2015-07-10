/*

Raspberry Pi BMP180 pressure sensor & AS3935 lightning sensor on  LCD 4D Sytems Visi-Genie communication code 
by:	Roy Trenneman
date 	May 2015

This is a derivative work based on:

   Jim Lindblom
   SparkFun Electronics
   date: 1/18/11
   Source: http://www.sparkfun.com/tutorial/Barometric/BMP085_Example_Code.pde

   & John Burns (www.john.geek.nz)

   & Gordon Henderson for BMP code
   & Folkert van Heusden for AS3935 code

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

Circuit detail:
  Using a AS3935 based lightning and storm sensor module
  Link : http://www.embeddedadventures.com/as3935_lightning_sensor_module_mod-1016.html
 Datasheet module: http://www.embeddedadventures.com/datasheets/MOD-1016_hw_v6_doc_v3.pdf
 Datasheet processor : http://www1.futureelectronics.com/doc/AUSTRIAMICROSYSTEMS/AS3935.pdf


Use make to compil.
Depends on libgeniePi https://github.com/4dsystems/ViSi-Genie-RaspPi-Library
Depends on GENIE_OBJ_THERMOMETER objet 
Depends on GENIE_OBJ_COOL_GAUGE objet
TODO:

Manage Genie Event
Make a bar graph history temperature
Add function to adjust noise level and some features for the lighting detection module
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
#include <time.h>
#include <geniePi.h>
#include <signal.h>
#include "bmp.h"
#include <wiringPi.h>
#include "as3935.h"

#ifndef	TRUE
#  define TRUE  (1==1)
#  define FALSE (1==2)
#endif

//AS3935 interrupt
#define	BUTTON_PIN    21	

// GAUGE Offsets
#define	TEMP_BASE	 0
#define	TEMP_BASE_MIN	 7
#define	TEMP_BASE_MAX	14
#define	PRESSURE_BASE	21
#define TAILLE_MAX 10 // Tableau de taille 1000
// Globals

int temps [8], minTemps [8], maxTemps [8] ;
int currentTemp, minTemp, maxTemp ;
int historyp [8] = {0} ;
int historyt [8] = {0} ;
int distance  ;
int INT_L;

void usage()
{
printf("Usage: Weather [-n] | [-L] | [-c capacitor ] | [-r register] \n "
"-n, --normal       Display on Stdout lighting detection, display on LCD 4D pressure & temperature \n "
"-L, --LCO          Display on the IRQ pin the resonance frequency of the antenna as a digital signal \n "
"-c, --calibrate    Tune the antenna adding the internal capacitors(from 0 to 120pF in steps of 8pf) \n "
"\t""capacitor is an integer between 0 and 15 \n "
"-r, --read         Read a register and return value in Hexa \n "
"\t""register in an integer between 0 and 8 \n ");
}


/*
 * updateTemp:
 *	This sends the relevant data to the current temperature & pressure
 *	guagues on the display.
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

static void trend(int livep)
{
	int i ;
	int p;
	historyp [7] = livep;

	for (i = 0 ; i < 8 ; ++i){
		p = historyp [i] - 940 ;
		if (p < 0)
		p = 0 ;
		 genieWriteObj (GENIE_OBJ_GAUGE, PRESSURE_BASE + i, p * 100 / 120) ;
	}

	for (i = 1 ; i < 8 ; ++i)
 	historyp [i - 1] = historyp [i] ;
}

void getdistance(void)
{
	usleep(4000);
// start reading Interrupt
	INT_L = AS3935_returnInterrupt();
	usleep(2000);
 
	if (INT_L == 8){
		distance = AS3935_returnDistance();
 		printf("Lightning detected, it was %d km away \n", distance);
		sleep(1);
 	} else if ( INT_L == 1 ){
		printf(" noise level too high, ajusting: \n");
	  	set_noise();
	}
// empty buffer
	fflush(stdout);
}

/*
 * handleTemperature :
 *	This is a thread that runs in a loop, polling the temperature, pression sensor and updating the display as required.
 *********************************************************************************
 */

static void *handleTemperaturePressure (void *data)
{
	double temperature;
	double pressure;
	time_t sec;
	double sample = 1200; //trigger time (second) for trend 
	time_t initial = time (NULL) ;
	double delta ;

	for (;;){
		if(bmp085_Calibration() > 0 ){
			temperature = bmp085_GetTemperature(bmp085_ReadUT());
			sleep(1);
			pressure = bmp085_GetPressure(bmp085_ReadUP());
			int press = round(pressure/100);
			int temp = round(temperature/10);
			updateTemp (temp) ;
			updatePressure (press) ;
			sleep(10) ; //Wait 10s to avoid concurrent access
  			sec = time (NULL);
  			delta = difftime(sec,  initial);
   			if (delta > sample ){
  				trend (press);
				initial = time(NULL);
				sleep (1);
			}
		}	
		sleep (10);
	}
	return 0;
}

/*
 *********************************************************************************
 * main:
 *********************************************************************************
 */

int main (int argc, char *argv[])
{
	if (argc > 1){
		if(strcmp(argv[1], "-L") ==0 ){
		display_LCO() ;
		exit (1);
		} else if(strcmp(argv[1], "-c") ==0) {
			int c = atoi(argv[2]);
			set_cap(c);
			exit (1);
		} else if(strcmp(argv[1], "-r") ==0){
			int i = atoi(argv[2]);
			int reg;
			reg = read_reg(i);
			printf("Value for Register %d is %#x \n", i, reg);
			fflush(stdout);
			exit (1);
		} else if(strcmp(argv[1], "-n") == 0 ){
//Create the thread
			pthread_t bmp085 ;
//Start the Calibration 
			AS3935_Calibration();

			if (wiringPiSetupGpio () < 0) {
				fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno)) ;
    				return 1 ;
  			}
// Start the interrupt reading (this creates a thread)
			wiringPiISR (BUTTON_PIN, INT_EDGE_RISING, &getdistance);
//Using the Raspberry Pi's on-board serial port.
			if (genieSetup ("/dev/ttyAMA0", 115200) < 0) {
    				fprintf (stderr, "rgb: Can't initialise Genie Display: %s\n", strerror (errno)) ;
				return 1 ;
			}
			genieWriteObj (GENIE_OBJ_FORM, 0, 0) ;
// Start the temperature and pressure sensor reading threads
			(void)pthread_create (&bmp085, NULL, handleTemperaturePressure, NULL) ;
			(void)pthread_join (bmp085, NULL);
			return 0 ;
  		} else {
		 usage();
		} 
	} else {
	 usage();
	}	
	return 0;
}
