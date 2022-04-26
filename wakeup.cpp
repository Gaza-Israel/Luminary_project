#include "wakeup.h"

#include "I2C_message_protocol.h"
#include "LDR.h"
#include "simulator.h"

template <class To, class From>
std::enable_if_t<
    sizeof(To) == sizeof(From) &&
        std::is_trivially_copyable_v<From> &&
        std::is_trivially_copyable_v<To>,
    To>
// constexpr support needs compiler magic
bit_cast(const From &src) noexcept
{
    static_assert(std::is_trivially_constructible_v<To>,
                  "This implementation additionally requires destination type to be trivially constructible");

    To dst;
    memcpy(&dst, &src, sizeof(To));
    return dst;
}

wake_up::wake_up(LDR *init_ldr, Simulator *sim)
{
    this->_ldr = init_ldr;
    this->_sim = sim;
}

void wake_up::calibrate_all()
{
    int aux_order = -1; // aux variable for calibration sequence position

    // gets addresses
    I2C_message_protocol::nodes_addr; // addresses;  // fazer variavel local e adicionar o (endereço local) (memcopy ou loop for)
    // get_addresses(addresses);

    // sorts addresses form smaller to larger
    // I2C_message_protocol::sort_addresses();  // usar quicksort do median.h

    // check the position of this luminaire in the calibration sequence
    // aux_order = I2C_message_protocol::addr_is_saved(/*endereço desta luminária*/)

    // each luminaire tells it is ready when they have all the addresses
    // ready_to_calibrate();

    // whole calibration loop
    for (int i = 0; i < N_LUMINARIES; i++)
    {
        if (i == aux_order)
        {
            // self calibration
        }
        else
        {
            // fazer flag para dizer que esta em calibraçao cruzada
            // other_calibration
        }
    }
}

// when this luminaire calibrates
void wake_up::self_calibration()
{
    Serial.print("Setting Coefficients...\n");
    this->_ldr->set_coefficients(-1.0, 4.8346); // Sets ldr coefficients

    // send message that its going to calibrate G
    I2C_message_protocol::g_calib_start();

    Serial.print("Calibrating G...\n");
    this->_sim->calibrate_G(50, false); // alterar para mandar send_duty cycle dentro da função

    // I2C_message_protocol::g_calib_end();

    Serial.print("Calibrating Tau...\n");
    this->_sim->calibrate_tau(5);

    Serial.print("Calibrating Theta...\n");
    this->_sim->calibrate_theta(1);

    // I2C_message_protocol::self_calib_end();

    // flushes buffer
    I2C::pop_message_from_buffer();
}

// when this luminaire is only reading to assess cross gains
// knows when to enter based on calibration sequence (queu)
void wake_up::other_calibration()
{

    // waits for other luminaire to send message that is going to calibrate
    //(...); sends acknowlagement

    // waits until receive message of other starting G calibration
    while (this->_waiting_for_crossed_g)
    {
        if (I2C::buffer_not_empty())
        {
            I2C_message_protocol::parse_message(I2C::pop_message_from_buffer());
        }
    }
    BLA::Matrix<CALIBRATION_POINTS, 1> DC;
    BLA::Matrix<CALIBRATION_POINTS, 1> L;
    BLA::Matrix<1, 1> square;
    BLA::Matrix<1, 1> Gain;

    // fazer ciclo recebe duty cycle e mede no LDR
    for (int i = 0; i < CALIBRATION_STEPS; i++)
    {

        while (1)
        {

            // receive duty cycle from cross calibrating luminaire
            if (I2C::buffer_not_empty())
            {
                I2C::i2c_message _msg_dc = (I2C::pop_message_from_buffer());
                if (_msg_dc.msg_id) == send_dc_MSG_ID)
                    {
                        DC(i, 0) = I2C_message_protocol::bit_cast<float>(_msg_dc.data);
                        break;
                    }
            }
        }

        // medir valor do LDR
        L(i, 0) = 10 * (-L0 + (this->_ldr->median_measure(5)));
    }

    // compute gains
    square = ~DC * DC;
    bool is_nonsingular = BLA::Invert(square);
    if (is_nonsingular)
    {
        Gain = (square * ~DC) * L;
        this->_G = Gain(0) / 10;
    }
    else
    {
        Serial.print("Matrix is Singular\n");
    }

    // waits for message about end of G calibration of the other luminaire

    // broadcasts that this luminiare is ready for next step
}

// assess when everyone is ready to calibrate
void wake_up::ready_to_calibrate()
{
    int all_ready = 0;

    // send I'm ready message
    //(...)

    // waits for the others being ready
    while (all_ready < 2)
    {
        if (1 /* get ready from others */)
        {
            all_ready++;
        }
    }

    return;
}

// receives empty array of addresses, populates it and returns the filled array
void wake_up::get_addresses()
{
    while (1) // faz broadcast e lê
        // broadcasts addresses
        //(...)
        // outro while para ler varias
        // gets others adderesses to array
        //(...)
        // save_addr(_addr);

        return;
}