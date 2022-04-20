#ifndef Luminary_h
#define Luminary_h
#include <Arduino.h>

#include "Controller.h"
#include "LDR.h"
#include "LED.h"
#include "Simulator.h"

#define SECUNDARY_TIMER_FREQ 100  // Hz

class Luminary {
 private:
  int _id;
  float _external_iluminance;
  float _curr_pwr;
  float _acc_energy = 0;
  float _acc_visibility_err = 0;
  float _acc_flicker = 0;
  unsigned long _acc_counter = 0;
  float _curr_flicker = 0;

 public:
  Luminary(int pin_led, int pin_ldr, int id);
  LED led;
  LDR ldr;
  Simulator sim;
  Controller contr;
  void print_state(bool verbose);
  float get_ext_ilu();  //
  float get_curr_pwr();
  float get_acc_energy();
  float get_acc_visibility_err();
  float get_acc_flicker();
  float get_curr_flicker();
  void calc_metrics(float l, float l1, float l2);
  void calc_ext_ilu();
  void calc_pwr();
  void reset_metrics();
  int get_id();
  void set_id(int id);
};
#endif
