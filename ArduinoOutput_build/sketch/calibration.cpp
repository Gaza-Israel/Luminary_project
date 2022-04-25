#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\calibration.cpp"
//#include "Arduino.h"
//#include <stdio.h>
//#include <math.h>
//#include "volt2lux.h"
#include "calibration.h"

using namespace std;

//direction: 0->up;1->down
double get_theta(bool direction, int led_pin, int read_pin){

    analogWrite(led_pin, 0);
    long startTime = 0.0;
    int aux = 0;
    volt_read read;

    startTime = micros();
    read.acq_time = (micros()) - startTime;
    Serial.print(read.acq_time);
    Serial.print(",");
    
    read.acq = analogRead(read_pin);

    read.voltage = read.get_true_voltage(read.acq);
    Serial.print(read.voltage,8);
    Serial.print("\n");
    
    analogWrite(led_pin, (DAC_RANGE-1)); // set led PWM

    for(aux = 1; aux < 2000 ; aux++){

    read.acq_time = (micros()) - startTime;
    Serial.print(read.acq_time);
    Serial.print(",");
    
    read.acq = analogRead(read_pin);

    read.voltage = read.get_true_voltage(read.acq);
    Serial.print(read.voltage,8);
    Serial.print("\n");

 
    }

    return 0.0;

}

//retirado de https://www.analyticsvidhya.com/blog/2020/04/machine-learning-using-c-linear-logistic-regression/

/*
bool custom_sort(double a, double b) {
    double  a1=abs(a-0);
    double  b1=abs(b-0);
    return a1<b1;
}

double LinearRegression(){


double x[] = {0.51891623, 0.48089804, 0.44349024, 0.80026270, 0.44349024, 0.48089804, 0.48089804, 0.48089804, 0.80026270, 3.37782551};    // defining x values
double y[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};          // defining y values

vector<double>error;             // array to store all error values
double err;
double b0 = 0;                   //initializing b0
double b1 = 0;                   //initializing b1
double alpha = 0.01;             //intializing error rate
 
for (int i = 0; i < (18*4); i ++) {   // since there are 18 values and we want 4 epochs so run for loop for 20 times
    int idx = i % 18;              //for accessing index after every epoch
    double p = b0 + b1 * x[idx];  //calculating prediction
    err = p - y[idx];              // calculating error
    b0 = b0 - alpha * err;         // updating b0
    b1 = b1 - alpha * err * x[idx];// updating b1
    //cout<<"B0="<<b0<<" "<<"B1="<<b1<<" "<<"error="<<err<<endl;// printing values after every updation
    error.push_back(err);
}
sort(error.begin(),error.end(),custom_sort);//sorting based on error values

//cout<<"Final Values are: "<<"B0="<<b0<<" "<<"B1="<<b1<<" "<<"error="<<error[0]<<endl;

Serial.print("Slope: ");
Serial.print(b0);
Serial.print("\n");

return b0;

}
*/


//FALTA FAZER ISTO!!!!
calibration_data get_G1(int led_pin, int read_pin, int DAC_RANGE){

    calibration_data c_data;
    float aux = 0.5;
    double duty_cycles[18] = {0.0};
    double lux_vector[18] = {0.0};
    int counter = 0;


    for(aux = 1.0; aux <= 10 ; aux=aux+0.5){

        volt_read read;
        analogWrite(led_pin, (aux/10)*(DAC_RANGE-1));
        delay(50);
        read.acq = analogRead(read_pin);
        read.voltage = read.get_true_voltage(read.acq);
        double lux = read.volt2lux(read.voltage);

        duty_cycles[counter] = (((float)aux)/10.0);

        lux_vector[counter] = lux;

        counter++;
    }

    counter = 0;
    for(aux = 1.0; aux <= 10 ; aux=aux+0.5){

        Serial.print(duty_cycles[counter]);
        Serial.print(",");
        Serial.print(lux_vector[counter]);
        Serial.print("\n");

        counter++;
    }
    
    //arranjar função que dê o ganho (slope) e ordenada na origem e guarde em c_data


    return c_data;
}

calibration_data get_G(){

    calibration_data data;

    double duty_cycle1 = 0.5;

    analogWrite(LED_PIN, (duty_cycle1)*(DAC_RANGE-1));
    delay(1000);

    double derived_lux1 = ((get_sample(READ_PIN, 0)).voltage_in_lux);

    analogWrite(LED_PIN, 0.0*(DAC_RANGE-1));
    delay(1000);

    double duty_cycle2 = 1.0;

    analogWrite(LED_PIN, (duty_cycle2)*(DAC_RANGE-1));
    delay(1000);

    double derived_lux2 = ((get_sample(READ_PIN,0.0)).voltage_in_lux);

    analogWrite(LED_PIN, 0.0*(DAC_RANGE-1));
    delay(1000);

    data.gain = (derived_lux2 - derived_lux1) / (((duty_cycle2)*(DAC_RANGE-1)) - ((duty_cycle1)*(DAC_RANGE-1)));
    data.noise = ((get_sample(READ_PIN,0.0)).voltage_in_lux);


    return data;
}