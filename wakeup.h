#ifndef Wakeup_h
#define Wakeup_h
#include <Arduino.h>
class LED;
class LDR;
class Simulator;

class wake_up {
 public:
  wake_up(LED *led,LDR *init_ldr, Simulator *sim);
  void calibrate_all();
  void self_calibration();
  void other_calibration();
  void ready_to_calibrate();
  void wait_for_message(uint8_t _message_id);
  bool check_ready(bool* ready);
 private:
  LED *_led;
  LDR *_ldr;
  Simulator *_sim;
  bool _waiting_for_crossed_g = 1;
};

#endif