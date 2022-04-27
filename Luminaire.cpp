#include "Luminaire.h"

#include "I2C_message_protocol.h"

/*
dc=5863308
r=177687
lms=193498321
lp=5863585
e=177674
p=177685
int=193495088
uff=193507878
ufb=193507874
u=177690
pw=5863724
xi=5863974
fl=5863383
*/

/* stream legend
 *  bit | 15 | 14  |   13   |   12   |  11 |  10  |  9  |   8  |   7  | 6 |  5  |    4    |    3    | 2 | 1 | 0
 *  var | DC | ref | L_meas | L_pred | err | prop | int | u_ff | u_fb | u | pwr | ext_ilu | flicker | - | - | -
 */

#define SET(byte, nbit) ((byte) |= (1 << (nbit)))
#define CLEAR(byte, nbit) ((byte) &= ~(1 << (nbit)))
#define TOGGLE(byte, nbit) ((byte) ^= (1 << (nbit)))
#define READ(byte, nbit) ((byte) & (1 << (nbit)))

/**
 * @brief Construct a new Luminary:: Luminary object.
 *  This object handle the creation of the LED, LDR, Controller, and Simulator objects.
 *
 * @param pin_led LED positive pin (needs to be PWM pin or DAC)
 * @param pin_ldr LDR negative pin (needs to be ADC pin)
 * @param id Luminary ID (next releases may handle multiple luminaries)
 */
Luminary::Luminary(int pin_led, int pin_ldr, int id)
    : led(pin_led),
      ldr(pin_ldr),
      sim(&this->led, &this->ldr),
      contr(&this->led, &this->ldr, &this->sim),
      wk(&led,&ldr,&sim) {
  _id = id;
}
/**
 * @brief Print some state variables of the Luminary
 * Deprecated - use stream method on parser.
 *
 * @param verbose Activates the verbose mode (prints more variables)
 */
void Luminary::print_state(bool verbose) {
  char buff[120];
  if (verbose) {
    sprintf(buff, "[%d]-LED(%%):%.3f, V(V*10):%.5f,  R(kOhms/100):%.5f,  L(Lux*10):%.5f, L_prev(Lux*10):%.5f", this->_id, float(this->led.dutty_cicle) / 10, 10 * this->ldr.voltage, float(this->ldr.resistance) / 100, 10 * this->ldr.lux, 10 * (this->sim.get_L0() + this->sim.get_G() * this->led.dutty_cicle));
  } else {
    sprintf(buff, "[%d]-LED(%%):%.3f, L(Lux*10):%.5f, L_prev(Lux*10):%.5f", this->_id, float(this->led.dutty_cicle) / 10, 10 * this->ldr.lux, 10 * (this->sim.get_L0() + this->sim.get_G() * this->led.dutty_cicle));
  }
  Serial.println(buff);
}
/**
 * @brief Get the current external illuminance estimative, based on the simulator and LDR values
 *
 * @return float - External illuminance
 */
float Luminary::get_ext_ilu() {
  return this->_external_iluminance;
}
/**
 * @brief Get the current power consumption of the LED and its resistance
 *
 * @return float - Power (Watts)
 */
float Luminary::get_curr_pwr() {
  return this->_curr_pwr;
}
/**
 * @brief Get the current value of accumulated energy spent by the LED and its resistance
 *
 * @return float - Energy spent
 */
float Luminary::get_acc_energy() {
  return this->_acc_energy;
}
/**
 * @brief Gets the accumulated visibility error value
 *
 * @return float Visibility error
 */
float Luminary::get_acc_visibility_err() {
  return this->_acc_visibility_err / this->_acc_counter;
}
/**
 * @brief Gets the accumulated flicker value
 *
 * @return float Accumulated Flicker
 */
float Luminary::get_acc_flicker() {
  return this->_acc_flicker / this->_acc_counter;
}
/**
 * @brief Gets the current flicker value
 *
 * @return float Current Flicker
 */
float Luminary::get_curr_flicker() {
  return this->_curr_flicker / this->_acc_counter;
}
/**
 * @brief Calculate the performance metrics
 *
 * @param l Current measured Illuminance
 * @param l1 Measured Illuminance ont t-1
 * @param l2 Measured Illuminance ont t-2
 */
void Luminary::calc_metrics(float l, float l1, float l2) {
  this->_acc_energy += this->_curr_pwr * 1 / SECUNDARY_TIMER_FREQ;
  this->_acc_visibility_err += max(0, this->contr.get_ref() - this->ldr.lux);
  this->_curr_flicker = ((abs(l - l1) + abs(l1 - l2)) * CONTROLLER_FREQ / 2) * ((l - l1) * (l1 - l2) < 0);
  this->_acc_flicker += this->_curr_flicker;
  this->_acc_counter++;
}
/**
 * @brief Calculate the external illuminance based on simulator and LDR values
 *
 */
void Luminary::calc_ext_ilu() {
  this->_external_iluminance = this->ldr.lux - this->sim.get_current_l_prediction();
}
/**
 * @brief Calculate the power consumption by the LED ant its resitance
 *
 */
void Luminary::calc_pwr() {
  this->_curr_pwr = this->led.dutty_cicle * AVRG_LED_MAX_CURRENT * Vcc / 100;
}
/**
 * @brief Reset the metrics accumulated values
 *
 */
void Luminary::reset_metrics() {
  this->_acc_counter = 0;
  this->_acc_energy = 0;
  this->_acc_flicker = 0;
  this->_acc_visibility_err = 0;
}
/**
 * @brief Gets the Luminary ID
 *
 * @return int - ID
 */
int Luminary::get_id() {
  return this->_id;
}
void Luminary::set_id(int id) {
  this->_id = id;
}

void Luminary::toggle_i2c_stream(unsigned long hash) {
  switch (hash) {
    case DC_HASH:
      TOGGLE(this->_i2c_stream, 15);
      break;
    case REF_HASH:
      TOGGLE(this->_i2c_stream, 14);
      break;
    case L_MEAS_HASH:
      TOGGLE(this->_i2c_stream, 13);
      break;
    case L_PRED_HASH:
      TOGGLE(this->_i2c_stream, 12);
      break;
    case ERR_HASH:
      TOGGLE(this->_i2c_stream, 11);
      break;
    case PROP_HASH:
      TOGGLE(this->_i2c_stream, 10);
      break;
    case INTEGRAL_HASH:
      TOGGLE(this->_i2c_stream, 9);
      break;
    case U_FF_HASH:
      TOGGLE(this->_i2c_stream, 8);
      break;
    case U_FB_HASH:
      TOGGLE(this->_i2c_stream, 7);
      break;
    case U_HASH:
      TOGGLE(this->_i2c_stream, 6);
      break;
    case PWR_HASH:
      TOGGLE(this->_i2c_stream, 5);
      break;
    case EXT_ILU_HASH:
      TOGGLE(this->_i2c_stream, 4);
      break;
    case FLICKER_HASH:
      TOGGLE(this->_i2c_stream, 3);
      break;
    default:
      Serial.println("Var not found");
      break;
  }
}
void Luminary::toggle_serial_stream(unsigned long hash) {
  switch (hash) {
    case DC_HASH:
      TOGGLE(this->_serial_stream, 15);
      break;
    case REF_HASH:
      TOGGLE(this->_serial_stream, 14);
      break;
    case L_MEAS_HASH:
      TOGGLE(this->_serial_stream, 13);
      break;
    case L_PRED_HASH:
      TOGGLE(this->_serial_stream, 12);
      break;
    case ERR_HASH:
      TOGGLE(this->_serial_stream, 11);
      break;
    case PROP_HASH:
      TOGGLE(this->_serial_stream, 10);
      break;
    case INTEGRAL_HASH:
      TOGGLE(this->_serial_stream, 9);
      break;
    case U_FF_HASH:
      TOGGLE(this->_serial_stream, 8);
      break;
    case U_FB_HASH:
      TOGGLE(this->_serial_stream, 7);
      break;
    case U_HASH:
      TOGGLE(this->_serial_stream, 6);
      break;
    case PWR_HASH:
      TOGGLE(this->_serial_stream, 5);
      break;
    case EXT_ILU_HASH:
      TOGGLE(this->_serial_stream, 4);
      break;
    case FLICKER_HASH:
      TOGGLE(this->_serial_stream, 3);
      break;
    default:
      Serial.println("Var not found");
      break;
  }
}

/**
 * @brief Updates the circular buffer with state variables
 *
 */
void Luminary::update_hist() {
  this->DC.push((this->led.dutty_cicle / 100) * 65535);
  this->ref.push(this->contr.get_ref() * 1000);
  this->L_meas.push(this->ldr.lux * 1000);
  this->L_pred.push(this->sim.get_current_l_prediction() * 1000);
  this->integral.push(this->contr.get_integral() * 1000);
}

void Luminary::print_stream() {
  char buff[300];
  snprintf_P(buff, 300, "s ");
  if (READ(this->_serial_stream, 15)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "DC%d %.2f,", this->get_id(), this->led.dutty_cicle);
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 0]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "DC%d %.2f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 0]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 0] = false;
    }
  }
  if (READ(this->_serial_stream, 14)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "Ref%d %.3f,", this->get_id(), this->contr.get_ref());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 1]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "Ref%d %.3f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 1]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 1] = false;
    }
  }
  if (READ(this->_serial_stream, 13)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "L_meas%d %.5f,", this->get_id(), this->ldr.lux);
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 2]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "L_meas%d %.5f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 2]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 2] = false;
    }
  }
  if (READ(this->_serial_stream, 12)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "L_pred%d %.5f,", this->get_id(), this->sim.get_current_l_prediction());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 3]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "L_pred%d %.5f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 3]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 3] = false;
    }
  }
  if (READ(this->_serial_stream, 11)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "err%d %.7f,", this->get_id(), this->contr.get_ref() - this->ldr.lux);
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 4]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "err%d %.7f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 4]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 4] = false;
    }
  }
  if (READ(this->_serial_stream, 10)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "prop%d %.5f,", this->get_id(), (this->contr.get_ref() - this->ldr.lux) * this->contr.get_kp());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 5]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "prop%d %.5f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 5]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 5] = false;
    }
  }
  if (READ(this->_serial_stream, 9)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "int%d %.5f,", this->get_id(), this->contr.get_integral());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 6]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "int%d %.5f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 6]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 6] = false;
    }
  }
  if (READ(this->_serial_stream, 8)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "u_ff%d %.6f,", this->get_id(), (this->contr.get_ref() - this->sim.get_L0()) / this->sim.get_G());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 7]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "u_ff%d %.6f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 7]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 7] = false;
    }
  }
  if (READ(this->_serial_stream, 7)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "u_fb%d %.6f,", this->get_id(), (this->contr.get_ref() - this->ldr.lux) * this->contr.get_kp() + this->contr.get_integral());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 8]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "u_fb%d %.6f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 8]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 8] = false;
    }
  }
  if (READ(this->_serial_stream, 6)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "u%d %.7f,", this->get_id(), ((this->contr.get_ref() - this->sim.get_L0()) / this->sim.get_G()) + ((this->contr.get_ref() - this->ldr.lux) * this->contr.get_kp() + this->contr.get_integral()));
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 9]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "u%d %.7f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 9]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 9] = false;
    }
  }
  if (READ(this->_serial_stream, 5)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "pwr%d %.3f,", this->get_id(), this->get_curr_pwr());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 10]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "pwr%d %.3f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 10]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 10] = false;
    }
  }
  if (READ(this->_serial_stream, 4)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "ex_ilu%d %.5f,", this->get_id(), this->get_ext_ilu());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 11]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "ex_ilu%d %.5f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 11]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 11] = false;
    }
  }
  if (READ(this->_serial_stream, 3)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "flck%d %.4f,", this->get_id(), this->get_curr_flicker());
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 12]) {
      snprintf(buff + strlen(buff), 300 - strlen(buff), "flck%d %.4f,", I2C_message_protocol::nodes_addr[i], I2C_message_protocol::buff_recv_i2c_stream[i * N_STREAM_VARIABLES + 12]);
      I2C_message_protocol::recv_new_i2c_stream[i * N_STREAM_VARIABLES + 12] = false;
    }
  }
  if (strlen(buff) > 4) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "t%d %d",this->get_id(), millis());
    Serial.println(buff);
  }
}

void Luminary::send_i2c_stream() {
  if (READ(this->_i2c_stream, 15)) {
    I2C_message_protocol::send_stream(0x00, 0, this->led.dutty_cicle);
  }
  if (READ(this->_i2c_stream, 14)) {
    I2C_message_protocol::send_stream(0x00, 1, this->contr.get_ref());
  }
  if (READ(this->_i2c_stream, 13)) {
    I2C_message_protocol::send_stream(0x00, 2, this->ldr.lux);
  }
  if (READ(this->_i2c_stream, 12)) {
    I2C_message_protocol::send_stream(0x00, 3, this->sim.get_current_l_prediction());
  }
  if (READ(this->_i2c_stream, 11)) {
    I2C_message_protocol::send_stream(0x00, 4, this->contr.get_ref() - this->ldr.lux);
  }
  if (READ(this->_i2c_stream, 10)) {
    I2C_message_protocol::send_stream(0x00, 5, (this->contr.get_ref() - this->ldr.lux) * this->contr.get_kp());
  }
  if (READ(this->_i2c_stream, 9)) {
    I2C_message_protocol::send_stream(0x00, 6, this->contr.get_integral());
  }
  if (READ(this->_i2c_stream, 8)) {
    I2C_message_protocol::send_stream(0x00, 7, (this->contr.get_ref() - this->sim.get_L0()) / this->sim.get_G());
  }
  if (READ(this->_i2c_stream, 7)) {
    I2C_message_protocol::send_stream(0x00, 8, (this->contr.get_ref() - this->ldr.lux) * this->contr.get_kp() + this->contr.get_integral());
  }
  if (READ(this->_i2c_stream, 6)) {
    I2C_message_protocol::send_stream(0x00, 9, ((this->contr.get_ref() - this->sim.get_L0()) / this->sim.get_G()) + ((this->contr.get_ref() - this->ldr.lux) * this->contr.get_kp() + this->contr.get_integral()));
  }
  if (READ(this->_i2c_stream, 5)) {
    I2C_message_protocol::send_stream(0x00, 10, this->get_curr_pwr());
  }
  if (READ(this->_i2c_stream, 4)) {
    I2C_message_protocol::send_stream(0x00, 11, this->get_ext_ilu());
  }
  if (READ(this->_i2c_stream, 3)) {
    I2C_message_protocol::send_stream(0x00, 12, this->get_curr_flicker());
  }
}