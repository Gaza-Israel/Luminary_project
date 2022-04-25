#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\calibration.h"
#ifndef CALIBRATION_H
#define CALIBRATION_H

//#include "Arduino.h"
//#include <stdio.h>
//#include <math.h>
#include "volt2lux.h"

class calibration_data{

    public:

    double gain = 0.0;    //PROVISORIO
    double noise = 0.0;
};

calibration_data get_G();

calibration_data get_G1(int, int, int);

double get_theta(bool, int, int);

bool custom_sort(double, double);
double LinearRegression();

#endif  //CALIBRATION_H
