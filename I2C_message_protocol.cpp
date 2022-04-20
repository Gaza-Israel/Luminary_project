#include "I2C_message_protocol.h"


namespace I2C_message_protocol {
void broadcast_node(int id) {
  I2C::i2c_message msg;
  msg.msg_id = 1;
  msg.data = id << 8 | I2C::get_I2C1_address();
  I2C::send_message(msg, 0x00);
}
}  // namespace I2C_message_protocol