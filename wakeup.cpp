#include "Controller.h"
#include "I2C_message_protocol.h"
#include "median_filter.h"
//#include <bits/stdc++.h>

namespace wake_up{


void calibrate_all(){

    int aux_order = -1; //aux variable for calibration sequence position

    //gets addresses
    I2C_message_protocol::nodes_addr addresses; //fazer variavel local e adicionar o (endereço local) (memcopy ou loop for)
    //get_addresses(addresses);

    //sorts addresses form smaller to larger
    I2C_message_protocol::sort_addresses(); //usar quicksort do median.h

    //check the position of this luminaire in the calibration sequence
    //aux_order = I2C_message_protocol::addr_is_saved(/*endereço desta luminária*/)

    //each luminaire tells it is ready when they have all the addresses
    //ready_to_calibrate();

    //whole calibration loop
    for (int i = 0; i < N_LUMINARIES; i++)
    {
        if (i == aux_order)
        {
            //self calibration
        }
        else
        {
            //fazer flag para dizer que esta em calibraçao cruzada
            //other_calibration
        }
        
        
    }


}

//when this luminaire calibrates
void self_calibration(){

    Serial.print("Setting Coefficients...\n");
    L1.ldr.set_coefficients(-1.0, 4.8346);  // Sets ldr coefficients

    //send message that its going to calibrate G
    I2C_message_protocol::g_calib_start();

    Serial.print("Calibrating G...\n");
    L1.sim.calibrate_G(50, false);  //alterar para mandar send_duty cycle dentro da função

    I2C_message_protocol::g_calib_end();

    Serial.print("Calibrating Tau...\n");
    L1.sim.calibrate_tau(5);

    Serial.print("Calibrating Theta...\n");
    L1.sim.calibrate_theta(1);

    I2C_message_protocol::self_calib_end();

}

//when this luminaire is only reading to assess cross gains
//knows when to enter based on calibration sequence (queu)
void other_calibration(){

    bool on_calibration = false;

    //waits until receive message of other starting G calibration
    while(this->_waiting_for_crossed_g){
        
        if (I2C::buffer_not_empty()) {
            I2C_message_protocol::parse_message(I2C::pop_message_from_buffer());
        }
    }
    //fazer ciclo recebe duty cycle e mede no LDR
    for (int i = 0; i < CALIBRATION_STEPS; i++)
    {
        on_calibration = true;

        while(this->){
        
        if (I2C::buffer_not_empty()) {
            I2C_message_protocol::parse_message(I2C::pop_message_from_buffer());
        }
    }

    on_calibration = false;
    }
    



    //cross calibrates
    //(INCOMPLETE!!!)

    double L0;

    this->ldr->average_measure(avrg_samples);

    L0 = this->ldr->lux;
    this->_L0 = L0;


    //waits for message about end of G calibration of the other luminaire

    //broadcasts that this luminiare is ready for next step
}

//assess when everyone is ready to calibrate
void ready_to_calibrate(){

    int all_ready = 0;

    //send I'm ready message
    //(...)

    //waits for the others being ready
    while(all_ready < 2){

        if (/* get ready from others */)
        {
            all_ready++;
        }
        
    }
    
    return;
}

//receives empty array of addresses, populates it and returns the filled array
void get_addresses(){

    while()//faz broadcast e lê
    //broadcasts addresses
    //(...)
    //outro while para ler varias
    //gets others adderesses to array
    //(...)
    //save_addr(_addr);

    return;
}

}