#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\controller.h"
#ifndef CONTROLLER_H
#define CONTROLLER_H

//#include "Arduino.h"
//#include <stdio.h>
//#include <math.h>
//#include "volt2lux.h"
//#include "calibration.h"
#include "feedforward.h"


class total_controll{

    public:

    double u_ff = 0.0;

    double u = 0.0;
    double up_fb = 0.0;
    double ui_fb = 0.0;
    double ui_before = 0.0;

    double ki = 500.0;
    double kp = 500.0;

    double vi = 0.0;
    double vf = 0.0;
    unsigned long int ti = 0.0;

};


total_controll get_controll_signal(volt_read, double, double, calibration_data, total_controll);

double anti_windup(double, double, double);


#endif  //CONTROLLER_H