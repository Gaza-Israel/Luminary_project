#include "Controller.h"

#include <Streaming.h>

#include "LDR.h"
#include "LED.h"
#include "simulator.h"
/**
 * @brief Construct a new Controller:: Controller object.
 * The controller is composed of a feedforward and a feedback branch.
 * The feedforward is bias and a gain, while the feedback is a PI with
 * antiwindup.
 *
 * @param led Pointer to the LED object (actuator)
 * @param ldr Pointer to the LDR object (sensor)
 * @param sim Pointer to the Simulator object (observer)
 */
Controller::Controller(LED *led, LDR *ldr, Simulator *sim) {
  this->_led = led;
  this->_ldr = ldr;
  this->_sim = sim;
}
/**
 * @brief Calculates the next feedback control action
 * 
 */
void Controller::compute_fdbk_ctrl_action() {
  //   float e = this->_ref - this->_ldr->lux;
  this->_e = this->_sim->get_current_l_prediction() - this->_ldr->lux;
  this->_integral += this->_ki * this->_e / CONTROLLER_FREQ;
  this->_u_fb = this->_kp * this->_e + this->_integral;
}
/**
 * @brief Calculates the next feedforward control action
 *
 */
void Controller::compute_ffwrd_ctrl_action() {
  this->_u_ff = max(0, min(100, (this->get_ref() - this->_sim->get_L0()) / this->_sim->get_G()));
}
/**
 * @brief Checks if the controller requested actuation 
 * is within limits and performs the antiwindup saturation of the integral
 * 
 */
void Controller::check_integral_limits() {
  this->_u = this->_fb_on * this->_u_fb + this->_ff_on * this->_u_ff;
  if ((this->_u > 100 || this->_u < 0) && this->_ki != 0) {
    float sat_max = 100 - this->_u_ff - this->_kp * this->_e;
    float sat_min = 0 - this->_u_ff - this->_kp * this->_e;
    if (this->_u > 100) {
      this->_integral = min(this->_integral, sat_max);
    } else {
      this->_integral = max(sat_min, this->_integral);
    }
    this->_u_fb = this->_kp * this->_e + this->_integral;
    this->_u = this->_fb_on * this->_u_fb + this->_ff_on * this->_u_ff;
  }
  this->_u = max(min(this->_u, 100), 0);
}
/**
 * @brief Sets the controller requested actuation on the LED.
 * If the antinwindup is on, checks the controller saturation 
 * before setting the LED duty cycle
 *
 */
void Controller::update_led_DC() {
  if (this->_anti_windup_on) {
    check_integral_limits();
  }
  this->_led->set_dutty_cicle(this->_u);
}
/**
 * @brief Set the feedback branch on or off
 * 
 * @param state - Desired state (bool)
 */
void Controller::set_fb(bool state) {
  this->_fb_on = state;
}
/**
 * @brief Set the feedforward branch on or off
 *
 * @param state - Desired state (bool)
 */
void Controller::set_ff(bool state) {
  this->_ff_on = state;
}
/**
 * @brief Set the occupancy of the desk on or off
 *
 * @param state - Desired state (bool)
 */
void Controller::set_occ(bool state) {
  this->_occ = state;
}
/**
 * @brief Set the antiwindup on or off
 *
 * @param state - Desired state (bool)
 */
void Controller::set_anti_windup(bool state) {
  this->_anti_windup_on = state;
}
/**
 * @brief Set the feedback gains ('kp' and 'ki')
 * 
 * @param kp Proportional gain
 * @param ki Integral gain (optional, default value 0)
 * 
 */
void Controller::set_gains(float kp, float ki /* = 0*/) {
  this->_kp = kp;
  this->_ki = ki;
}
/**
 * @brief Set the controller reference value
 * 
 * @param ref Desired reference value
 * @param occ Occupancy status, 
 * if 1 changes the occupied reference value, 
 * if 0 the unnoccupied reference value
 */
void Controller::set_ref(float ref, bool occ /* = true*/) {
  if (occ) {
    this->_up_ref = ref;
  } else {
    this->_low_ref = ref;
  }
}
/**
 * @brief Gets the current 'kp' value
 * 
 * @return float - Proportional gain
 */
float Controller::get_kp() {
  return this->_kp;
}
/**
 * @brief Gets the current 'ki' value
 *
 * @return float - Integral gain
 */
float Controller::get_ki() {
  return this->_ki;
}
/**
 * @brief Gets the current 'integral' value
 *
 * @return float - Accumulated error (integral)
 */
float Controller::get_integral() {
  return this->_integral;
}
/**
 * @brief Gets the current reference ('ref') value, based on the occupancy status
 *
 * @return float - Reference
 */
float Controller::get_ref() {
  if (this->_occ) {
    return this->_up_ref;
  } else {
    return this->_low_ref;
  }
}
/**
 * @brief Gets the current error ('e') value
 *
 * @return float - error
 */
float Controller::get_e() {
  return this->_e;
}

/**
 * @brief Gets the current feedback actuation value
 *
 * @return float - Feedback actuation value
 */
float Controller::get_u_fb() {
  return this->_u_fb;
}

/**
 * @brief Gets the current feedforward actuation value
 *
 * @return float - Feedforward actuation value
 */
float Controller::get_u_ff() {
  return this->_u_ff;
}

/**
 * @brief Gets the current occupancy status
 *
 * @return float - Occupancy status
 */
bool Controller::get_occ() {
  return this->_occ;
}

/**
 * @brief Gets the current antiwindup status
 *
 * @return float - Antiwindup status
 */
bool Controller::get_anti_windup_status() {
  return this->_anti_windup_on;
}