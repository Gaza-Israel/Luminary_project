#include "I2C_message_protocol.h"

#include "Luminaire.h"
#include "median_filter.h"
#include "minimizer.h"
#include "System_config.h"
template <class To, class From>
std::enable_if_t<
    sizeof(To) == sizeof(From) &&
        std::is_trivially_copyable_v<From> &&
        std::is_trivially_copyable_v<To>,
    To>
// constexpr support needs compiler magic
bit_cast(const From& src) noexcept {
  static_assert(std::is_trivially_constructible_v<To>,
                "This implementation additionally requires destination type to be trivially constructible");

  To dst;
  memcpy(&dst, &src, sizeof(To));
  return dst;
}
namespace I2C_message_protocol {
float buff_recv_i2c_stream[N_STREAM_VARIABLES * N_LUMINARIES] = {0};
bool recv_new_i2c_stream[N_STREAM_VARIABLES * N_LUMINARIES] = {0};
uint8_t nodes_addr[N_STREAM_VARIABLES * N_LUMINARIES] = {0};
uint8_t n_addr_saved = 0;
Luminary* L;

/**
 * @brief Checks if an address is already stored on the nodes_addr array.
 * If the address is already there, return the index of the address, else return -1.
 *
 * @param addr
 * @return int index of the address or -1 if not stored yet
 */
int addr_is_saved(uint8_t addr) {
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (I2C_message_protocol::nodes_addr[i] == addr) {
      return i;
    }
  }
  return -1;
}
void save_addr(uint8_t addr) {
  if (addr_is_saved(addr) == -1) {
    I2C_message_protocol::nodes_addr[I2C_message_protocol::n_addr_saved] = addr;
    I2C_message_protocol::n_addr_saved++;
  }
}

void sort_addresses() {
  quickSort_uint8(I2C_message_protocol::nodes_addr, 0, N_LUMINARIES - 1);
}
/*
Command forwarding
=====================================
*/

/*
--------------------------------------------------------
                    Set commands
--------------------------------------------------------
*/

void broadcast_node(int id) {
  I2C::i2c_message msg;
  msg.msg_id = BROADCAST_MSG_ID;
  msg.data = id;
  I2C_message_protocol::save_addr(I2C::get_I2C1_address());
  I2C::send_message(msg, 0x00);
}
void g_calib_start() {
  I2C::i2c_message msg;
  msg.msg_id = G_CALIB_START;
  I2C::send_message(msg, 0x00);
}
void self_calib_end() {
  I2C::i2c_message msg;
  msg.msg_id = SELF_CALIB_END;
  I2C::send_message(msg, 0x00);
}
bool set_dc(uint8_t addr, float value) {
  I2C::i2c_message msg;
  msg.msg_id = set_dc_MSG_ID;
  msg.data = bit_cast<uint32_t>(value);
  return !I2C::send_message(msg, addr);
}
bool set_lux_ref(uint8_t addr, float value, bool occ) {
  I2C::i2c_message msg;
  msg.msg_id = occ ? set_occ_lux_ref_MSG_ID : set_unocc_lux_ref_MSG_ID;
  msg.data = bit_cast<uint32_t>(value);
  return !I2C::send_message(msg, addr);
}
bool set_occ_status(uint8_t addr, bool value) {
  I2C::i2c_message msg;
  msg.msg_id = set_occ_status_MSG_ID;
  msg.data = value;
  return !I2C::send_message(msg, addr);
}
bool set_anti_windup_status(uint8_t addr, bool value) {
  I2C::i2c_message msg;
  msg.msg_id = set_anti_windup_status_MSG_ID;
  msg.data = value;
  return !I2C::send_message(msg, addr);
}
bool set_ff_status(uint8_t addr, bool value) {
  I2C::i2c_message msg;
  msg.msg_id = set_ff_status_MSG_ID;
  msg.data = value;
  return !I2C::send_message(msg, addr);
}
bool set_fb_status(uint8_t addr, bool value) {
  I2C::i2c_message msg;
  msg.msg_id = set_fb_status_MSG_ID;
  msg.data = value;
  return !I2C::send_message(msg, addr);
}
bool toggle_stream(uint8_t addr, unsigned long value) {
  I2C::i2c_message msg;
  msg.msg_id = toggle_stream_MSG_ID;
  msg.data = value;
  return !I2C::send_message(msg, addr);
}
bool set_controller_gains(uint8_t addr, float kp, float ki) {
  I2C::i2c_message msg;
  msg.msg_id = set_controller_gains_MSG_ID;
  msg.data = uint16_t(kp * 10) << 16 | uint16_t(ki * 10);
  return !I2C::send_message(msg, addr);
}

/*
--------------------------------------------------------
                    Get commands
--------------------------------------------------------
*/

bool get_dc(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_dc_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_lux_ref(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_lux_ref_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_lux_meas(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_lux_meas_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_occ_status(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_occ_status_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_anti_windup_status(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_anti_windup_status_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_ff_status(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_ff_status_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_fb_status(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_fb_status_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_ext_ilu(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_ext_ilu_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_curr_pwr(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_curr_pwr_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_time(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_time_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_hist(uint8_t addr, unsigned long value) {
  I2C::i2c_message msg;
  msg.msg_id = get_hist_MSG_ID;
  msg.data = bit_cast<uint32_t>(value);
  return !I2C::send_message(msg, addr);
}
bool get_acc_enrgy_comsumption(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_acc_enrgy_comsumption_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_acc_visibility_err(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_acc_visibility_err_MSG_ID;
  return !I2C::send_message(msg, addr);
}
bool get_acc_flicker(uint8_t addr) {
  I2C::i2c_message msg;
  msg.msg_id = get_acc_flicker_MSG_ID;
  return !I2C::send_message(msg, addr);
}

bool send_stream(uint8_t addr, uint8_t var, float data) {
  I2C::i2c_message msg;
  msg.msg_id = send_stream_DC + var;

  char buff[100];
  snprintf(buff, 100, "Sendind stream with message id %d", msg.msg_id);
  Serial.println(buff);
  msg.data = bit_cast<uint32_t>(data);
  return !I2C::send_message(msg, addr);
}

bool broadcast_dc(float value) {
  I2C::i2c_message msg;
  msg.msg_id = send_dc_MSG_ID;
  msg.data = bit_cast<uint32_t>(value);
  return I2C::send_message(msg, 0x00);
}

void broadcast_local_consensus_dc(uint8_t addr) {
  I2C::i2c_message msg;

  byte tx_buff[15];  // size 11 (struct pad to 16, but only 11 are necessary)

  msg.node = I2C::get_I2C1_address();
  msg.msg_id = BROADCAST_CONSENSUS_DC;
  msg.ts = bit_cast<uint32_t>(L->d[0]);
  msg.data = bit_cast<uint32_t>(L->d[1]);
  // msg.data2 = bit_cast<uint32_t>(L->d[2]);

  tx_buff[0] = msg.node;
  tx_buff[1] = msg.msg_id;
  tx_buff[2] = msg.ts >> 0;
  tx_buff[3] = msg.ts >> 8;
  tx_buff[4] = msg.ts >> 16;
  tx_buff[5] = msg.ts >> 24;
  tx_buff[6] = msg.data >> 0;
  tx_buff[7] = msg.data >> 8;
  tx_buff[8] = msg.data >> 16;
  tx_buff[9] = msg.data >> 24;
  tx_buff[10] = msg.data2 >> 0;
  tx_buff[11] = msg.data2 >> 8;
  tx_buff[12] = msg.data2 >> 16;
  tx_buff[13] = msg.data2 >> 24;
  tx_buff[14] = I2C::calculate_pec(tx_buff, 15 - 1);

  Wire.beginTransmission(addr);
  Wire.write(tx_buff, sizeof(tx_buff));
  Wire.endTransmission();
}
/*
--------------------------------------------------------
                 Receive commands
--------------------------------------------------------
*/

void send_ack(uint8_t addr, uint32_t ID) {
  I2C::i2c_message msg;
  msg.msg_id = ACK_MSG_ID;
  msg.data = ID;
  I2C::send_message(msg, addr);
}

void send_error(uint8_t addr, uint32_t ERROR_REASON) {
  I2C::i2c_message msg;
  msg.msg_id = ERROR_MSG_ID;
  msg.data = ERROR_REASON;
  I2C::send_message(msg, addr);
}

void parse_message(I2C::i2c_message msg) {
  I2C::i2c_message repply_msg;
  switch (msg.msg_id) {
    case BROADCAST_MSG_ID: {
      save_addr(msg.node);
      break;
    }
    case ACK_MSG_ID: {
      char buff[30];
      snprintf(buff, 30, "Ack by %d of command %lu", msg.node, msg.data);
      Serial.println(buff);
      break;
    }
    case BROADCAST_CONSENSUS_DC: {
      float dc0 = bit_cast<float>(msg.ts);
      float dc1 = bit_cast<float>(msg.data);
      float dc2 = bit_cast<float>(msg.data2);
      Serial.println(dc0);
      Serial.println(dc1);
      Serial.println(dc2);
      Serial.println(addr_is_saved(msg.node));
      if (!L->d_received[addr_is_saved(msg.node)]) {
        L->d_[0] = L->d_[0] + dc0 / N_LUMINARIES;
        L->d_[1] = L->d_[1] + dc1 / N_LUMINARIES;
        // L->d_[2] += dc2 / N_LUMINARIES;
        L->d_received[addr_is_saved(msg.node)] = true;
      }
    }
    default: {
      if (GET_MESSAGES(msg.msg_id)) {
        parse_get(msg);
        break;
      }
      if (SET_MESSAGES(msg.msg_id)) {
        parse_set(msg);
        break;
      }
      if (SEND_MESSAGES(msg.msg_id)) {
        parse_send(msg);
        break;
      }
      send_error(msg.node, COMMAND_INVALID_ERROR);
    }
  }
}

void parse_set(I2C::i2c_message msg) {
  I2C::i2c_message repply_msg;
  switch (msg.msg_id) {
    case set_dc_MSG_ID: {
      L->led.set_dutty_cicle(bit_cast<float>(msg.data));
      send_ack(msg.node, msg.msg_id);
      break;
    }
    case set_occ_lux_ref_MSG_ID: {
      L->contr.set_ref(bit_cast<float>(msg.data), true);
      send_ack(msg.node, msg.msg_id);
      break;
    }
    case set_unocc_lux_ref_MSG_ID: {
      L->contr.set_ref(bit_cast<float>(msg.data), false);
      send_ack(msg.node, msg.msg_id);
      break;
    }
    case set_occ_status_MSG_ID: {
      L->contr.set_occ(bool(msg.data));
      send_ack(msg.node, msg.msg_id);
      break;
    }
    case set_anti_windup_status_MSG_ID: {
      L->contr.set_anti_windup(bool(msg.data));
      send_ack(msg.node, msg.msg_id);
      break;
    }
    case set_ff_status_MSG_ID: {
      L->contr.set_ff(bool(msg.data));
      send_ack(msg.node, msg.msg_id);
      break;
    }
    case set_fb_status_MSG_ID: {
      L->contr.set_fb(bool(msg.data));
      send_ack(msg.node, msg.msg_id);
      break;
    }
    case toggle_stream_MSG_ID: {
      L->toggle_i2c_stream(msg.data);
      send_ack(msg.node, msg.msg_id);
      break;
    }
    case set_controller_gains_MSG_ID: {
      float kp = float((msg.data & 0xFFFF0000) >> 16) / 10;
      float ki = float(msg.data & 0x0000FFFF) / 10;
      L->contr.set_gains(kp, ki);
      send_ack(msg.node, msg.msg_id);
      break;
    }
  }
}
void parse_get(I2C::i2c_message msg) {
  I2C::i2c_message repply_msg;
  switch (msg.msg_id) {
    case get_dc_MSG_ID: {
      repply_msg.msg_id = send_dc_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(L->led.dutty_cicle);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_lux_ref_MSG_ID: {
      repply_msg.msg_id = send_lux_ref_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(L->contr.get_ref());
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_lux_meas_MSG_ID: {
      repply_msg.msg_id = send_lux_meas_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(L->ldr.lux);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_occ_status_MSG_ID: {
      repply_msg.msg_id = send_occ_status_MSG_ID;
      repply_msg.data = L->contr.get_occ();
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_anti_windup_status_MSG_ID: {
      repply_msg.msg_id = send_anti_windup_status_MSG_ID;
      repply_msg.data = L->contr.get_anti_windup_status();
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_ff_status_MSG_ID: {
      repply_msg.msg_id = send_ff_status_MSG_ID;
      repply_msg.data = L->contr.get_u_ff_status();
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_fb_status_MSG_ID: {
      repply_msg.msg_id = send_fb_status_MSG_ID;
      repply_msg.data = L->contr.get_u_fb_status();
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_ext_ilu_MSG_ID: {
      repply_msg.msg_id = send_ext_ilu_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(L->get_ext_ilu());
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_curr_pwr_MSG_ID: {
      repply_msg.msg_id = send_curr_pwr_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(L->get_curr_pwr());
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_time_MSG_ID: {
      repply_msg.msg_id = send_time_MSG_ID;
      repply_msg.data = millis();
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_hist_MSG_ID: {
      parse_get_hist(msg);
      break;
    }
    case get_acc_enrgy_comsumption_MSG_ID: {
      repply_msg.msg_id = send_acc_enrgy_comsumption_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(L->get_acc_energy());
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_acc_visibility_err_MSG_ID: {
      repply_msg.msg_id = send_acc_visibility_err_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(L->get_acc_visibility_err());
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case get_acc_flicker_MSG_ID: {
      repply_msg.msg_id = send_acc_flicker_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(L->get_acc_flicker());
      I2C::send_message(repply_msg, msg.node);
      break;
    }
  }
}
void parse_get_hist(I2C::i2c_message msg) {
  I2C::i2c_message repply_msg;
  char buff[20];
  switch (msg.data) {
    case DC_HASH: {
      repply_msg.msg_id = start_DC_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->DC.first()) * 100 / 65535);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->DC.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->DC[i]) * 100 / 65535);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->DC.last()) * 100 / 65535);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case REF_HASH: {
      repply_msg.msg_id = start_ref_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->ref.first()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->ref.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->ref[i]) / 1000);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->ref.last()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case L_MEAS_HASH: {
      repply_msg.msg_id = start_L_meas_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->L_meas.first()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->L_meas.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->L_meas[i]) / 1000);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->L_meas.last()) / 1000);
      I2C::send_message(repply_msg, msg.node);

      break;
    }
    case L_PRED_HASH: {
      repply_msg.msg_id = start_L_pred_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->L_pred.first()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->L_pred.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->L_pred[i]) / 1000);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->L_pred.last()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case ERR_HASH: {
      repply_msg.msg_id = start_err_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->ref.first() - L->L_meas.first()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->ref.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->ref[i] - L->L_meas[i]) / 1000);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->ref.last() - L->L_meas.last()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case PROP_HASH: {
      repply_msg.msg_id = start_prop_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->ref.first() - L->L_meas.first()) * L->contr.get_kp() / 1000);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->ref.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->ref[i] - L->L_meas[i]) * L->contr.get_kp() / 1000);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->ref.last() - L->L_meas.last()) * L->contr.get_kp() / 1000);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case INTEGRAL_HASH: {
      repply_msg.msg_id = start_int_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->integral.first()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->ref.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->integral[i]) / 1000);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->integral.last()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case U_FF_HASH: {
      repply_msg.msg_id = start_u_ff_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>((float(L->ref.first()) / 1000 - L->sim.get_L0()) / L->sim.get_G());
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->ref.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>((float(L->ref[i]) / 1000 - L->sim.get_L0()) / L->sim.get_G());
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>((float(L->ref.last()) / 1000 - L->sim.get_L0()) / L->sim.get_G());
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case U_FB_HASH: {
      repply_msg.msg_id = start_u_fb_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->integral.first()) / 1000 + float(L->ref.first() - L->L_meas.first()) * L->contr.get_kp() / 1000);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->ref.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->integral[i]) / 1000 + float(L->ref[i] - L->L_meas[i]) * L->contr.get_kp() / 1000);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->integral.last()) / 1000 + float(L->ref.last() - L->L_meas.last()) * L->contr.get_kp() / 1000);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case U_HASH: {
      repply_msg.msg_id = start_u_HIST_TRANSMISSION_MSG_ID;
      float u_fb, u_ff;
      u_fb = L->contr.get_u_fb_status() * (float(L->integral.first()) / 1000 + float(L->ref.first() - L->L_meas.first()) * L->contr.get_kp() / 1000);
      u_ff = L->contr.get_u_ff_status() * ((float(L->ref.first()) / 1000 - L->sim.get_L0()) / L->sim.get_G());
      repply_msg.data = bit_cast<uint32_t>(u_fb + u_ff);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->ref.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        u_fb = L->contr.get_u_fb_status() * (float(L->integral[i]) / 1000 + float(L->ref[i] - L->L_meas[i]) * L->contr.get_kp() / 1000);
        u_ff = L->contr.get_u_ff_status() * ((float(L->ref[i]) / 1000 - L->sim.get_L0()) / L->sim.get_G());
        repply_msg.data = bit_cast<uint32_t>(u_fb + u_ff);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      u_fb = L->contr.get_u_fb_status() * (float(L->integral.last()) / 1000 + float(L->ref.last() - L->L_meas.last()) * L->contr.get_kp() / 1000);
      u_ff = L->contr.get_u_ff_status() * ((float(L->ref.last()) / 1000 - L->sim.get_L0()) / L->sim.get_G());
      repply_msg.data = bit_cast<uint32_t>(u_fb + u_ff);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case PWR_HASH: {
      repply_msg.msg_id = start_pwr_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float((float(L->DC.first()) / 65535) * AVRG_LED_MAX_CURRENT * Vcc));
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->DC.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float((float(L->DC[i]) / 65535) * AVRG_LED_MAX_CURRENT * Vcc));
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float((float(L->DC.last()) / 65535) * AVRG_LED_MAX_CURRENT * Vcc));
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case EXT_ILU_HASH: {
      repply_msg.msg_id = start_ext_ilu_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->L_meas.first() - L->L_pred.first()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 1; i < -1 + L->L_meas.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        repply_msg.data = bit_cast<uint32_t>(float(L->L_meas[i] - L->L_pred[i]) / 1000);
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      repply_msg.data = bit_cast<uint32_t>(float(L->L_meas.last() - L->L_pred.last()) / 1000);
      I2C::send_message(repply_msg, msg.node);
      break;
    }
    case FLICKER_HASH: {
      repply_msg.msg_id = start_flicker_HIST_TRANSMISSION_MSG_ID;
      float flicker, l, l1, l2;
      l = float(L->L_meas[2]) / 1000;
      l1 = float(L->L_meas[1]) / 1000;
      l2 = float(L->L_meas[0]) / 1000;
      flicker = bit_cast<uint32_t>(((abs(l - l1) + abs(l1 - l2)) * CONTROLLER_FREQ / 2) * ((l - l1) * (l1 - l2) < 0));
      repply_msg.data = flicker;
      I2C::send_message(repply_msg, msg.node);
      for (uint16_t i = 3; i < L->L_meas.size(); i++) {
        repply_msg.msg_id = send_hist_MSG_ID;
        l = float(L->L_meas[i]) / 1000;
        l1 = float(L->L_meas[i - 1]) / 1000;
        l2 = float(L->L_meas[i - 2]) / 1000;
        flicker = bit_cast<uint32_t>(((abs(l - l1) + abs(l1 - l2)) * CONTROLLER_FREQ / 2) * ((l - l1) * (l1 - l2) < 0));
        repply_msg.data = flicker;
        I2C::send_message(repply_msg, msg.node);
      }
      repply_msg.msg_id = end_HIST_TRANSMISSION_MSG_ID;
      l = float(L->L_meas.last()) / 1000;
      l1 = float(L->L_meas[L->L_meas.size() - 2]) / 1000;
      l2 = float(L->L_meas[L->L_meas.size() - 3]) / 1000;
      flicker = bit_cast<uint32_t>(((abs(l - l1) + abs(l1 - l2)) * CONTROLLER_FREQ / 2) * ((l - l1) * (l1 - l2) < 0));
      repply_msg.data = flicker;
      I2C::send_message(repply_msg, msg.node);

      break;
    }
    default: {
      send_error(msg.node, VARIABLE_NOT_FOUND_ERROR);
      break;
    }
  }
}
void parse_send(I2C::i2c_message msg) {
  switch (msg.msg_id) {
    case send_dc_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "d%d %.3f", msg.node, bit_cast<float>(msg.data));
      Serial.println(buff);
      break;
    }
    case send_lux_ref_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "r%d %.6f", msg.node, bit_cast<float>(msg.data));
      Serial.println(buff);
      break;
    }
    case send_lux_meas_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "l%d %.6f", msg.node, bit_cast<float>(msg.data));
      Serial.println(buff);
      break;
    }
    case send_occ_status_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "o%d %d", msg.node, bool(msg.data));
      Serial.println(buff);
      break;
    }
    case send_anti_windup_status_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "a%d %d", msg.node, bool(msg.data));
      Serial.println(buff);
      break;
    }
    case send_ff_status_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "w%d %d", msg.node, bool(msg.data));
      Serial.println(buff);
      break;
    }
    case send_fb_status_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "b%d %d", msg.node, bool(msg.data));
      Serial.println(buff);
      break;
    }
    case send_ext_ilu_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "x%d %.5f", msg.node, bit_cast<float>(msg.data));
      Serial.println(buff);
      break;
    }
    case send_curr_pwr_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "p%d %.5f", msg.node, bit_cast<float>(msg.data));
      Serial.println(buff);
      break;
    }
    case send_time_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "t%d %.3f", msg.node, bit_cast<float>(msg.data) / 1000);
      Serial.println(buff);
      break;
    }
    case send_acc_enrgy_comsumption_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "e%d %.3f", msg.node, bit_cast<float>(msg.data));
      Serial.println(buff);
      break;
    }
    case send_acc_visibility_err_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "v%d %.3f", msg.node, bit_cast<float>(msg.data));
      Serial.println(buff);
      break;
    }
    case send_acc_flicker_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "f%d %.3f", msg.node, bit_cast<float>(msg.data));
      Serial.println(buff);
      break;
    }
    default: {
      if (TRANSMISSION_MESSAGES(msg.msg_id)) {
        parse_send_transmission(msg);
        break;
      }
      if (STREAM_MESSAGES(msg.msg_id)) {
        parse_send_stream(msg);
        break;
      }
      send_error(msg.node, COMMAND_INVALID_ERROR);
    }
  }
}
void parse_send_transmission(I2C::i2c_message msg) {
  switch (msg.msg_id) {
    case start_DC_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h dc%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_ref_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h r%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_L_meas_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h lms%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_L_pred_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h lp%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_err_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h e%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_prop_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h p%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_int_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h int%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_u_ff_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h uff%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_u_fb_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h ufb%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_u_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h u%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_pwr_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h pw%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_ext_ilu_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h xi%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case start_flicker_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, "h fl%d, %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case send_hist_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, ", %.3f ", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
    case end_HIST_TRANSMISSION_MSG_ID: {
      char buff[20];
      snprintf(buff, 20, ", %.3f\n", msg.node, bit_cast<float>(msg.data));
      Serial.print(buff);
      break;
    }
  }
}
void parse_send_stream(I2C::i2c_message msg) {
  switch (msg.msg_id) {
    case send_stream_DC: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 0;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_ref: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 1;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_L_meas: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 2;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_L_pred: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 3;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_err: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 4;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_prop: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 5;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_int: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 6;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_u_ff: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 7;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_u_fb: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 8;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_u: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 9;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_pwr: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 10;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_ext_ilu: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 11;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
    case send_stream_flicker: {
      uint8_t idx = addr_is_saved(msg.node);
      if (idx != -1) {
        idx = idx * N_STREAM_VARIABLES + 12;
        buff_recv_i2c_stream[idx] = bit_cast<float>(msg.data);
        recv_new_i2c_stream[idx] = true;
      }
      break;
    }
  }
}
}  // namespace I2C_message_protocol
