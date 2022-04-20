#include "Simulator.h"

#include <BasicLinearAlgebra.h>
#include <Streaming.h>

#include "LDR.h"
#include "LED.h"
#include "median_filter.h"

#define CALIBRATION_POINTS 20
#define CALIBRATION_STEPS 20
#define STEADY_STATE_DELAY 300
#define ADC_FREQ 10000
// #define PRINT_STATE
#define DEBUG

/**
 * @brief Construct a new Simulator:: Simulator object
 * 
 * @param init_led LED pointer
 * @param init_ldr LDR pointer
 */
Simulator::Simulator(LED *init_led, LDR *init_ldr) {
  this->led = init_led;
  this->ldr = init_ldr;
}
/**
 * @brief Sets the Plant gain (G)
 * 
 * @param G_gain Plant gain
 */
void Simulator::set_G(float G_gain) {
  this->_G = G_gain;
}
/**
 * @brief Gets the Plant gain
 * 
 * @return float - Plant gain
 */
float Simulator::get_G() {
  return this->_G;
}
/**
 * @brief Gets the ambient base illuminance
 * 
 * @return float - L0
 */
float Simulator::get_L0() {
  return this->_L0;
}
/**
 * @brief Converts the duty cycle to expected Lux value
 * 
 * @param dc duty cycle
 * @return float - Expected Lux value
 */
float Simulator::DC2Lux(float dc){
  return this->get_G() * dc + this->_L0;
}
/**
 * @brief Calculate next simulater step
 * 
 * @param last_v Last predicted LDR voltage
 * @param u Current LED actuation
 * @param dt Time step
 * @return float - predicted LDR voltage
 */
float Simulator::simulate_next_step(float last_v, float u, unsigned long dt) {
  const float L = this->DC2Lux(u);
  const int R2 = this->ldr->lux2resistance(L);
  const float Vf = this->ldr->resistance2voltage(R2);
  const int tau = this->_calculate_tau(u, Vf > last_v);
  const float dt_tau = float(dt) / (tau);
  float next_v = last_v * (1 - dt_tau) + Vf * dt_tau;
  return next_v;
}
/**
 * @brief Callback to perform the simulation, should be called every cycle
 * 
 * @param DC Current duty cycle of the LED
 */
void Simulator::sim_callback(float DC) {
  this->_last_v = this->simulate_next_step(this->_last_v, DC, 1000000 / SIMULATOR_FREQ);
  this->_v_hist[this->_v_pos_counter++] = this->_last_v;
  if (this->_v_pos_counter > V_HIST_SIZE - 1) {
    this->_v_pos_counter = 0;
  }
}
/**
 * @brief Gets the prediction in volts for the current time step from the circular 
 * buffer accounting for the Theta delay
 * 
 * @return float - Current prediction
 */
float Simulator::get_current_v_prediction() {
  int idx_theta_delay = int(this->_theta * SIMULATOR_FREQ / 1000000);
  int idx = this->_v_pos_counter - 1 - idx_theta_delay;
  if (idx < 0) {
    idx = V_HIST_SIZE + idx;
  }
  return this->_v_hist[idx];
}
/**
 * @brief Gets the prediction in LUX for the current time step from the circular
 * buffer accounting for the Theta delay
 *
 * @return float - Current prediction
 */
float Simulator::get_current_l_prediction() {
  float v = this->get_current_v_prediction();
  v = max(v, 0.00001);
  float r = this->ldr->voltage2resistance(v);
  float lux = this->ldr->resistance2lux(r);
  return lux;
}
/**
 * @brief Calculates the Tau value for the current state
 * 
 * @param dc Duty cycle
 * @param going_up Bool that indicates if the voltage is going up or down
 * @return int - Tau value
 */
int Simulator::_calculate_tau(float dc, bool going_up) {
  float tau_interval = 100 / TAU_POINTS;
  float index_decimal = dc / tau_interval;
  float w = index_decimal - floor(index_decimal);
  float low_tau;
  float high_tau;
  if (going_up) {
    low_tau = this->_tau_up[int(floor(index_decimal))];
    high_tau = this->_tau_up[int(floor(index_decimal)) + 1];
  } else {
    low_tau = this->_tau_down[int(floor(index_decimal))];
    high_tau = this->_tau_down[int(floor(index_decimal)) + 1];
  }
  return low_tau * (1 - w) + high_tau * w;
}
/**
 * @brief Calibrates the plant gain (G)
 * 
 * @param avrg_samples Number of samples to filter with median
 * @param fast_mode If fast mode perform simple calibration. 
 */
void Simulator::calibrate_G(int avrg_samples, bool fast_mode) {
#ifdef DEBUG
  Serial.println("Starting Calibration...");
#endif
  double L0;

  // Calculating minimum LUX in this ambient (L0)
  this->led->set_dutty_cicle(0);
  sleep_ms(STEADY_STATE_DELAY);

  this->ldr->average_measure(avrg_samples);

  L0 = this->ldr->lux;
  this->_L0 = L0;

#ifdef DEBUG
  Serial << "Minimum Lux (*10) = " << _FLOAT(L0 * 10, 5) << endl;
#endif
  if (fast_mode) {
    // Calculating maximum LUX in this ambient
    this->led->set_dutty_cicle(100);
    sleep_ms(STEADY_STATE_DELAY);

    this->ldr->median_measure(avrg_samples);
    this->_G = (this->ldr->lux - L0) / 100;
  } else {
    BLA::Matrix<CALIBRATION_POINTS, 1> DC;
    BLA::Matrix<CALIBRATION_POINTS, 1> L;
    BLA::Matrix<1, 1> square;
    BLA::Matrix<1, 1> Gain;
    int idx = 0;

    // Calibrating dutty cycle input to Lux gain
    for (int i = 1; i <= CALIBRATION_STEPS; i++) {
      // Set the new LED dutty cycle and sleeps for 50 ms to wait steady state
      this->led->set_dutty_cicle(i * 100 / CALIBRATION_STEPS);
      sleep_ms(STEADY_STATE_DELAY);

      // Measure the defined number of points
      for (int j = 0; j < floor(CALIBRATION_POINTS / CALIBRATION_STEPS); j++) {
        this->ldr->median_measure(avrg_samples);

        // Stores the measured data on the matrix
        DC(idx, 0) = this->led->dutty_cicle;
        L(idx, 0) = 10 * (-L0 + this->ldr->lux);
        idx++;
#ifdef PRINT_STATE
        this->_print_state(false);
#endif
      }
    }

    square = ~DC * DC;
#ifdef DEBUG
    Serial << "DC: " << DC << '\n';
    Serial << "L: " << L << '\n';
    Serial << "square: " << _FLOAT(square(0, 0), 8) << '\n';
#endif
    bool is_nonsingular = BLA::Invert(square);
    if (is_nonsingular) {
      Gain = (square * ~DC) * L;
      this->_G = Gain(0) / 10;
#ifdef DEBUG
      Serial << "inverted square: " << _FLOAT(square(0, 0), 8) << '\n';
      Serial.print("Calibration ended, Gain = ");
      Serial.println(this->_G, 5);
#endif
    } else {
#ifdef DEBUG
      Serial.println("ERROR: Singular matrix");
#endif
    }
  }
}
/**
 * @brief Calibrate the RC time constante Tau
 *
 * @param avrg_samples  Number of samples to filter with median sliding window
 */
void Simulator::calibrate_tau(int avrg_samples) {
#ifdef DEBUG
  Serial.println("Starting Tau calibration...");
#endif

  const int array_size = int(STEADY_STATE_DELAY * ADC_FREQ / 1000);
  float dc_target = 0;

#ifdef DEBUG
  Serial << "Alocating variables with size " << array_size;
#endif

  int voltages[array_size];
  int f_voltages[array_size];
  unsigned int times[array_size];
  unsigned long t0;

  int j = 0;
  bool starting_from_0 = false;

#ifdef DEBUG
  Serial << "....OK" << endl
         << "Starting cycles\n";
#endif
  for (int c = 0; c < 2; c++) {
    starting_from_0 = !starting_from_0;
    // Duttty Cycle for
    for (int i = 0; i <= TAU_POINTS; i++) {
      dc_target = i * 100 / TAU_POINTS;
      // Set the innitial DD based on the greatest distance (maximize delta V)
      if (!starting_from_0) {
#ifdef DEBUG
        Serial << "Starting from 100" << endl
               << "------------------------" << endl;
#endif
        this->led->set_dutty_cicle(100);
        sleep_ms(STEADY_STATE_DELAY);
      } else {
#ifdef DEBUG
        Serial << "Starting from 0" << endl
               << "------------------------" << endl;
#endif
        this->led->set_dutty_cicle(0);
        sleep_ms(STEADY_STATE_DELAY);
      }

      j = 0;

#ifdef PRINT_STATE
      Serial << "timestamp,idx,t,v,v_f,f" << endl;
#endif
      t0 = micros();
      times[j] = 0;
      voltages[j++] = this->ldr->read_adc();

      // Set the new LED dutty cycle
      this->led->set_dutty_cicle(dc_target);

      do {
        while ((micros() - t0) - times[j - 1] < 1000000 / ADC_FREQ) {
        }
        times[j] = micros() - t0;
        voltages[j] = this->ldr->read_adc();
      } while (++j < array_size);

      f_voltages[0] = voltages[0];
#ifdef PRINT_STATE
      Serial << "0 - " << times[0] << "," << voltages[0] << "," << f_voltages[0] << "," << 0 << endl;
#endif

      // Filteringg
      for (int k = 1; k < j; k++) {
        if (k < avrg_samples) {
          f_voltages[k] = median(voltages, 0, k + 1);
        } else {
          f_voltages[k] = median(voltages, k - avrg_samples, avrg_samples + 1);
        }
#ifdef PRINT_STATE
        Serial << k << " - " << times[k] << "," << voltages[k] << "," << f_voltages[k] << "," << 1000000 / (times[k] - times[k - 1]) << endl;
#endif
      }

      int idx = 0;
      int v_target = 0.63 * (f_voltages[array_size - 1] - f_voltages[0]) + f_voltages[0];
      while (starting_from_0 && (v_target > f_voltages[idx]) || (!starting_from_0 && (v_target < f_voltages[idx]))) {
        idx++;
      }
      if (starting_from_0) {
        this->_tau_up[i] = times[idx];
      } else {
        this->_tau_down[i] = times[idx];
      }
    }
  }
  this->_tau_up[0] = this->_tau_down[0];
  this->_tau_down[TAU_POINTS - 1] = this->_tau_up[TAU_POINTS - 1];
#ifdef DEBUG
  Serial << "Tau_up: ";
  for (int i = 0; i < TAU_POINTS; i++) {
    Serial << _FLOAT(this->_tau_up[i], 8) << ", ";
  }
  Serial << "\nTau_down: ";
  for (int i = 0; i < TAU_POINTS; i++) {
    Serial << _FLOAT(this->_tau_down[i], 8) << ", ";
  }
#endif
}
/**
 * @brief Calibrates the theta delay
 * 
 * @param repeat Number of repetitions of the calibration to average the result
 */
void Simulator::calibrate_theta(int repeat) {
#ifdef DEBUG
  Serial << "Starting theta calibration\n";
#endif
  int i = 0;

  const int size_array = 2000;
  const int start_led = 1000;
  const int avrg_samples = 5;

  int v[size_array];
  int v_f[size_array];
  unsigned long t[size_array];

  float v0;

  unsigned long start;

  this->_theta = 0;
  for (int j = 0; j < repeat; j++) {
    this->led->set_dutty_cicle(0);
    sleep_ms(STEADY_STATE_DELAY);

    for (int n = 0; n < start_led; n++) {
      v[n] = this->ldr->read_adc();
      t[n] = micros();
    }

    start = micros();
    this->led->set_dutty_cicle(100);

    for (int n = start_led; n < size_array; n++) {
      v[n] = this->ldr->read_adc();
      t[n] = micros();
    }

    for (int n = 0; n < size_array; n++) {
      if (n < avrg_samples) {
        v_f[n] = median(v, 0, n + 1);
      } else {
        v_f[n] = median(v, n - avrg_samples, avrg_samples + 1);
      }
      if (n < start_led) {
        v0 += v_f[n] / start_led;
      }
    }

    for (int k = 1; k < j; k++) {
#ifdef PRINT_STATE
      Serial << "start voltage = " << v0 << endl;
#endif
    }
#ifdef PRINT_STATE
    for (int n = 0; n < size_array; n++) {
      Serial << t[n] << "," << v_f[n] << endl;
    }
#endif
    i = start_led;
    while (abs(v_f[i] - v0) < 12) {
      i++;
    }
    unsigned long delta = t[i] - start;
    this->_theta += float(delta) / repeat;

#ifdef DEBUG
    Serial << "theta prov = " << delta << "," << i << endl;
#endif
  }
#ifdef PRINT_STATE
  Serial << "theta = " << this->_theta << endl;
#endif
}
/**
 * @brief Test the current plant gain to compare the predicted with the measured Lux
 * 
 */
void Simulator::test_G() {
#ifdef DEBUG
  Serial.println("Starting Gain test...");
#endif
  this->led->set_dutty_cicle(0);
  sleep_ms(STEADY_STATE_DELAY);

  // Stepping dutty cycle
  for (int i = 0; i <= 100; i += 2) {
    // Set the new LED dutty cycle and sleeps for 50 ms to wait steady state
    this->led->set_dutty_cicle(i);
    sleep_ms(STEADY_STATE_DELAY);
    // Measure the defined number of points
    for (int j = 0; j < 1; j++) {
      this->ldr->median_measure(50);
      this->_print_state(true);
    }
  }
}
/**
 * @brief Print some state variables.
 * Deprecated - use stream method on parser.
 *
 * @param verbose Activates the verbose mode (prints more variables)
 */
void Simulator::_print_state(bool verbose) {
  char buff[120];
  if (verbose) {
    sprintf(buff, "[]-LED(%%/10):%.3f, V(V*10):%.5f,  R(Ohms/100):%.5f,  L(Lux*10):%.5f, L_prev(Lux*10):%.5f", float(this->led->dutty_cicle) / 10, 10 * this->ldr->voltage, float(this->ldr->resistance) / 100, 10 * this->ldr->lux, 10 * (this->_L0 + this->_G * this->led->dutty_cicle));
  } else {
    sprintf(buff, "[]-LED(%%/10):%.3f, L(Lux*10):%.5f, L_prev(Lux*10):%.5f", float(this->led->dutty_cicle) / 10, 10 * this->ldr->lux, 10 * (this->_L0 + this->_G * this->led->dutty_cicle));
  }
  Serial.println(buff);
}