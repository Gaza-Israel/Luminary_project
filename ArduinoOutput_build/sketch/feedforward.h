#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\feedforward.h"
#ifndef FEEDFORWARD_H
#define FEEDFORWARD_H

//#include "Arduino.h"
//#include <stdio.h>
//#include <math.h>
//#include "volt2lux.h"
#include "calibration.h"

double get_feedforward_command(double, calibration_data);

#endif  //FEEDFORWARD_H