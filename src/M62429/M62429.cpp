/*
 * M62429 Serial Volume Controller
 *
 * Original information:
 * http://fanoutsendai-yagiyama.blogspot.com/2017/11/arduinofm62845.html
 */

#include "M62429/M62429.hpp"

#define FADEOUT_DURATION 8000  // ms
#define FADEOUT_STEPS 50

#define M62429_DATA PB9   // データ
#define M62429_CLOCK PB8  // クロック
#define M62429_DATA_HIGH (GPIO_BOP(GPIOB) = GPIO_PIN_9)
#define M62429_DATA_LOW (GPIO_BC(GPIOB) = GPIO_PIN_9)
#define M62429_CLOCK_HIGH (GPIO_BOP(GPIOB) = GPIO_PIN_8)
#define M62429_CLOCK_LOW (GPIO_BC(GPIOB) = GPIO_PIN_8)

#define PT2257_ADDR 0x44       // Chip address
#define EVC_OFF 0b11111111     // Function OFF (-79dB)
#define EVC_2CH_1 0b11010000   // 2-Channel, -1dB/step
#define EVC_2CH_10 0b11100000  // 2-Channel, -10dB/step
#define EVC_L_1 0b10100000     // Left Channel, -1dB/step
#define EVC_L_10 0b10110000    // Left Channel, -10dB/step
#define EVC_R_1 0b00100000     // Right Channel, -1dB/step
#define EVC_R_10 0b00110000    // Right Channel, -10dB/step
#define EVC_MUTE 0b01111000    // 2-Channel MUTE


M62429cls::M62429cls() {
  muted = false;
  currentVolume = 0;
  fadeOutIsOn = false;
}

void M62429cls::reset() {
  fadeOutIsOn = false;
  set_db(0);
}

void M62429cls::begin() {
  pinMode(M62429_DATA, OUTPUT);
  pinMode(M62429_CLOCK, OUTPUT);
  fadeOutIsOn = false;
  mute();
}

// set new volume in dB
void M62429cls::set_db(uint8_t volume) {
  if (currentVolume == volume) return;  // 変更なければ戻る

  currentVolume = volume;

  uint8_t bits;
  uint16_t data = 0;  // control word is built by OR-ing in the bits

  if (volume > 83) {  // mute
    data |= (0 << 0);
    data |= (0 << 0);     // D0 (channel select: 0=ch1, 1=ch2)
    data |= (0 << 1);     // D1 (individual/both select: 0=both, 1=individual)
    data |= (0b11 << 9);  // D9...D10 // D9 & D10 must both be 1
  } else {
    // generate 10 bits of data
    data |= (0 << 0);  // D0 (channel select: 0=ch1, 1=ch2)
    data |= (0 << 1);  // D1 (individual/both select: 0=both, 1=individual)
    data |= ((21 - (volume / 4)) << 2);  // D2...D6 (ATT1: coarse attenuator:
                                         // 0,-4dB,-8dB, etc.. steps of 4dB)
    data |=
        ((3 - (volume % 4))
         << 7);  // D7...D8 (ATT2: fine attenuator: 0...-1dB... steps of 1dB)
    data |= (0b11 << 9);  // D9...D10 // D9 & D10 must both be 1
  }

  for (bits = 0; bits < 11; bits++) {  // send out 11 control bits
    M62429_DATA_LOW;
    // Tick.delay_us(1); // なくても動く
    M62429_CLOCK_LOW;
    // Tick.delay_us(1); // なくても動く
    if (((data >> bits) & 0x1) == 1) {
      M62429_DATA_HIGH;
    } else {
      // M62429_DATA_LOW; // いらない
    }
    Tick.delay_us(1);
    M62429_CLOCK_HIGH;
    // Tick.delay_us(1); // なくても動く
  }
  M62429_DATA_HIGH;
  // Tick.delay_us(1); // なくても動く
  M62429_CLOCK_LOW;



  byte bbbaaaa = evc_level(volume);

  byte aaaa = bbbaaaa & 0b00001111;
  byte bbb = (bbbaaaa >> 4) & 0b00001111;

  Wire.beginTransmission(PT2257_ADDR);
  Wire.write(EVC_2CH_10 | bbb);
  Wire.write(EVC_2CH_1 | aaaa);
  Wire.endTransmission();
}

byte M62429cls::evc_level(uint8_t dB) {
  if (dB > 79) dB = 79;

  uint8_t b = dB / 10;  // get the most significant digit (eg. 79 gets 7)
  uint8_t a = dB % 10;  // get the least significant digit (eg. 79 gets 9)

  b = b & 0b0000111;  // limit the most significant digit to 3 bit (7)

  return (b << 4) | a;  // return both numbers in one byte (0BBBAAAA)
}

// 音量設定 (Aカーブ)
// 0 = 最大音量
// FADEOUT_STEPS-1 = 最小音量
void M62429cls::set_volume(uint8_t volume) {
  if (volume > FADEOUT_STEPS - 1) volume = FADEOUT_STEPS - 1;
  set_db(db[volume]);
}

void M62429cls::mute() {
  set_db(FADEOUT_STEPS - 1);
  muted = true;
}

void M62429cls::unmute() {
  muted = false;
  set_db(0);
}

void M62429cls::start_fadeout() {
  if (!fadeOutIsOn) {
    fadeOutIsOn = true;
    fadeOutTick = Tick.millis2();
  }
}

bool M62429cls::process_fadeout() {
  if (!fadeOutIsOn) return false;

  unsigned long tick = Tick.millis2() - fadeOutTick;

  if (tick < FADEOUT_DURATION) {
    uint8_t delta = (float)(tick) / FADEOUT_DURATION * FADEOUT_STEPS;
    set_volume(delta);
    return false;
  } else {
    fadeOutIsOn = false;
    return true;
  }
}

M62429cls M62429;