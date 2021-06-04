/*

  Simple Wire
  I2C lib for Longan Nano

  Based on
  https://github.com/riscv-mcu/GD32VF103_Firmware_Library/blob/master/Examples/I2C/Master_transmitter/main.c
  Note: Still no reading functionality.

*/

#include "Wire.hpp"
#define I2C0_OWN_ADDRESS7 0x72

void TwoWire::begin(void) {
  /* enable GPIOB clock */
  rcu_periph_clock_enable(RCU_GPIOB);

  /* enable I2C0 clock */
  rcu_periph_clock_enable(RCU_I2C0);

  /* connect PB6 to I2C0_SCL */
  /* connect PB7 to I2C0_SDA */
  gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);

  /* I2C clock configure */
  i2c_clock_config(I2C0, 400000, I2C_DTCY_2);  // 400kHz
  /* I2C address configure */
  i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS,
                       I2C0_OWN_ADDRESS7);
  /* enable I2C0 */
  i2c_enable(I2C0);
  /* enable acknowledge */
  i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}

void TwoWire::beginTransmission(uint8_t address) {
  /* wait until I2C bus is idle */
  while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
  /* send a start condition to I2C bus */
  i2c_start_on_bus(I2C0);
  /* wait until SBSEND bit is set */
  while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));

  /* send slave address to I2C bus */
  i2c_master_addressing(I2C0, address << 1, I2C_TRANSMITTER);
  /* wait until ADDSEND bit is set */
  while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
  /* clear ADDSEND bit */
  i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
}

size_t TwoWire::write(uint8_t dat) {
  /* data transmission */
  i2c_data_transmit(I2C0, dat);
  /* wait until the TBE bit is set */
  while (!i2c_flag_get(I2C0, I2C_FLAG_TBE));
  return 1;
}

size_t TwoWire::write(const uint8_t *dat, size_t quantity) {
  for (size_t i = 0; i < quantity; i++) {
    /* data transmission */
    i2c_data_transmit(I2C0, dat[i]);
    /* wait until the TBE bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_TBE));
  }
  return quantity;
}

void TwoWire::endTransmission(void) {
  /* send a stop condition to I2C bus */
  i2c_stop_on_bus(I2C0);
  /* wait until stop condition generate */
  while (I2C_CTL0(I2C0) & I2C_CTL0_STOP)
    ;
}

TwoWire Wire;
