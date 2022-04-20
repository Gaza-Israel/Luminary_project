#ifndef LED_h
#define LED_h
#include "Arduino.h"
#define AVRG_LED_MAX_CURRENT 0.0112
class LED {
public:
  LED(int pin);
  float dutty_cicle;
  void set_dutty_cicle(float DC);
  void start_sequence();
private:
  int _pin;
};
#endif
