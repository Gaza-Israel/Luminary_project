#ifndef I2C_h
#define I2C_h
#include <CircularBuffer.h>
#include <Wire.h>

#include "Arduino.h"

namespace I2C {
typedef struct i2c_message {
  uint8_t node;
  uint8_t msg_id;
  uint32_t ts;
  uint32_t data;
  uint8_t pec;
} i2c_message;

static CircularBuffer<i2c_message, 30> in_buff;

static uint8_t this_pico_id[8];
static uint8_t i2c_address;
static uint8_t n_overflows = 0;
static uint8_t n_invalid = 0;

void config_I2C(int SDA0, int SDC0, int SDA1, int SDC1);
int send_message(i2c_message msg, uint8_t address);
void recv(int len);
void read_buffer();

bool buffer_not_empty();
i2c_message pop_message_from_buffer();

uint8_t calculate_pec(byte bytes[], int len);

uint8_t get_I2C1_address();
void print_I2C1_full_address();

}  // namespace I2C

#endif
