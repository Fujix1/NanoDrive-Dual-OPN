#include "FM.hpp"
extern "C" {
#include "lcd/lcd.h"
}
// Output Pins
#define D0 PC15
#define D1 PC14
#define D2 PC13
#define D3 PA0
#define D4 PA1
#define D5 PA2
#define D6 PA3
#define D7 PA4

#define A0 PB5
#define WR PB4
#define CS0_PIN PA12
#define CS1_PIN PA11
#define IC PA8

//#define CS2_PIN PB5

#define A0_HIGH (GPIO_BOP(GPIOB) = GPIO_PIN_5)
#define A0_LOW (GPIO_BC(GPIOB) = GPIO_PIN_5)
#define WR_HIGH (GPIO_BOP(GPIOB) = GPIO_PIN_4)
#define WR_LOW (GPIO_BC(GPIOB) = GPIO_PIN_4)
#define IC_HIGH (GPIO_BOP(GPIOA) = GPIO_PIN_8)
#define IC_LOW (GPIO_BC(GPIOA) = GPIO_PIN_8)

#define CS0_HIGH (GPIO_BOP(GPIOA) = GPIO_PIN_12)  // HIGH
#define CS0_LOW (GPIO_BC(GPIOA) = GPIO_PIN_12)    // LOW
#define CS1_HIGH (GPIO_BOP(GPIOA) = GPIO_PIN_11)  // HIGH
#define CS1_LOW (GPIO_BC(GPIOA) = GPIO_PIN_11)    // LOW
//#define CS2_HIGH (GPIO_BOP(GPIOB) = GPIO_PIN_5)  // HIGH
//#define CS2_LOW  (GPIO_BC(GPIOB)  = GPIO_PIN_5)  // LOW

/*
  YM**** -> SN76489AN
    WR -> WE
    CS -> CE
*/

void FMChip::begin() {
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);

  pinMode(WR, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(IC, OUTPUT);

  pinMode(CS0_PIN, OUTPUT);
  pinMode(CS1_PIN, OUTPUT);
  // pinMode(CS2_PIN, OUTPUT);
}

void FMChip::reset(void) {
  CS0_LOW;
  CS1_LOW;
  // CS2_LOW;
  WR_HIGH;
  A0_LOW;
  IC_LOW;
  Tick.delay_us(100);  // at least 12 cycles // at 1.5MHz: 0.67us
  IC_HIGH;
  CS0_HIGH;
  CS1_HIGH;
  // CS2_HIGH;
  Tick.delay_us(100);
}

void FMChip::write(byte data, byte chipno = CS1) {
  if ((data & 0x80) == 0) {
    if ((psgFrqLowByte & 0x0F) == 0) {
      if ((data & 0x3F) == 0) psgFrqLowByte |= 1;
    }
    writeRaw(psgFrqLowByte, chipno);
    writeRaw(data, chipno);
  } else if ((data & 0x90) == 0x80 && (data & 0x60) >> 5 != 3)
    psgFrqLowByte = data;
  else
    writeRaw(data, chipno);
}

void FMChip::writeRaw(byte data, byte chipno = CS1) {
  uint32_t data_bits = 0;

  switch (chipno) {
    case CS0:
      CS0_LOW;
      CS1_HIGH;
      // CS2_HIGH;
      break;
    case CS1:
      CS0_HIGH;
      CS1_LOW;
      // CS2_HIGH;
      break;
      /*    case CS2:
            CS0_HIGH;
            CS1_HIGH;
            //CS2_LOW;
            break;*/
  }

  WR_HIGH;

  //---------------------------------------
  // data
  if (data & 0b00000001) {
    data_bits += GPIO_PIN_15;
  }
  if (data & 0b00000010) {
    data_bits += GPIO_PIN_14;
  }
  if (data & 0b00000100) {
    data_bits += GPIO_PIN_13;
  }
  // LOW
  GPIO_BC(GPIOC) = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_BC(GPIOA) =
      GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0;

  // HIGH
  GPIO_BOP(GPIOC) = data_bits;
  GPIO_BOP(GPIOA) = (data & 0b11111000) >> 3;
  /*  if (data & 0b10000000) {
      GPIO_BOP(GPIOA) = GPIO_PIN_6;
    }*/
  // コントロールレジスタに登録するには WR_LOW → WR_HIGH 最低32クロック
  // 4MHz     :　0.25us   * 32 = 8 us
  // 3.579MHz :  0.2794us * 32 = 8.94 us
  // 1.5MHz   :  0.66us   * 32 = 21.3 us
  WR_LOW;
  Tick.delay_us(21);
  WR_HIGH;
  CS0_HIGH;
  CS1_HIGH;
  // CS2_HIGH;
}

void FMChip::set_register(byte addr, byte data, uint8_t chipno = CS0) {
  // LOW
  GPIO_BC(GPIOC) = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_BC(GPIOA) =
      GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0;
  // HIGH
  GPIO_BOP(GPIOC) =
      ((addr & 0b1) << 15) | ((addr & 0b10) << 13) | ((addr & 0b100) << 11);
  GPIO_BOP(GPIOA) = (addr & 0b11111000) >> 3;  // D3, D4 ,D5, D6, D7
  A0_LOW;
  switch (chipno) {
    case CS0:
      CS0_LOW;
      CS1_HIGH;
      break;
    case CS1:
      CS0_HIGH;
      CS1_LOW;
      break;
  }
  WR_LOW;  // TWW >= 200ns
  Tick.delay_500ns();
  WR_HIGH;
  A0_HIGH;

  //---------------------------------------
  // data

  Tick.delay_us(6);  // 6が限界 Darius

  // LOW
  GPIO_BC(GPIOC) = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_BC(GPIOA) =
      GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0;
  // HIGH
  GPIO_BOP(GPIOC) =
      ((data & 0b1) << 15) | ((data & 0b10) << 13) | ((data & 0b100) << 11);
  GPIO_BOP(GPIOA) = (data & 0b11111000) >> 3;

  WR_LOW;
  Tick.delay_500ns();
  WR_HIGH;
  CS0_HIGH;

  Tick.delay_us(16);  // 16 が限界  Dragon Slayer
}

FMChip FM;
