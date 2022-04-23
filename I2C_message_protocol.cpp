#include "I2C_message_protocol.h"

namespace I2C_message_protocol {
void broadcast_node(int id) {
  I2C::i2c_message msg;
  msg.msg_id = 1;
  msg.data = id << 8 | I2C::get_I2C1_address();
  I2C::send_message(msg, 0x00);
}

bool set_dc(uint8_t addr, float value) {
}
bool get_dc(uint8_t addr) {
}
bool set_lux_ref(uint8_t addr, float value, bool occ) {
}
bool get_lux_ref(uint8_t addr) {
}
bool get_lux_meas(uint8_t addr) {
}
bool set_occ_status(uint8_t addr, bool value) {
}
bool get_occ_status(uint8_t addr) {
}
bool set_anti_windup_status(uint8_t addr, bool value) {
}
bool get_anti_windup_status(uint8_t addr) {
}
bool set_ff_status(uint8_t addr, bool value) {
}
bool get_ff_status(uint8_t addr) {
}
bool set_fb_status(uint8_t addr, bool value) {
}
bool get_fb_status(uint8_t addr) {
}
bool get_ext_ilu(uint8_t addr) {
}
bool get_curr_pwr(uint8_t addr) {
}
bool get_time(uint8_t addr) {
}
bool toggle_stream(uint8_t addr, unsigned long value) {
}
bool get_hist(uint8_t addr, unsigned long value) {
}
bool get_acc_enrgy_comsumption(uint8_t addr) {
}
bool get_acc_visibility_err(uint8_t addr) {
}
bool get_acc_flicker(uint8_t addr) {
}
bool set_controller_gains(uint8_t addr, float kp, float ki) {
}
}  // namespace I2C_message_protocol