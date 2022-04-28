#include "I2C_message_protocol.h"
#include "wakeup.h"

// this function will be called when someone requests a change in the system luminance in the main loop (it will be listening)
void get_all_control()
{

    // luminaire with lowest address takes control of operations and calculates controls
    int aux_order = -1;
    bool control_over = true;
    int aux_loop = 0;
    float u1 = 0.0;
    float u2 = 0.0;
    float u3 = 0.0;
    float u1_prev = 0.0;
    float u2_prev = 0.0;
    float u3_prev = 0.0;

    // check the position of this luminaire in the calibration sequence
    aux_order = I2C_message_protocol::addr_is_saved(I2C::get_I2C1_address());

    if (aux_order == 0) // first luminary
    {
        while (!control_over)
        {
            if (aux_loop == 0)
            {
                // first time uses default u2 and u3
                // computes u1
                u1 = compute_control();

                // sends to second and third
                I2C_message_protocol::broadcast_control(); // falta fazer mensagem (manda commando)

                // dont forget to flush self
                while (I2C::buffer_not_empty())
                {
                    I2C::pop_message_from_buffer();
                }

                u1_prev = u1;
            }
            else
            {
                // keeps listening to others
                for (int i = 0; i < 2; i++)
                {
                    if (i == 0)
                    {
                        u2 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem
                    }
                    else
                    {
                        u3 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem
                    }
                }

                // computes next u1
                u1 = compute_control();

                // sends to second and third
                I2C_message_protocol::broadcast_control(); // falta fazer mensagem (manda commando)

                // dont forget to flush self
                while (I2C::buffer_not_empty())
                {
                    I2C::pop_message_from_buffer();
                }

                u1_prev = u1;
            }

            aux_loop++;

            // test convergence
            if ((u1_prev - u1) < 0.01 || (u1_prev - u1) > -0.01)
            {

                control_over = true;
            }
        }
    }
    else if (aux_order == 1) // second luminary
    {
        while (!control_over)
        {
            if (aux_loop == 0)
            {
                // first time waits for u1 and uses default u3
                u1 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem

                // computes u2
                u2 = compute_control();

                // sends to first and third
                I2C_message_protocol::broadcast_control(); // falta fazer mensagem (manda commando)

                // dont forget to flush self
                while (I2C::buffer_not_empty())
                {
                    I2C::pop_message_from_buffer();
                }

                u2_prev = u2;
            }
            else
            {
                // keeps listening to others
                for (int i = 0; i < 2; i++)
                {
                    if (i == 0)
                    {
                        u3 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem
                    }
                    else
                    {
                        u1 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem
                    }
                }

                // computes next u2
                u2 = compute_control();

                // sends to first and third
                I2C_message_protocol::broadcast_control(); // falta fazer mensagem (manda commando)

                // dont forget to flush self
                while (I2C::buffer_not_empty())
                {
                    I2C::pop_message_from_buffer();
                }

                u2_prev = u2;
            }

            aux_loop++;

            // test convergence
            if ((u2_prev - u2) < 0.01 || (u2_prev - u2) > -0.01)
            {

                control_over = true;
            }
        }
    }
    else if (aux_order == 1) // third luminary
    {
        while (!control_over)
        {
            if (aux_loop == 0)
            {
                // first time waits for u1 and u2
                for (int i = 0; i < 2; i++)
                {
                    if (i == 0)
                    {
                        u1 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem
                    }
                    else
                    {
                        u2 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem
                    }
                }

                // computes u3
                u3 = compute_control();

                // sends to first and third
                I2C_message_protocol::broadcast_control(); // falta fazer mensagem (manda commando)

                // dont forget to flush self
                while (I2C::buffer_not_empty())
                {
                    I2C::pop_message_from_buffer();
                }

                u3_prev = u3;
            }
            else
            {
                // keeps listening to others
                for (int i = 0; i < 2; i++)
                {
                    if (i == 0)
                    {
                        u1 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem
                    }
                    else
                    {
                        u2 = wait_for_message_control(CONTROL_BROADCAST); // falta fazer mensagem
                    }
                }

                // computes next u3
                u3 = compute_control();

                // sends to second and third
                I2C_message_protocol::broadcast_control(); // falta fazer mensagem (manda commando)

                // dont forget to flush self
                while (I2C::buffer_not_empty())
                {
                    I2C::pop_message_from_buffer();
                }

                u3_prev = u3;
            }

            aux_loop++;

            // test convergence
            if ((u3_prev - u3) < 0.01 || (u3_prev - u3) > -0.01)
            {

                control_over = true;
            }
        }

        int resp_aux = 0;
        int aux_2 = 0;

        // dont forget to flush self
        while (I2C::buffer_not_empty())
        {
            I2C::pop_message_from_buffer();
        }

        control_over = false;
        // if reaches convergence, needs to tell others
        while (resp_aux < 1)
        {
            I2C_message_protocol::broadcast_end(); //

            // keeps sending message to end everything to others until gets response

            sleep_ms(100);

            // listens to others
            if (I2C::buffer_not_empty())
            {
                I2C::i2c_message _message = (I2C::pop_message_from_buffer());
                if (_message.msg_id == END_RESPONSE)
                {
                    resp_aux++;
                }
                else
                {
                    // dont forget to flush self
                    while (I2C::buffer_not_empty())
                    {
                        I2C::pop_message_from_buffer();
                    }
                }
            }

            aux_2++;
        }

        float compute_control(float _l, float **_k, float _o, float _ux, float _uy, int _order)
        {
            // checkar indices matriz

            float _u = ((_l - _o) / _k[_order][0]) - (((_k[_order][1]) * _ux) / _k[_order][0]) - (((_k[_order][2]) * _uy) / _k[_order][0]);

            return _u;
        }

        // falta saber o campo da mensagem
        float wait_for_message_control(uint8_t _message_id)
        {
            while (1)
            {
                if (I2C::buffer_not_empty())
                {
                    I2C::i2c_message _message = (I2C::pop_message_from_buffer());
                    if (_message.msg_id == _message_id)
                    {
                        return _message.control //?? QUAL é o campo??
                            break;
                    }
                    else if(_message.msg_id == BROADCAST_END)
                    {
                        return //exit par alterar control_over
                    }
                    
                }
            }
        }
