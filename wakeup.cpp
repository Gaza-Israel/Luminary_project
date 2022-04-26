#include "Controller.h"
#include "I2C_message_protocol.h"
#include <bits/stdc++.h>

void calibrate_all(){

    int aux_order = -1; //aux variable for calibration sequence position

    //gets addresses
    I2C_message_protocol::nodes_addr addresses;
    //get_addresses(addresses);

    //sorts addresses form smaller to larger
    sort(addresses, N_LUMINARIES);

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
            //other_calibration
        }
        
        
    }


}

//when this luminaire calibrates
void self_calibration(){

    Serial.print("Setting Coefficients...\n");
    L1.ldr.set_coefficients(-1.0, 4.8346);  // Sets ldr coefficients

    //send message that its going to calibrate G, others send acknolagment
    //(...)

    //when receives confirmation from others, proceed to calibrate G, others enter in cross calibration mode
    //(...)

    Serial.print("Calibrating G...\n");
    L1.sim.calibrate_G(50, false);

    //sends message about ending calibration of G so others end cross calibration
    (...)

    Serial.print("Calibrating Tau...\n");
    L1.sim.calibrate_tau(5);

    Serial.print("Calibrating Theta...\n");
    L1.sim.calibrate_theta(1);

    //sends message about ending calibration of this luminaire
    //(...)

}

//when this luminaire is only reading to assess cross gains
//knows when to enter based on calibration sequence (queu)
void other_calibration(){

    //waits for other luminaire to send message that is going to calibrate
    //(...); sends acknowlagement

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
void get_addresses(I2C_message_protocol::nodes_addr _addresses){

    //broadcasts addresses
    //(...)

    //gets others adderesses to array
    //(...)
    //save_addr(_addr);

    return;
}