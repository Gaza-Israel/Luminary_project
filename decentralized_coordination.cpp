#include "I2C_message_protocol.h"
#include "wakeup.h"

// this function will be called when someone requests a change in the system luminance in the main loop (it will be listening)
void get_all_control()
{

    // luminaire with lowest address takes control of operations and calculates controls
    int aux_order = -1;
    bool got_data = false;
    bool data_sent = false;
    bool received_controls = false;
    int aux_got_data = 0;

    // check the position of this luminaire in the calibration sequence
    aux_order = I2C_message_protocol::addr_is_saved(I2C::get_I2C1_address());

    if (aux_order == 0) // this is the luminary that will take controll
    {
        // request data from other luminaires (current luminances,external luminances, etc)
        I2C_message_protocol::broadcast_get_data(); // falta implementar funcção e mensagem

        // listens to responses
        while (aux_got_data < 2)
        {

            // saves requested data from other 2 luminaires (...)

            // each time it gets data from luminary; aux_got_data++;
        }

        // compute optimization problem and assess controls u1, u2, and u3

        // sends controls to other luminaires

        // changes illuminance on this luminary
    }
    else // this luminary will not take control
    {
        // when receives request from master luminary, sends requested data
        while (!data_sent)
        {

            // waits for data request
            wake_up::wait_for_message(BROADCAST_GET_DATA); // falta implementar mensagem

            // sends data FALTA FAZER MENSAGEM QUE MANDA DADOS NECESSARIOS

            data_sent = true;
        }
        while (!received_controls)
        {
            // receives controls

            // received_controls = true;
        }

        // changes illuminance in this luminary
    }
}

void quadratic_solver(float * _K, float * _o, float * _L)
{
    
}