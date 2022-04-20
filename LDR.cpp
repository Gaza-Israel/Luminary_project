#include "LDR.h"


#include "median_filter.h"
/**
 * @brief Construct a new LDR::LDR object
 * 
 * @param pin LDR pin (needs to be and ADC pin)
 */
LDR::LDR(int pin) {
  pinMode(pin, INPUT);
  _pin = pin;
}
/**
 * @brief Reads the LDR ADC pin
 *
 * @return int - ADC value (0-4095)
 */
int LDR::read_adc() {
  return analogRead(_pin);
}
/**
 * @brief Reads the ADC and converts its value to voltage.
 * Stores the result on the voltage field of the LDR object
 */
void LDR::read_voltage() {
  this->voltage = this->read_adc() * Vcc / 4095;
}
/**
 * @brief Converts the voltage value to LDR resistance
 * 
 * @param volt Voltage on negative pin of the LDR
 * @return float - Resistance of the LDR
 */
float LDR::voltage2resistance(float volt) {
  return (R1 * (-1 + Vcc / volt));
}
/**
 * @brief Converts LDR resistance to voltage on its negative pin
 * 
 * @param res Resistance of the LDR
 * @return float - Voltage on the negative pinof the LDR
 */
float LDR ::resistance2voltage(float res) {
  return R1 * Vcc / (R1 + res);
}
/**
 * @brief Converts LDR resistance to Lux
 * 
 * @param res LDR resistance
 * @return float - Lux
 */
float LDR::resistance2lux(float res) {
  return pow(10, ((log10(res) - this->_b) / this->_m));  //+3 is due to resistance being in kOhms
}
/**
 * @brief Converts Lux values to LDR resistance
 * 
 * @param l Lux value
 * @return float - LDR resistance
 */
float LDR::lux2resistance(float l) {
  return pow(10, this->_m * log10(l) + this->_b);
}
/**
 * @brief Reads ADC and performs all conversion until the Lux output
 * Stores Lux, resistance, and voltage on the respective object fields
 */
void LDR::read_and_convert() {
  this->read_voltage();
  this->resistance = this->voltage2resistance(this->voltage);
  this->lux = this->resistance2lux(this->resistance);
}
/**
 * @brief Sets the LDR coefficients for the characteristic curve
 *
 * @param m Angular coefFicient
 * @param b Linear coefFicient
 */
void LDR::set_coefficients(float m, float b) {
  Serial.print(".");
  this->_m = m;
  Serial.print(".");
  this->_b = b;
  Serial.print(".");
}
/**
 * @brief Measures 'n' values and outputs the average result
 * 
 * @param n Number of values to average
 */
void LDR::average_measure(int n) {
  double voltage = 0, lux = 0;
  long resistance = 0;
  for (int i = 0; i < n; i++) {
    this->read_and_convert();
    voltage += this->voltage;
    resistance += long(this->resistance);
    lux += this->lux;
  }
  this->voltage = voltage / n;
  this->resistance = int(resistance / n);
  this->lux = lux / n;
}
/**
 * @brief Measures 'n' values and outputs its median
 * 
 * @param n Number of values to measure
 */
void LDR::median_measure(int n) {
  int v[n];
  for (int i = 0; i < n; i++) {
    v[i] = this->read_adc();
  }
  this->voltage = median(v, 0, n)*Vcc/4095;
  this->resistance = this->voltage2resistance(this->voltage);
  this->lux = this->resistance2lux(this->resistance);
}
/**
 * @brief Gets the value of the 'm' coefficient
 * 
 * @return float - m
 */
float LDR::get_m() {
  return this->_m;
}

/**
 * @brief Gets the value of the 'b' coefficient
 *
 * @return float - b
 */
float LDR::get_b() {
  return this->_b;
}