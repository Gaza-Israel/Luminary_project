#ifndef I2C_message_protocol_h
#define I2C_message_protocol_h
#include "I2C.h"

class Luminary;

#define N_LUMINARIES 2
#define N_STREAM_VARIABLES 13

/*
--------------------------------------------------------
                    Error Code
--------------------------------------------------------
*/
#define VARIABLE_NOT_FOUND_ERROR 1
#define COMMAND_INVALID_ERROR 2
/*
--------------------------------------------------------
                  General messages
--------------------------------------------------------
*/
#define ERROR_MSG_ID 0
#define ACK_MSG_ID 1
#define IM_READY 2
#define BROADCAST_MSG_ID 3
#define G_CALIB_START 4
#define SELF_CALIB_END 5
/*
--------------------------------------------------------
                    Set messages
--------------------------------------------------------
*/
#define SET_MESSAGES(id) (id >= 10 && id <= 18)
#define set_dc_MSG_ID 10
#define set_occ_lux_ref_MSG_ID 11
#define set_unocc_lux_ref_MSG_ID 12
#define set_occ_status_MSG_ID 13
#define set_anti_windup_status_MSG_ID 14
#define set_ff_status_MSG_ID 15
#define set_fb_status_MSG_ID 16
#define toggle_stream_MSG_ID 17
#define set_controller_gains_MSG_ID 18
/*
--------------------------------------------------------
                    Get messages
--------------------------------------------------------
*/
#define GET_MESSAGES(id) (id >= 30 && id <= 43)
#define get_dc_MSG_ID 30
#define get_lux_ref_MSG_ID 31
#define get_lux_meas_MSG_ID 32
#define get_occ_status_MSG_ID 33
#define get_anti_windup_status_MSG_ID 34
#define get_ff_status_MSG_ID 35
#define get_fb_status_MSG_ID 36
#define get_ext_ilu_MSG_ID 37
#define get_curr_pwr_MSG_ID 38
#define get_time_MSG_ID 39
#define get_hist_MSG_ID 40
#define get_acc_enrgy_comsumption_MSG_ID 41
#define get_acc_visibility_err_MSG_ID 42
#define get_acc_flicker_MSG_ID 43
/*
--------------------------------------------------------
                  Send messages
--------------------------------------------------------
*/
#define SEND_MESSAGES(id) (id >= 50 && id <= 91)
#define send_dc_MSG_ID 50
#define send_lux_ref_MSG_ID 51
#define send_lux_meas_MSG_ID 52
#define send_occ_status_MSG_ID 53
#define send_anti_windup_status_MSG_ID 54
#define send_ff_status_MSG_ID 55
#define send_fb_status_MSG_ID 56
#define send_ext_ilu_MSG_ID 57
#define send_curr_pwr_MSG_ID 58
#define send_time_MSG_ID 59
//-------------------------------------
#define TRANSMISSION_MESSAGES(id) (id >= 60 && id <= 73)
#define start_DC_HIST_TRANSMISSION_MSG_ID 60
#define start_ref_HIST_TRANSMISSION_MSG_ID 61
#define start_L_meas_HIST_TRANSMISSION_MSG_ID 62
#define start_L_pred_HIST_TRANSMISSION_MSG_ID 63
#define start_err_HIST_TRANSMISSION_MSG_ID 64
#define start_prop_HIST_TRANSMISSION_MSG_ID 65
#define start_int_HIST_TRANSMISSION_MSG_ID 66
#define start_u_ff_HIST_TRANSMISSION_MSG_ID 67
#define start_u_fb_HIST_TRANSMISSION_MSG_ID 68
#define start_u_HIST_TRANSMISSION_MSG_ID 69
#define start_pwr_HIST_TRANSMISSION_MSG_ID 70
#define start_ext_ilu_HIST_TRANSMISSION_MSG_ID 71
#define start_flicker_HIST_TRANSMISSION_MSG_ID 72
#define end_HIST_TRANSMISSION_MSG_ID 73
#define send_hist_MSG_ID 74
//-------------------------------------
#define STREAM_MESSAGES(id) (id >= 75 && id <= 87)
#define send_stream_DC 75
#define send_stream_ref 76
#define send_stream_L_meas 77
#define send_stream_L_pred 78
#define send_stream_err 79
#define send_stream_prop 80
#define send_stream_int 81
#define send_stream_u_ff 82
#define send_stream_u_fb 83
#define send_stream_u 84
#define send_stream_pwr 85
#define send_stream_ext_ilu 86
#define send_stream_flicker 87
//-------------------------------------88
#define send_acc_enrgy_comsumption_MSG_ID 89
#define send_acc_visibility_err_MSG_ID 90
#define send_acc_flicker_MSG_ID 91

namespace I2C_message_protocol {

extern float buff_recv_i2c_stream[N_STREAM_VARIABLES * N_LUMINARIES];
extern bool recv_new_i2c_stream[N_STREAM_VARIABLES * N_LUMINARIES];
extern uint8_t nodes_addr[N_STREAM_VARIABLES * N_LUMINARIES];
extern uint8_t n_addr_saved;
extern Luminary *L;

int addr_is_saved(uint8_t addr);
void save_addr(uint8_t addr);
void sort_addresses();

void broadcast_node(int id);

void g_calib_start();
void self_calib_end();

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

bool send_stream(uint8_t addr, uint8_t var, float data);
bool broadcast_dc(float value);

void send_ack(uint8_t addr,uint32_t ID);
void send_error(uint8_t addr, uint32_t ERROR_REASON);
void parse_message(I2C::i2c_message msg);
void parse_set(I2C::i2c_message msg);
void parse_get(I2C::i2c_message msg);
void parse_get_hist(I2C::i2c_message msg);
void parse_send(I2C::i2c_message msg);
void parse_send_transmission(I2C::i2c_message msg);
void parse_send_stream(I2C::i2c_message msg);

}  // namespace I2C_message_protocol

#endif
