#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\simulator.cpp"
#include "simulator.h"


//this uses the dynamic equation solution
//predicts the supposed voltage at time t
double get_simulation_voltage_lux(total_controll _data, volt_read _read){

    total_controll data = _data;
    volt_read read = _read;
    unsigned long int ti = data.ti;

    unsigned long int t = ti + 10000;   //t é o tempo da previsão (10ms depois da aquisição anterior)

    double tau = 12000; //tau em MICROsegundos PROVISORIO (versao final chamar tau selector)
    
    double t_fixed = t - THETA;   //corrected time with theta (delay para começar a resposta)

   /* Serial.print("t: ");
    Serial.print(t,8);
    Serial.print("\n");
    Serial.print("vf: ");
    Serial.print(data.vf,8);
    Serial.print("\n");
    Serial.print("vi: ");
    Serial.print(data.vi,8);
    Serial.print("\n"); */
    double base_product = data.vf - data.vi;
    /*Serial.print("base product: ");
    Serial.print(base_product,8);
    Serial.print("\n");*/
    double exponent = (- ((t_fixed - data.ti) / tau));
    /*Serial.print("exponent: ");
    Serial.print(exponent,8);
    Serial.print("\n");*/
    double pred_voltage = data.vf - (base_product * exp(exponent)); //predicted voltage at time t (t_fixed)

    double pred_lux = volt_read::volt2lux(pred_voltage);
    
    return pred_lux; //returns predicted lux for that future aquisition time


    //PROVISORIO para imprimir valores e comparar real com simulação
    /*Serial.print(ti, 8);
    Serial.print(",");
    Serial.print(t_fixed, 8);
    Serial.print(",");
    Serial.print((base_product * exp(exponent)), 8);
    Serial.print(",");
    Serial.print(exponent, 8);
    Serial.print(",");
    Serial.print(t_fixed, 8);
    Serial.print(",");
    Serial.print(output_data.pred_voltage, 15);
    Serial.print(",");
    Serial.print(read_real.voltage, 8);
    Serial.print("\n");
    */

}

//returns specific tau to use based on step range
double tau_selector(double vi, double vf){

    double tau = 0.0;

    //usar switch...case

    return tau;
}


