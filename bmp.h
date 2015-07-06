#ifndef LIB_BMP_H
#define LIB_BMP_H

extern unsigned int bmp085_Calibration();
extern unsigned int bmp085_ReadUP();
extern unsigned int bmp085_ReadUT();
extern unsigned int bmp085_GetPressure(unsigned int up);
extern unsigned int bmp085_GetTemperature(unsigned int ut);

#endif
