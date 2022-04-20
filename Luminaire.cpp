#include "Luminaire.h"

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
      contr(&this->led, &this->ldr, &this->sim) {
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
  return this->_acc_visibility_err/this->_acc_counter;
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
void Luminary::set_id(int id){
  this->_id = id;
}