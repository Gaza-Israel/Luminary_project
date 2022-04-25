#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\controller.cpp"
//#include "Arduino.h"
//#include <stdio.h>
//#include <math.h>
//#include "volt2lux.h"
//#include "calibration.h"
//#include "feedforward.h"
#include "controller.h"
#include "simulator.h"


total_controll get_controll_signal(volt_read _read, double ref, double desired_lux, calibration_data _c_data, total_controll _controll_data){   

    total_controll controll_data = _controll_data;
    calibration_data c_data = _c_data;
    volt_read read = _read;

    unsigned long int _T = read.acq_time;
    double y = read.voltage_in_lux;
    unsigned long int t = read.real_time;
    double T = (double)_T / 1000000.0;  //passa _T para segundos
    double predict_ff = 0.0;    //predits output y in lux if only applied feedforward

    //feedforward
    //controll_data.u_ff = get_feedforward_command(desired_lux, c_data);
    controll_data.u_ff = 0.0; //PROVISORIO

    //ref = get_simulation_voltage_lux(controll_data, read);
    //controll_data.u_ff = get_feedforward_command(ref, c_data);
/*
    predict_ff = (volt_read::lux2volt((c_data.gain * (controll_data.u_ff)) + c_data.noise));
   
    Serial.print("predict final voltage of sample: ");
    Serial.print(predict_ff,8);
    Serial.print("\n");
    Serial.print("predict final lux of sample: ");
    Serial.print(volt_read::volt2lux(predict_ff),8);
    Serial.print("\n");
*/
    //IMPLEMENTAR ypred !!! (usar equaçoes da dinamica)

    //ref = get_simulation_voltage(controll_data, t);

    //feedback
    
    double e = ref - y;

    /*Serial.print("T: ");
    Serial.print(T,8);
    Serial.print(",");

    Serial.print("erro: ");
    Serial.print(e,8);
    Serial.print(",");

    Serial.print("ref: ");
    Serial.print(ref,8);
    Serial.print(",");

    Serial.print("y: ");
    Serial.print(y,8);
    Serial.print("\n");*/

    controll_data.up_fb = controll_data.kp * e;   //proportional update
    controll_data.ui_fb = controll_data.ui_before + (controll_data.ki * T * e); //integral update
    controll_data.u = controll_data.up_fb + controll_data.ui_fb + controll_data.u_ff; //falta adicionar u_ff
    controll_data.ui_fb = anti_windup(controll_data.ui_fb, controll_data.up_fb, controll_data.u_ff);
    if (controll_data.u < 0)
    {
        controll_data.u = 0.0;
    }
    else if (controll_data.u > (1*(DAC_RANGE-1)))
    {
        controll_data.u = (DAC_RANGE-1);
    }
    
    
    /*Serial.print("kp: ");
    Serial.print(controll_data.kp,8);
    Serial.print(",");

    Serial.print("ki: ");
    Serial.print(controll_data.ki,8);
    Serial.print(",");
    
    Serial.print("up: ");
    Serial.print(controll_data.up_fb,8);
    Serial.print(",");

    Serial.print("ui: ");
    Serial.print(controll_data.ui_fb,8);
    Serial.print("\n");*/

    controll_data.ui_before = controll_data.ui_fb;  //integral for sum update
    

    return controll_data;
}



//antiwindup COMEÇAR SO A FUNCIONAR, DEPOIS COLOCAR BACK CALCULATION SE TIVER TEMPO
// w is the sum of the other feedback terms, in my case, up (proportional)
double anti_windup(double ui, double w, double uff){

    double ui_out = 0.0;
    double u = ui + w + uff; 

    if((u > (1.0*(DAC_RANGE-1))) || (u < 0)){

        ui_out = 0.0;
        //Serial.print("Reset integrador\n");
    }
    else{

        ui_out = ui;
    }

    return ui_out;
}


