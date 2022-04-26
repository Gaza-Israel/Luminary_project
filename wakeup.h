#ifndef Wakeup_h
#define Wakeup_h
#include <Arduino.h>
class LDR;
class Simulator;

class wake_up {
 public:
  wake_up(LDR *init_ldr, Simulator *sim);
  void calibrate_all();
  void self_calibration();
  void other_calibration();
  void ready_to_calibrate();
  void wait_for_message(uint8_t _message_id);

 private:
  LDR *_ldr;
  Simulator *_sim;
  bool _waiting_for_crossed_g = 1;
};

#endif