#ifndef LIB_AS_H
#define LIB_AS_H

void AS3935_Calibration();
int AS3935_returnDistance();
int AS3935_returnInterrupt();
void display_LCO();
void set_cap(int c);
void set_noise();
int read_reg(int i);

#endif
