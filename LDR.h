#ifndef LDR_h
#define LDR_h
#include "Arduino.h"

#define R1 9760
// #define R1 10000
#define Vcc 3.3
class LDR {
 public:
  LDR(int pin);
  double voltage;
  int resistance;
  double lux;
  int read_adc();
  void read_voltage();
  void read_and_convert();
  float voltage2resistance(float volt);
  float resistance2voltage(float res);
  float resistance2lux(float res);
  float lux2resistance(float l);

  void average_measure(int n);
  void median_measure(int n);
  
  void set_coefficients(float m, float b);
  float get_m();
  float get_b();
 private:
  int _pin;
  float _m;
  float _b;
};
#endif
