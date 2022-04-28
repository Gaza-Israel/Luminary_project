#include <Streaming.h>

#include "Controller.h"
#include "I2C.h"
#include "I2C_message_protocol.h"
#include "LDR.h"
#include "LED.h"
#include "Luminaire.h"
#include "Parser.h"
#include "Simulator.h"
#include "minimizer.h"

/*
  TODO
  Switch conversions on LDR to inline functions
 */

// The AUTO_TEST define activates the base reference curve,
//  it is used to help the controller tunning process

// #define AUTO_TEST
#ifdef AUTO_TEST
float dc_target[] = {100, 0, 10, 30, 0, 80, 55, 29, 85, 40, 72, 10};
int sleep[] = {400, 100, 350, 90, 130, 280, 800, 500, 700, 100, 300, 500};
int kp_v[] = {0, 10, 30, 50, 70};
int ki_v[] = {0, 300, 500, 700, 900};
int kp_counter = -1;
int ki_counter = 4;
int counter = 11;
bool change_kp = false;
#endif

uint8_t ID = 0;
Luminary L1(16, 26, 1);             // creates a Luminary object with LED on pin 16 and LDR on pin 26
Parser myparser;                    // Parser for serial commands
struct repeating_timer main_timer;  // Main timer structure
I2C::i2c_message tx_message;

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);  // 12 bit resolution (default is 10 ...)
  analogWriteFreq(30000);    // 30KHz
  analogWriteRange(4095);    // Max PWM value (correspods to 100%)

  pass_lum_to_parser(&L1);  // Passes the Luminaire object to the parser
  I2C_message_protocol::L = &L1;
  Minimizer::L = &L1;
  myparser.setup();         // Setup the commands on the parser
  L1.led.start_sequence();  // Blinks the led to signal its ready for serial connections

  I2C::config_I2C(12, 13, 10, 11);

  I2C::print_I2C1_full_address();
  Serial.println(I2C::get_I2C1_address());

  L1.set_id(I2C::get_I2C1_address());

  // L1.wk.calibrate_all();

  L1.contr.set_gains(50, 500);
  L1.contr.set_fb(true);
  L1.contr.set_ff(true);
  L1.contr.set_anti_windup(true);
  L1.contr.set_ref(0);

  add_repeating_timer_us(-1000000 / SIMULATOR_FREQ, main_timer_callback, NULL, &main_timer);
  Serial.println("Ready");
#ifdef AUTO_TEST
  Serial.println("DC,Ref,L_meas,L_pred,err,prop,in,u_ff,u_fb,u,pwr,ex_ilu,flck,t");
#endif
}
void loop() {
#ifdef AUTO_TEST
  L1.contr.set_ref(dc_target[counter] * 1.2 / 100 + 0.15);
  sleep_ms(sleep[counter++]);
  if (counter == 12) {
    counter = 0;
    L1.contr.set_ref(0);
    if (kp_counter == 4 && ki_counter == 4) {
      noInterrupts();
      Serial.println("___________________________________________________________");
      Serial.println("___________________________________________________________");
      sleep_ms(20000);
      kp_counter = -1;
      ki_counter = 4;
    }
    Serial.println("___________________________________________________________");

    Serial.println("DC,Ref,L_meas,L_pred,err,prop,in,u_ff,u_fb,u,pwr,ex_ilu,flck,t");
    char buff[128];
    if (ki_counter == 4) {
      ki_counter = 0;
      change_kp = true;
    }
    if (change_kp) {
      L1.contr.set_gains(kp_v[++kp_counter], ki_v[ki_counter]);
      change_kp = false;
    } else {
      L1.contr.set_gains(kp_v[kp_counter], ki_v[++ki_counter]);
    }

    sprintf(buff, "kp = %d, ki = %d", kp_v[kp_counter], ki_v[ki_counter]);
    Serial.println(buff);
    sleep_ms(300);
  }
#endif
  /*
   * The default loop only handles the parsing of commands,
   * if there is a new serial message available it stores the line
   * and passes it to the parser. Then it waits for the command to
   * be handled and prints the return of the parser
   */
  if (Serial.available()) {
    char line[128];
    size_t lineLength = Serial.readBytesUntil('\n', line, 127);
    line[lineLength] = '\0';

    char response[MyCommandParser::MAX_RESPONSE_SIZE];
    myparser._parser.processCommand(line, response);
    Serial.println(response);
  }
  if (I2C::buffer_not_empty()) {
    I2C_message_protocol::parse_message(I2C::pop_message_from_buffer());
  }
}
