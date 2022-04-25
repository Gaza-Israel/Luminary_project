#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\volt2lux.h"
#ifndef VOLT2LUX_H
#define VOLT2LUX_H

#include "Arduino.h"
#include <stdio.h>
#include <math.h>

using namespace std;

//timer declarations from pico SDK
struct repeating_timer;
typedef bool(* repeating_timer_callback_t )(repeating_timer_t *rt);

//timer declarations from slides
//volatile unsigned long int timer_time {0};
//volatile bool timer_fired {false};
//struct repeating_timer timer;


const int DAC_RANGE = 4096;
const float VCC = 3.3;
const float M = -0.799999; //m constant to convert volt to lux for this system
const float R1 = 10.0;  //R1 em KOhm (!!!)
//const float B = 3.152182518;    //b constant to convert volt to lux
const float B = 2.99999;    //b constant to convert volt to lux
//const double G = 25.193653112631583;    //gain G that relates input duty cycle with lux output in LED
const double G = 29.83926822;
const double G_b = -2.640710520771920;  //ordenada na origem from linear regression that relates duty cycle with lux
const float THETA = 1.5;  //theta in microseconds PROVISORIO!!!
//float conversion_factor = (3.3 / 65536.0); //gostava de saber porque raio não funciona com isto...

const int LED_PIN = 15;
const int READ_PIN = 26;


// timer functions from pico SDK
//bool alarm_pool_add_repeating_timer_us (alarm_pool_t *pool, int64_t delay_us, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out);
//static bool alarm_pool_add_repeating_timer_ms (alarm_pool_t *pool, int32_t delay_ms, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out);
static bool add_repeating_timer_us (int64_t delay_us, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out);
static bool add_repeating_timer_ms (int32_t delay_ms, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out);
//bool cancel_repeating_timer (repeating_timer_t *timer);



class volt_read{

    public:

    unsigned long int acq_time = 0;
    unsigned long int real_time = 0;    //stores time
    int acq = 0;   //stores read voltage (0 to 1023), (?) to obtain true voltage (3.3/1024)*acq_volt (??)
    double voltage = 0.0; //stores true voltage read (not ADC value)
    double voltage_in_lux = 0.0;
    
    static double volt2lux(double);
    static double volt2R2(double);
    static double lux2R2(double);
    static double R2_2volt(double);
    static double lux2volt(double);
    static double get_true_voltage(double);
};

volt_read get_sample(int, double);

#endif  //VOLT2LUX_H
