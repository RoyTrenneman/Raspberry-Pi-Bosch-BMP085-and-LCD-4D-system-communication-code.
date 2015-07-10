/*
Raspberry Pi AS3935 communication code.
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include "smbus.h" 
#define AS3935_I2C_ADDRESS 0x03

int AS3935_i2c_Begin()
{
	int as;
	char *fileName = "/dev/i2c-1";
   
   // Open port for reading and writing
	if ((as = open(fileName, O_RDWR)) < 0 )
	exit(1);
  
   // Set the port options and set the address of the device
	if (ioctl(as, I2C_SLAVE, AS3935_I2C_ADDRESS) < 0) {               
	close(as);
      	exit(1);
   	}
	return as;
}
// Read block from the AS3935 
void AS3935_i2c_Read_Int(int as, __u8 address, __u8 length, __u8 *values)
{
	if(i2c_smbus_read_i2c_block_data(as, address,length,values)<0) {
      	close(as);
      	exit(1);
  	}	
}
//Write a byte to the AS3935
void AS3935_i2c_Write_Byte(int as, __u8 address, __u8 value)
{
	if (i2c_smbus_write_byte_data(as, address, value) < 0) {
     	close(as);
      	exit(1);
   	}
}

int read_data(int i)
{
	__u8 values[8];
	int as = AS3935_i2c_Begin();
	AS3935_i2c_Read_Int(as, 0x00, 9, values);
	close(as);
	usleep(2000);
	return values[i];
}

void AS3935_Calibration()
{
	int values;
 //CALIB_RCO: 
	int as = AS3935_i2c_Begin();
// Power Down / Power on
	values = read_data(0);
	AS3935_i2c_Write_Byte(as,0x00, (values | 0x01)) ;
	usleep(5000);
	AS3935_i2c_Write_Byte(as,0x00, (values & 0xFE)) ;
	usleep(3000);
//Set all registers in default mode
// write 0x96 into Register 0x3C
	AS3935_i2c_Write_Byte(as,0x3C,0x96);
	usleep(3000);
//Calibrate Internal Capacitor to max (120pF)
	values = read_data(8);
//reg8 = AS3935_i2c_Read_Int(as,0x08);
	usleep(3000);
	AS3935_i2c_Write_Byte(as,0x08, (values & 0xF0) | 0x0f) ;
	usleep(3000);
//Recalibrating the automatically the RC ocsillators
	AS3935_i2c_Write_Byte(as,0x3D,0x96);
	usleep(3000);
// Watchdog threshold set to 0
	values = read_data(1);
	AS3935_i2c_Write_Byte(as,0x01, (values & 0xF0)) ;
	usleep(3000);
//Read Register 0x08; 
	values = read_data(8);
	//reg8 = AS3935_i2c_Read_Int(as,0x08);
	usleep(3000);
//write 0x01 into Register 0x08[5]; 
	AS3935_i2c_Write_Byte(as,0x08, (values | 0x20)) ;
	usleep(3000);
//Read Register 0x08; 
	values = read_data(8);
//reg8 = AS3935_i2c_Read_Int(as,0x08);
	usleep(3000);
//write 0x00 into Register 0x08[5]; 
	AS3935_i2c_Write_Byte(as,0x08, (values & 0xdf)) ;
	usleep(2000);
	close(as);
}

unsigned int AS3935_returnDistance()
{
	unsigned int distance;
	int values;
//Read Register 0x07; 
	values = read_data(7);
//Return Bits[5:0]
	distance = values & 0x3f;
	return distance;
	//close(as);
}

int AS3935_returnInterrupt()
{
	int interrupt;
	int values;
//Read Register 0x03; 
	values = read_data(3);
//Return Bits[5:0]
	interrupt = values & 0x0f;
	return interrupt;
}

void display_LCO()
{
	int as = AS3935_i2c_Begin();
	int values3 ;
	values3 = read_data(3);
	int values8 ;
	values8 = read_data(8);
//reg3 = AS3935_i2c_Read_Int(as,0x03);
	usleep(2000);
	AS3935_i2c_Write_Byte(as,0x03, (values3 & 0x3F) | 0xC0) ;
	usleep(2000);
//reg8 = AS3935_i2c_Read_Int(as,0x08);
	usleep(2000);
	AS3935_i2c_Write_Byte(as,0x08, (values8 & 0xFF) | 0x80) ;
	close(as);
	values8 = read_data(8);
	int cap = values8 & 0x0F;
	int capacitor = (cap * 120/15 );
	printf ("cap is %d pf \n",capacitor);
}

void set_cap(int c)
{
//Calibrate Internal Capacitor to max (120pF)              
	int values ;
	int as = AS3935_i2c_Begin();
	values = read_data(8);
        usleep(3000);        
//	int cprime = (c & 0x0F); 
        AS3935_i2c_Write_Byte(as,0x08, (values & 0xF0) | c ) ;     
        usleep(3000);
//      reg8 = AS3935_i2c_Read_Int(as,0x08);              
	values = read_data(8);
	int cap = values & 0x0F;
	int capacitor = (cap * 120/15 );
	printf ("cap is %d pf \n",capacitor);
	close(as);
//printf ("%d", reg8);
}
void set_noise()
{
	int noise_level;
	int values ;
	values = read_data(1);
	int as = AS3935_i2c_Begin();
      	noise_level = (values & 0x70) >> 4;              
        usleep(3000);        
	if (noise_level < 7){
		int new_noise_level = (noise_level + 1) << 4 ;
  		AS3935_i2c_Write_Byte(as,0x01, (values & 0x8F) | new_noise_level ) ;     
     		usleep(3000);
//reg8 = AS3935_i2c_Read_Int(as,0x08);              
		int rounded = round(new_noise_level * 100/112);
		printf("noise level is %d per cent\n ", rounded );
       		fflush(stdout);
		close(as);
	}else {
	printf(" the threshold for the noise floor is set to max" );
       	fflush(stdout);
	}
}
int read_reg(int i)
{
	int reg ;
	reg = read_data(i);
	return reg & 0xFF;
}
