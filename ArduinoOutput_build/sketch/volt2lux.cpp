#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\volt2lux.cpp"
//#include "Arduino.h"
//#include <stdio.h>
//#include <math.h>
#include "volt2lux.h"

using namespace std;

//converts voltage value (in volts) into lux
double volt_read::volt2lux(double acq_volt){

    double R2 = volt2R2(acq_volt);  //R2 em KOhms
    double base = 10.0;
    double exponent = ((log10(R2) - B) / M);    //R2 em KOhms
    double lux = pow(base, exponent);

    return lux; 
}

//converts from voltage read (in volts) into R2 (LDR) value in KOhm (!!!)
double volt_read::volt2R2(double acq_volt){

    //double R2 = ((R1 * 1000.0) * ((VCC / (acq_volt)) - 1));
    double R2 = ((VCC * R1 * 1000) - (acq_volt * R1 * 1000)) / acq_volt;

    return (R2 / 1000);
}

//converts lux into the corresponding value of R2 (LDR) in KOhm
double volt_read::lux2R2(double lux){
    
    double base = 10.0;
    double exponent = (M * log10(lux)) + B;
    double R2 = (pow(base, exponent));

    return R2;
}

//converts the value of R2 (LDR) in ohms into voltage read in volts
double volt_read::R2_2volt(double R2){

    double volt = (VCC * (R1 * 1000)) / ((R1 * 1000) + R2);
    
    return volt;
}

//gets true voltage value (in volts) from ADC read
double volt_read::get_true_voltage(double acq){
    
    double conversion_factor = (3.3 / 4096.0);
    //double conversion_factor = (3.3) / (1 << 12);
    double voltage = acq * conversion_factor;

    return voltage;
}

//gets voltage (in volts) from lux
double volt_read::lux2volt(double lux){

    double volt = R2_2volt((lux2R2(lux)) * 1000);

    return volt;
}

volt_read get_sample(int read_pin, double ti){

    volt_read read;

    read.real_time = (micros());    //gets real time, useful for tracking acquisition since ti
    read.acq_time = ((read.real_time) - ti);  //how much time as passed since the last aquisition

    /*
    Serial.print("t inside get_sample: ");
    Serial.print(read.acq_time);
    Serial.print("\n");
    */
    
    read.acq = analogRead(read_pin);

    read.voltage = read.get_true_voltage(read.acq);

    read.voltage_in_lux = read.volt2lux(read.voltage);

    return read;

}