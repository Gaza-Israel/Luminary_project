
#include "I2C.h"

#include <hardware/flash.h>

#define FRAME_SIZE 11

namespace I2C {

void config_I2C(int SDA0, int SCL0, int SDA1, int SCL1) {
  Wire.setSDA(SDA0);
  Wire.setSCL(SCL0);
  Wire.setClock(100000);
  Wire.begin();

  flash_get_unique_id(this_pico_id);
  i2c_address = this_pico_id[6] & 0b01111111;  // least significant byte - make sure it is not a reserved slave address

  Wire1.setSDA(SDA1);
  Wire1.setSCL(SCL1);
  Wire1.setClock(100000);
  Wire1.begin(i2c_address);
  Wire1.onReceive(recv);
}

int send_message(i2c_message msg, uint8_t address) {
  byte tx_buff[FRAME_SIZE];  // size 11 (struct pad to 16, but only 11 are necessary)

  msg.node = get_I2C1_address();
  msg.ts = millis();

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
  tx_buff[10] = calculate_pec(tx_buff, FRAME_SIZE - 1);

  Wire.beginTransmission(address);
  Wire.write(tx_buff, sizeof(tx_buff));
  return Wire.endTransmission();
}

void recv(int len) {
  I2C::i2c_message msg;
  byte rx_buff[FRAME_SIZE] = {0};

  for (int i = 0; i < len; i++) {
    rx_buff[i] = Wire1.read();
  }

  if (calculate_pec(rx_buff, FRAME_SIZE) != 0) {
    n_invalid++;
    return;
  }

  msg.node = rx_buff[0];
  msg.msg_id = rx_buff[1];
  msg.ts = rx_buff[2] << 0 | rx_buff[3] << 8 | rx_buff[4] << 16 | rx_buff[5] << 24;
  msg.data = rx_buff[6] << 0 | rx_buff[7] << 8 | rx_buff[8] << 16 | rx_buff[9] << 24;
  msg.pec = rx_buff[10];

  if (in_buff.isFull()) {
    n_overflows++;
    return;
  }
  in_buff.unshift(msg);
}

void read_buffer() {
  if (in_buff.size() > 0) {
    I2C::i2c_message msg = in_buff.pop();
    char buff[100];
    snprintf(buff, 100, "Received message from %d, at %d, with id %d, data %d, and CRC-8 of %d", msg.node, msg.ts, msg.msg_id, msg.data, msg.pec);
    Serial.println(buff);
  }
}
bool buffer_not_empty(){
  return in_buff.size() != 0;
}

i2c_message pop_message_from_buffer(){
  return in_buff.pop();
}

uint8_t calculate_pec(byte bytes[], int len) {
  const byte generator = 0x1D;
  byte crc = 0; /* start with 0 so first byte can be 'xored' in */
  for (int i = 0; i < len; i++) {
    crc ^= bytes[i]; /* XOR-in the next input byte */

    for (int i = 0; i < 8; i++) {
      if ((crc & 0x80) != 0) { /* check if MSB is set */
        crc = (byte)((crc << 1) ^ generator);
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}

uint8_t get_I2C1_address() {
  return i2c_address;
}
void print_I2C1_full_address() {
  for (int i = 0; i < 8; i++) {
    Serial.print(this_pico_id[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
}

}  // namespace I2C