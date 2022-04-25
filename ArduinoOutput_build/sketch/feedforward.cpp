#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\feedforward.cpp"
//#include "Arduino.h"
//#include <stdio.h>
//#include <math.h>
//#include "volt2lux.h"
//#include "calibration.h"
#include "feedforward.h"

double get_feedforward_command(double desired_lux, calibration_data c_data){

    double u_ff = (1.0 / (c_data.gain)) * (desired_lux - c_data.noise);

    Serial.print("control signal from feedforward: ");
    Serial.print(u_ff);
    Serial.print("\n");

    return u_ff;

}