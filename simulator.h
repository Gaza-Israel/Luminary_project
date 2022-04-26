
#ifndef Simulator_h
#define Simulator_h
#include <Arduino.h>

#define TAU_POINTS 5
#define SIMULATOR_FREQ 100  // Hz
#define V_HIST_SIZE 100
#define CALIBRATION_STEPS 20

class LED;
class LDR;

class Simulator {
 public:
  Simulator(LED *init_led, LDR *init_ldr);
  LED *led;
  LDR *ldr;

  void set_G(float m_gain);

  float get_G();
  float get_L0();

  float simulate_next_step(float last_v, float u, unsigned long dt);
  void sim_callback(float DC);
  float get_current_v_prediction();
  float get_current_l_prediction();
  void calibrate_G(int avrg_samples, bool fast_mode);
  void calibrate_tau(int avrg_samples);
  void calibrate_theta(int repeat);
  void test_G();
  float DC2Lux(float dc);

 private:
  void _print_state(bool verbose);
  int _calculate_tau(float dc, bool going_up);

  float _G = 0;
  float _K[N_LUMINARIES] = {0};
  float _L0 = 0;
  int _tau_up[TAU_POINTS];
  int _tau_down[TAU_POINTS];
  float _theta = 0;

  volatile float _last_v = 0;
  volatile int _v_pos_counter = 0;
  volatile float _v_hist[V_HIST_SIZE];
};
#endif
