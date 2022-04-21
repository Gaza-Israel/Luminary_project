#include "Parser.h"

#include <CircularBuffer.h>

#include "I2C.h"
#include "Luminaire.h"
#include "Streaming.h"

#define DC_HASH 5863308
#define REF_HASH 177687
#define L_MEAS_HASH 193498321
#define L_PRED_HASH 5863585
#define ERR_HASH 177674
#define PROP_HASH 177685
#define INTEGRAL_HASH 193495088
#define U_FF_HASH 193507878
#define U_FB_HASH 193507874
#define U 177690
#define PWR_HASH 5863724
#define EXT_ILU_HASH 5863974
#define FLICKER_HASH 5863383
/*
dc=5863308
r=177687
lms=193498321
lp=5863585
e=177674
p=177685
int=193495088
uff=193507878
ufb=193507874
u=177690
pw=5863724
xi=5863974
fl=5863383
*/

#define SET(byte, nbit) ((byte) |= (1 << (nbit)))
#define CLEAR(byte, nbit) ((byte) &= ~(1 << (nbit)))
#define TOGGLE(byte, nbit) ((byte) ^= (1 << (nbit)))
#define READ(byte, nbit) ((byte) & (1 << (nbit)))

Luminary *G_L1;
uint16_t stream = 0;
CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> DC;        // 0 - 65535 (0-100% times 65535/100)
CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> ref;       // (ref * 1000)
CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> L_meas;    // (L_meas * 1000)
CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> L_pred;    // (L_pred * 1000)
CircularBuffer<uint16_t, 60 * CONTROLLER_FREQ> integral;  //(integral * 1000)
// CircularBuffer<float, 60 * CONTROLLER_FREQ> pwr;
// CircularBuffer<float, 60 * CONTROLLER_FREQ> ext_ilu;
// CircularBuffer<float, 60 * CONTROLLER_FREQ> flicker;

/* stream legend
 *  bit | 15 | 14  |   13   |   12   |  11 |  10  |  9  |   8  |   7  | 6 |  5  |    4    |    3    | 2 | 1 | 0
 *  var | DC | ref | L_meas | L_pred | err | prop | int | u_ff | u_fb | u | pwr | ext_ilu | flicker | - | - | -
 */

// #define AUTO_TEST
/**
 * @brief Construct a new Parser:: Parser object
 *
 */
Parser::Parser() {
}
/**
 * @brief Set up the parser, registering the available commands and its callbacks
 *
 */
void Parser::setup() {
#ifdef AUTO_TEST
  TOGGLE(stream, 15);
  TOGGLE(stream, 14);
  TOGGLE(stream, 13);
  TOGGLE(stream, 12);
  TOGGLE(stream, 11);
  TOGGLE(stream, 10);
  TOGGLE(stream, 9);
  TOGGLE(stream, 8);
  TOGGLE(stream, 7);
  TOGGLE(stream, 6);
  TOGGLE(stream, 5);
  TOGGLE(stream, 4);
  TOGGLE(stream, 3);
#endif
  _parser.registerCommand("d", "id", &set_dc);
  _parser.registerCommand("g_d", "i", &get_dc);
  _parser.registerCommand("r", "idi", &set_lux_ref);
  _parser.registerCommand("g_r", "i", &get_lux_ref);
  _parser.registerCommand("g_l", "i", &get_lux_meas);
  _parser.registerCommand("o", "ii", &set_occ_status);
  _parser.registerCommand("g_o", "i", &get_occ_status);
  _parser.registerCommand("a", "ii", &set_anti_windup_status);
  _parser.registerCommand("g_a", "i", &get_anti_windup_status);
  _parser.registerCommand("w", "ii", &set_ff_status);
  _parser.registerCommand("g_w", "i", &get_ff_status);
  _parser.registerCommand("b", "ii", &set_fb_status);
  _parser.registerCommand("g_b", "i", &get_fb_status);
  _parser.registerCommand("g_x", "i", &get_ext_ilu);
  _parser.registerCommand("g_p", "i", &get_curr_pwr);
  _parser.registerCommand("g_t", "i", &get_time);
  _parser.registerCommand("s", "is", &toggle_stream);
  _parser.registerCommand("h", "is", &get_hist);
  _parser.registerCommand("g_e", "i", &get_acc_enrgy_comsumption);
  _parser.registerCommand("g_v", "i", &get_acc_visibility_err);
  _parser.registerCommand("g_f", "i", &get_acc_flicker);
  _parser.registerCommand("c_g", "idd", &set_controller_gains);
}
/**
 * @brief Get the Luminary object by its id (currently only works for ID = 1)
 *
 * @param id Luminary ID
 * @return Luminary*
 */
Luminary *get_lum_by_id(int id) {
  return G_L1;
}
/**
 * @brief Calculate a Hash value for an string
 *
 * @param str String to be hased
 * @return const unsigned long - Hash
 */
const unsigned long hash(const char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  return hash;
}

void Parser::set_dc(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  float dc = args[1].asDouble;
  Luminary *L = get_lum_by_id(id);
  L->led.set_dutty_cicle(dc);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_dc(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "d%d %.3f", id, L->led.dutty_cicle);
}
void Parser::set_lux_ref(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  float ref = args[1].asDouble;
  bool occ = args[2].asInt64;
  Luminary *L = get_lum_by_id(id);
  L->contr.set_ref(ref, occ);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_lux_ref(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "r%d %.6f", id, L->contr.get_ref());
}
void Parser::get_lux_meas(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "r%d %.6f", id, L->ldr.lux);
}
void Parser::set_occ_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  bool occ = args[1].asInt64;
  Luminary *L = get_lum_by_id(id);
  L->contr.set_occ(occ);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_occ_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "o%d %d", id, L->contr.get_occ());
}
void Parser::set_anti_windup_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  bool anti_windup_on = args[1].asInt64;
  Luminary *L = get_lum_by_id(id);
  L->contr.set_anti_windup(anti_windup_on);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_anti_windup_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "a%d %d", id, L->contr.get_anti_windup_status());
}
void Parser::set_ff_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  bool ff_on = args[1].asInt64;
  Luminary *L = get_lum_by_id(id);
  L->contr.set_ff(ff_on);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_ff_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "w%d %d", id, L->contr.get_u_ff_status());
}
void Parser::set_fb_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  bool fb_on = args[1].asInt64;
  Luminary *L = get_lum_by_id(id);
  L->contr.set_fb(fb_on);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_fb_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "w%d %d", id, L->contr.get_u_fb_status());
}
void Parser::get_ext_ilu(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "x%d %.5f", id, L->get_ext_ilu());
}
void Parser::get_curr_pwr(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "p%d %.5f", id, L->get_curr_pwr());
}
void Parser::get_time(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "t%d %.3f", id, float(millis()) / 1000);
}
void Parser::toggle_stream(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  char *var = args[1].asString;
  unsigned long var_h = hash(var);
  switch (var_h) {
    case DC_HASH:
      TOGGLE(stream, 15);
      break;
    case REF_HASH:
      TOGGLE(stream, 14);
      break;
    case L_MEAS_HASH:
      TOGGLE(stream, 13);
      break;
    case L_PRED_HASH:
      TOGGLE(stream, 12);
      break;
    case ERR_HASH:
      TOGGLE(stream, 11);
      break;
    case PROP_HASH:
      TOGGLE(stream, 10);
      break;
    case INTEGRAL_HASH:
      TOGGLE(stream, 9);
      break;
    case U_FF_HASH:
      TOGGLE(stream, 8);
      break;
    case U_FB_HASH:
      TOGGLE(stream, 7);
      break;
    case U:
      TOGGLE(stream, 6);
      break;
    case PWR_HASH:
      TOGGLE(stream, 5);
      break;
    case EXT_ILU_HASH:
      TOGGLE(stream, 4);
      break;
    case FLICKER_HASH:
      TOGGLE(stream, 3);
      break;
    default:
      Serial.println("Var not found");
      break;
  }
}
void Parser::get_hist(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  char *var = args[1].asString;
  unsigned long var_h = hash(var);
  char buff[20];
  Luminary *L = get_lum_by_id(id);
  switch (var_h) {
    case DC_HASH:
      snprintf(buff, 20, "h dc%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < DC.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(DC[i]) * 100 / 65535);
        Serial.print(buff);
      }
      break;
    case REF_HASH:
      snprintf(buff, 20, "h r%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(ref[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case L_MEAS_HASH:
      snprintf(buff, 20, "h lms%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < L_meas.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(L_meas[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case L_PRED_HASH:
      snprintf(buff, 20, "h lp%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < L_pred.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(L_pred[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case ERR_HASH:
      snprintf(buff, 20, "h e%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(ref[i] - L_meas[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case PROP_HASH:
      snprintf(buff, 20, "h p%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(ref[i] - L_meas[i]) * G_L1->contr.get_kp() / 1000);
        Serial.print(buff);
      }
      break;
    case INTEGRAL_HASH:
      snprintf(buff, 20, "h int%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(integral[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case U_FF_HASH:
      snprintf(buff, 20, "h u_ff%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", (float(ref[i]) / 1000 - G_L1->sim.get_L0()) / G_L1->sim.get_G());
        Serial.print(buff);
      }
      break;
    case U_FB_HASH:
      snprintf(buff, 20, "h u_fb%d ", L->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(integral[i]) / 1000 + float(ref[i] - L_meas[i]) * G_L1->contr.get_kp() / 1000);
        Serial.print(buff);
      }
      break;
    case U:
      snprintf(buff, 20, "h u%d ", L->get_id());
      Serial.print(buff);
      float u_fb, u_ff;
      for (uint16_t i = 0; i < ref.size(); i++) {
        u_fb = L->contr.get_u_fb_status() * (float(integral[i]) / 1000 + float(ref[i] - L_meas[i]) * G_L1->contr.get_kp() / 1000);
        u_ff = L->contr.get_u_ff_status() * ((float(ref[i]) / 1000 - G_L1->sim.get_L0()) / G_L1->sim.get_G());
        snprintf(buff, 20, ",%.3f", u_fb + u_ff);
        Serial.print(buff);
      }
      break;
    case PWR_HASH:
      snprintf(buff, 20, "h pw%d ", L->get_id());
      Serial.print(buff);
      for (uint16_t i = 0; i < DC.size(); i++) {
        snprintf(buff, 20, ",%.3f", (float(DC[i]) / 65535) * AVRG_LED_MAX_CURRENT * Vcc);
        Serial.print(buff);
      }
      break;
    case EXT_ILU_HASH:
      snprintf(buff, 20, "h xi%d ", L->get_id());
      Serial.print(buff);
      for (uint16_t i = 0; i < L_meas.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(L_meas[i] - L_pred[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case FLICKER_HASH:
      snprintf(buff, 20, "h fl%d ", L->get_id());
      Serial.print(buff);
      float flicker, l, l1, l2;
      for (uint16_t i = 2; i < L_meas.size(); i++) {
        l = float(L_meas[i]) / 1000;
        l1 = float(L_meas[i - 1]) / 1000;
        l2 = float(L_meas[i - 2]) / 1000;
        flicker = ((abs(l - l1) + abs(l1 - l2)) * CONTROLLER_FREQ / 2) * ((l - l1) * (l1 - l2) < 0);
        snprintf(buff, 20, ",%.4f", flicker);
        Serial.print(buff);
      }
      break;
    default:
      Serial.println("Var not found");
      break;
  }
  Serial.print("\n");
}
void Parser::get_acc_enrgy_comsumption(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "e%d %.3f", id, L->get_acc_energy());
}
void Parser::get_acc_visibility_err(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "v%d %.3f", id, L->get_acc_visibility_err());
}
void Parser::get_acc_flicker(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  Luminary *L = get_lum_by_id(id);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "f%d %.3f", id, L->get_acc_flicker());
}
void Parser::set_controller_gains(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  float kp = args[1].asDouble;
  float ki = args[2].asDouble;
  Luminary *L = get_lum_by_id(id);
  L->contr.set_gains(kp, ki);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "ack");
}

//_____________________________________________________________________________

void init_globals(Luminary *lum) {
  G_L1 = lum;
}
/**
 * @brief Updates the circular buffer with state variables
 *
 */
void update_hist() {
  DC.push((G_L1->led.dutty_cicle / 100) * 65535);
  ref.push(G_L1->contr.get_ref() * 1000);
  L_meas.push(G_L1->ldr.lux * 1000);
  L_pred.push(G_L1->sim.get_current_l_prediction() * 1000);
  integral.push(G_L1->contr.get_integral() * 1000);
}

void print_stream(Luminary *Lum) {
  char buff[300];
  snprintf_P(buff, 300, "s ");
  if (READ(stream, 15)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "DC%d %.2f,", G_L1->get_id(), G_L1->led.dutty_cicle);
  }
  if (READ(stream, 14)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "Ref%d %.3f,", G_L1->get_id(), G_L1->contr.get_ref());
  }
  if (READ(stream, 13)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "L_meas%d %.5f,", G_L1->get_id(), G_L1->ldr.lux);
  }
  if (READ(stream, 12)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "L_pred%d %.5f,", G_L1->get_id(), G_L1->sim.get_current_l_prediction());
  }
  if (READ(stream, 11)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "err%d %.7f,", G_L1->get_id(), G_L1->contr.get_ref() - G_L1->ldr.lux);
  }
  if (READ(stream, 10)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "prop%d %.5f,", G_L1->get_id(), (G_L1->contr.get_ref() - G_L1->ldr.lux) * G_L1->contr.get_kp());
  }
  if (READ(stream, 9)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "int%d %.5f,", G_L1->get_id(), G_L1->contr.get_integral());
  }
  if (READ(stream, 8)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "u_ff%d %.6f,", G_L1->get_id(), (G_L1->contr.get_ref() - G_L1->sim.get_L0()) / G_L1->sim.get_G());
  }
  if (READ(stream, 7)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "u_fb%d %.6f,", G_L1->get_id(), (G_L1->contr.get_ref() - G_L1->ldr.lux) * G_L1->contr.get_kp() + G_L1->contr.get_integral());
  }
  if (READ(stream, 6)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "u%d %.7f,", G_L1->get_id(), ((G_L1->contr.get_ref() - G_L1->sim.get_L0()) / G_L1->sim.get_G()) + ((G_L1->contr.get_ref() - G_L1->ldr.lux) * G_L1->contr.get_kp() + G_L1->contr.get_integral()));
  }
  if (READ(stream, 5)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "pwr%d %.3f,", G_L1->get_id(), G_L1->get_curr_pwr());
  }
  if (READ(stream, 4)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "ex_ilu%d %.5f,", G_L1->get_id(), G_L1->get_ext_ilu());
  }
  if (READ(stream, 3)) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "flck%d %.4f,", G_L1->get_id(), G_L1->get_curr_flicker());
  }
  if (strlen(buff) > 4) {
    snprintf(buff + strlen(buff), 300 - strlen(buff), "t %d", millis());
    Serial.println(buff);
  }
}
/**
 * @brief Controller and simulator routine implemented on the main timer callback
 *
 * @param t Timer structure
 * @return true
 * @return false
 */
bool main_timer_callback(struct repeating_timer *t) {
  G_L1->sim.sim_callback(G_L1->contr.get_u_ff());
  G_L1->ldr.median_measure(5);
  G_L1->contr.compute_fdbk_ctrl_action();
  G_L1->contr.compute_ffwrd_ctrl_action();
  G_L1->contr.update_led_DC();
  G_L1->calc_ext_ilu();
  G_L1->calc_pwr();
  int size_buff = L_meas.size();
  G_L1->calc_metrics(L_meas[size_buff - 1], L_meas[size_buff - 2], L_meas[size_buff - 3]);
  update_hist();
  print_stream(G_L1);
  return true;
}