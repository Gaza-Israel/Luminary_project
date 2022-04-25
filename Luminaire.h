#ifndef Luminary_h
#define Luminary_h
#include <Arduino.h>
#include <CircularBuffer.h>

#include "Controller.h"
#include "LDR.h"
#include "LED.h"
#include "Simulator.h"

#define SECUNDARY_TIMER_FREQ 100  // Hz
// #define N_STREAM_VARIABLES 13 - moved to I2C_message_protocol
#define DC_HASH 5863308
#define REF_HASH 177687
#define L_MEAS_HASH 193498321
#define L_PRED_HASH 5863585
#define ERR_HASH 177674
#define PROP_HASH 177685
#define INTEGRAL_HASH 193495088
#define U_FF_HASH 193507878
#define U_FB_HASH 193507874
#define U_HASH 177690
#define PWR_HASH 5863724
#define EXT_ILU_HASH 5863974
#define FLICKER_HASH 5863383

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
  uint16_t _i2c_stream = 0;
  uint16_t _serial_stream = 0;

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
  void toggle_i2c_stream(unsigned long hash);
  void toggle_serial_stream(unsigned long hash);
  
  void update_hist();
  void print_stream();
  void send_i2c_stream();

  CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> DC;        // 0 - 65535 (0-100% times 65535/100)
  CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> ref;       // (ref * 1000)
  CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> L_meas;    // (L_meas * 1000)
  CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> L_pred;    // (L_pred * 1000)
  CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> integral;  //(integral * 1000)
};
#endif
