#include <Arduino.h>
#line 1 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\main.ino"
//#include "Arduino.h"
//#include <stdio.h>
//#include "volt2lux.h"
//#include "calibration.h"
//#include "feedforward.h"
//#include "controller.h"
#include "simulator.h"

//timer declarations
volatile unsigned long int timer_time {0};
volatile bool timer_fired {false};
repeating_timer timer;

#line 14 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\main.ino"
bool my_repeating_timer_callback(struct repeating_timer *t);
#line 34 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\main.ino"
void setup();
#line 55 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\main.ino"
void loop();
#line 14 "c:\\Users\\andre\\Desktop\\SCDTR\\Codigo\\Projeto1\\main\\main.ino"
bool my_repeating_timer_callback(struct repeating_timer *t){

    if(!timer_fired) {

        timer_time = micros();
        timer_fired = true;
    }
    
    return true;
}

int counter = 0;
float aux = 0;
long startTime = 0.0;
long endTime = 0.0;

volt_read read;
calibration_data calib_data;

// the setup function runs once 
void setup() {

    Serial.begin(115200);
    analogReadResolution(12); 
    analogWriteFreq(30000); //previously 60000
    analogWriteRange(DAC_RANGE); //100% duty cycle

    //add 100 Hz timer
    add_repeating_timer_ms(-10, my_repeating_timer_callback, NULL, &timer); //100 Hz

    Serial.print("Calibrating:\n");
    calib_data = get_G();
    Serial.print("Gain is: ");
    Serial.print(calib_data.gain,8);
    Serial.print("\nNoise in lux: ");
    Serial.print(calib_data.noise,8);

    //calib_data.gain = G;
}

// the loop function runs over and over again
void loop() {

    Serial.print("\nReady\n");
    delay(5*1000);


    double forward = 0.0;
    float incomingByte = 0.0;

    int flag_fb = 0;

    if (Serial.available() > 0) {

        // lê do buffer o dado recebido:
        //char incomingByte = Serial.read();
        incomingByte = Serial.parseFloat();

        // responde com o dado recebido:
        if (Serial.read() == '\n')
        {
            Serial.print("I received: ");
            Serial.print(incomingByte, DEC);
            Serial.print("\n");
        }

    }

    double wanted_lux = 0.0;
    wanted_lux = (double)incomingByte;
    Serial.print("wanted_lux: ");
    Serial.print(wanted_lux);
    Serial.print("\n");


    flag_fb = 0;
    volt_read read_fb;
    total_controll controll;
    unsigned long int T = 0;
    double ti_aux = 0.0;
    bool first = true;
    timer_fired = false;

    while (flag_fb != 1)
    {
        if((timer_fired) && (flag_fb != 1)) {
        
            read_fb = get_sample(READ_PIN, ti_aux);
            delay(2);

            timer_fired = false;
            
            if (first)
            {
                controll.vi = read_fb.voltage;
                controll.vf = volt_read::lux2volt(wanted_lux);
                controll.ti = read_fb.real_time;

                Serial.print("controll.vi: ");
                Serial.print(controll.vi,8);
                Serial.print("\n");
                
                read_fb.acq_time = 1.0;
                first = false;
            }else
            {
                controll.ti = controll.ti + 10000;  //ti passa 10ms após a aquisição anterior
            }
            


            if (((wanted_lux - 0.1) <= read_fb.voltage_in_lux) && (read_fb.voltage_in_lux <= wanted_lux + 0.1))
            {
                flag_fb = 1;
            }
            else{

                controll = get_controll_signal(read_fb, wanted_lux, read_fb.voltage_in_lux, calib_data, controll);

                ti_aux = read_fb.real_time;

                //writing only with feedback
               //analogWrite(LED_PIN, (controll.u));

               //writing only with feedforward
                //Serial.print("uff fora: ");
                //Serial.print(controll.u_ff,8);
                //Serial.print("\n");

               analogWrite(LED_PIN, (controll.u));
               delay(2);
               
               //flag_fb = 1;
            }
        }
    }


    delay(1000);
    double  read_10001 = (get_sample(READ_PIN, ti_aux)).voltage_in_lux;
    Serial.print("read_lux: ");
    Serial.print(read_10001);
    Serial.print("\n");
    
    delay(1000*15);






/*
    analogWrite(LED_PIN, 0.75*(DAC_RANGE-1)); // set led PWM

    //Serial.print("Percent: ");
    //Serial.print(percent);
    //Serial.print("\n");

    delay(1000*10);




    startTime = micros();


    analogWrite(LED_PIN, 1.0*(DAC_RANGE-1)); // set led PWM

    for(aux = 1; aux < 20000 ; aux++){

    read.acq_time = (micros()) - startTime;
    Serial.print(read.acq_time);
    Serial.print(",");
    
    read.acq = analogRead(READ_PIN);

    read.voltage = read.get_true_voltage(read.acq);
    Serial.print(read.voltage,8);
    Serial.print("\n");
    delay(0.5); //delay em ms, correspondente a adquirir 2mil amostras por segundo (1/20000)*1000
    
    }
    Serial.print("Over");

    //percent = percent + 0.25;

    delay(1000*15);
*/
}

