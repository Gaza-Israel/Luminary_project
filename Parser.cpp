#include "Parser.h"

#include <CircularBuffer.h>

#include "I2C_message_protocol.h"
#include "Luminaire.h"
#include "Streaming.h"

Luminary *G_L1;

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
  if (id!=G_L1->get_id()){
    bool msg_sent = I2C_message_protocol::set_dc(id,dc);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  G_L1->led.set_dutty_cicle(dc);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}

void Parser::get_dc(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_dc(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "d%d %.3f", id, G_L1->led.dutty_cicle);
}
void Parser::set_lux_ref(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  float ref = args[1].asDouble;
  bool occ = args[2].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::set_lux_ref(id, ref, occ);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  G_L1->contr.set_ref(ref, occ);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_lux_ref(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_lux_ref(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "r%d %.6f", id, G_L1->contr.get_ref());
}
void Parser::get_lux_meas(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_lux_meas(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "l%d %.6f", id, G_L1->ldr.lux);
}
void Parser::set_occ_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  bool occ = args[1].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::set_occ_status(id,occ);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  G_L1->contr.set_occ(occ);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_occ_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_occ_status(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "o%d %d", id, G_L1->contr.get_occ());
}
void Parser::set_anti_windup_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  bool anti_windup_on = args[1].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::set_anti_windup_status(id,anti_windup_on);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  G_L1->contr.set_anti_windup(anti_windup_on);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_anti_windup_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_anti_windup_status(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "a%d %d", id, G_L1->contr.get_anti_windup_status());
}
void Parser::set_ff_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  bool ff_on = args[1].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::set_anti_windup_status(id,ff_on);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  G_L1->contr.set_ff(ff_on);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_ff_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_ff_status(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "w%d %d", id, G_L1->contr.get_u_ff_status());
}
void Parser::set_fb_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  bool fb_on = args[1].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::set_fb_status(id,fb_on);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  G_L1->contr.set_fb(fb_on);
  strlcpy(response, "ack", MyCommandParser::MAX_RESPONSE_SIZE);
}
void Parser::get_fb_status(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_fb_status(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "w%d %d", id, G_L1->contr.get_u_fb_status());
}
void Parser::get_ext_ilu(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_ext_ilu(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "x%d %.5f", id, G_L1->get_ext_ilu());
}
void Parser::get_curr_pwr(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_curr_pwr(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "p%d %.5f", id, G_L1->get_curr_pwr());
}
void Parser::get_time(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_time(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "t%d %.3f", id, float(millis()) / 1000);
}
void Parser::toggle_stream(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  char *var = args[1].asString;
  unsigned long var_h = hash(var);
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::toggle_stream(id,var_h);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  G_L1->toggle_serial_stream(var_h);
}

void Parser::get_hist(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  char *var = args[1].asString;
  unsigned long var_h = hash(var);
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_hist(id,var_h);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  char buff[20];
  switch (var_h) {
    case DC_HASH:
      snprintf(buff, 20, "h dc%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->DC.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->DC[i]) * 100 / 65535);
        Serial.print(buff);
      }
      break;
    case REF_HASH:
      snprintf(buff, 20, "h r%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->ref[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case L_MEAS_HASH:
      snprintf(buff, 20, "h lms%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->L_meas.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->L_meas[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case L_PRED_HASH:
      snprintf(buff, 20, "h lp%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->L_pred.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->L_pred[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case ERR_HASH:
      snprintf(buff, 20, "h e%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->ref[i] - G_L1->L_meas[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case PROP_HASH:
      snprintf(buff, 20, "h p%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->ref[i] - G_L1->L_meas[i]) * G_L1->contr.get_kp() / 1000);
        Serial.print(buff);
      }
      break;
    case INTEGRAL_HASH:
      snprintf(buff, 20, "h int%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->integral[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case U_FF_HASH:
      snprintf(buff, 20, "h u_ff%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", (float(G_L1->ref[i]) / 1000 - G_L1->sim.get_L0()) / G_L1->sim.get_G());
        Serial.print(buff);
      }
      break;
    case U_FB_HASH:
      snprintf(buff, 20, "h u_fb%d ", G_L1->get_id());
      Serial.print(buff);

      for (uint16_t i = 0; i < G_L1->ref.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->integral[i]) / 1000 + float(G_L1->ref[i] - G_L1->L_meas[i]) * G_L1->contr.get_kp() / 1000);
        Serial.print(buff);
      }
      break;
    case U_HASH:
      snprintf(buff, 20, "h u%d ", G_L1->get_id());
      Serial.print(buff);
      float u_fb, u_ff;
      for (uint16_t i = 0; i < G_L1->ref.size(); i++) {
        u_fb = G_L1->contr.get_u_fb_status() * (float(G_L1->integral[i]) / 1000 + float(G_L1->ref[i] - G_L1->L_meas[i]) * G_L1->contr.get_kp() / 1000);
        u_ff = G_L1->contr.get_u_ff_status() * ((float(G_L1->ref[i]) / 1000 - G_L1->sim.get_L0()) / G_L1->sim.get_G());
        snprintf(buff, 20, ",%.3f", u_fb + u_ff);
        Serial.print(buff);
      }
      break;
    case PWR_HASH:
      snprintf(buff, 20, "h pw%d ", G_L1->get_id());
      Serial.print(buff);
      for (uint16_t i = 0; i < G_L1->DC.size(); i++) {
        snprintf(buff, 20, ",%.3f", (float(G_L1->DC[i]) / 65535) * AVRG_LED_MAX_CURRENT * Vcc);
        Serial.print(buff);
      }
      break;
    case EXT_ILU_HASH:
      snprintf(buff, 20, "h xi%d ", G_L1->get_id());
      Serial.print(buff);
      for (uint16_t i = 0; i < G_L1->L_meas.size(); i++) {
        snprintf(buff, 20, ",%.3f", float(G_L1->L_meas[i] - G_L1->L_pred[i]) / 1000);
        Serial.print(buff);
      }
      break;
    case FLICKER_HASH:
      snprintf(buff, 20, "h fl%d ", G_L1->get_id());
      Serial.print(buff);
      float flicker, l, l1, l2;
      for (uint16_t i = 2; i < G_L1->L_meas.size(); i++) {
        l = float(G_L1->L_meas[i]) / 1000;
        l1 = float(G_L1->L_meas[i - 1]) / 1000;
        l2 = float(G_L1->L_meas[i - 2]) / 1000;
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
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_acc_enrgy_comsumption(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "e%d %.3f", id, G_L1->get_acc_energy());
}
void Parser::get_acc_visibility_err(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_acc_visibility_err(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "v%d %.3f", id, G_L1->get_acc_visibility_err());
}
void Parser::get_acc_flicker(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::get_acc_flicker(id);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "f%d %.3f", id, G_L1->get_acc_flicker());
}
void Parser::set_controller_gains(MyCommandParser::Argument *args, char *response) {
  int id = args[0].asInt64;
  float kp = args[1].asDouble;
  float ki = args[2].asDouble;
  if (id != G_L1->get_id()) {
    bool msg_sent = I2C_message_protocol::set_controller_gains(id,kp,ki);
    strlcpy(response, msg_sent ? "sent" : "error", MyCommandParser::MAX_RESPONSE_SIZE);
    return;
  }
  G_L1->contr.set_gains(kp, ki);
  snprintf(response, MyCommandParser::MAX_RESPONSE_SIZE, "ack");
}

//_____________________________________________________________________________

void pass_lum_to_parser(Luminary *lum) {
  G_L1 = lum;
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
  int size_buff = G_L1->L_meas.size();
  G_L1->calc_metrics(G_L1->L_meas[size_buff - 1], G_L1->L_meas[size_buff - 2], G_L1->L_meas[size_buff - 3]);
  G_L1->update_hist();
  G_L1->print_stream();
  G_L1->send_i2c_stream();
  return true;
}