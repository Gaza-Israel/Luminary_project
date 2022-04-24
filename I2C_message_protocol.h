#ifndef I2C_message_protocol_h
#define I2C_message_protocol_h
#include "I2C.h"
#include "Luminaire.h"

namespace I2C_message_protocol {

void broadcast_node(int id);

bool set_dc(uint8_t addr, float value);
bool set_lux_ref(uint8_t addr, float value, bool occ);
bool set_occ_status(uint8_t addr, bool value);
bool set_anti_windup_status(uint8_t addr, bool value);
bool set_ff_status(uint8_t addr, bool value);
bool set_fb_status(uint8_t addr, bool value);
bool toggle_stream(uint8_t addr, unsigned long value);
bool set_controller_gains(uint8_t addr, float kp, float ki);

bool get_dc(uint8_t addr);
bool get_lux_ref(uint8_t addr);
bool get_lux_meas(uint8_t addr);
bool get_occ_status(uint8_t addr);
bool get_anti_windup_status(uint8_t addr);
bool get_ff_status(uint8_t addr);
bool get_fb_status(uint8_t addr);
bool get_ext_ilu(uint8_t addr);
bool get_curr_pwr(uint8_t addr);
bool get_time(uint8_t addr);
bool get_hist(uint8_t addr, unsigned long value);
bool get_acc_enrgy_comsumption(uint8_t addr);
bool get_acc_visibility_err(uint8_t addr);
bool get_acc_flicker(uint8_t addr);

void send_ack(uint8_t addr);
void send_error(uint8_t addr, uint32_t ERROR_REASON);
void parse_message(I2C::i2c_message msg, Luminary* L);

}  // namespace I2C_message_protocol

#endif
