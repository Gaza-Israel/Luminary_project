
#ifndef Controller_h
#define Controller_h
#include <Arduino.h>
class LED;
class LDR;
class Simulator;

#define CONTROLLER_FREQ SIMULATOR_FREQ

class Controller {
 public:
  Controller(LED *init_led, LDR *init_ldr, Simulator *sim);

  void compute_fdbk_ctrl_action();
  void compute_ffwrd_ctrl_action();
  void check_integral_limits();
  void set_gains(float kp, float ki = 0);
  float get_kp();
  float get_ki();
  float get_integral();
  float get_ref();
  float get_e();
  float get_u_fb();
  float get_u_ff();
  bool get_u_fb_status();
  bool get_u_ff_status();
  bool get_occ();
  bool get_anti_windup_status();
  void set_ref(float ref, bool occ = true);

  void set_fb(bool state);
  void set_ff(bool state);
  void set_occ(bool state);
  void set_anti_windup(bool state);
  void update_led_DC();


 private:
  LED *_led;
  LDR *_ldr;
  Simulator *_sim;
  float _up_ref = 0;
  float _low_ref = 0;
  float _e = 0;
  float _kp = 0;
  float _ki = 0;
  float _integral = 0;
  float _u_fb = 0;
  float _u_ff = 0;
  float _u = 0;
  bool _fb_on = false;
  bool _ff_on = false;
  bool _occ = true;
  bool _anti_windup_on = true;
};
#endif