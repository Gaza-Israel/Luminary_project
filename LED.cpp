#include "LED.h"

#define SS_ON_TIME 200
#define SS_BLINKS 20

/**
 * @brief Construct a new LED::LED object
 * 
 * @param pin LED positive pin (needs to be PWM pin)
 */
LED::LED(int pin) {
  pinMode(pin, OUTPUT);
 _pin = pin;
}

/**
 * @brief Set the PWM duty cycle, changing the LED brighness
 * 
 * @param DC Desired duty cycle value (0-100)
 */
void LED::set_dutty_cicle(float DC){
  analogWrite(this->_pin, DC*4095/100);
  this->dutty_cicle = DC; 
}
/**
 * @brief Performs a start sequence on the LED, blinking it 
 * to signal Serial read to connect.
 * 
 */
void LED::start_sequence(){
  for (int i = 0; i<SS_BLINKS; i++){
    this->set_dutty_cicle(100);
    sleep_ms(SS_ON_TIME-SS_ON_TIME/SS_BLINKS*i);
    this->set_dutty_cicle(0);
    sleep_ms(SS_ON_TIME-SS_ON_TIME/SS_BLINKS*i);
  }
   this->set_dutty_cicle(100);
   sleep_ms(200);
   Serial.println("Board Started");
}
