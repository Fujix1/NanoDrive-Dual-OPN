/*
 * PT2257 I2C Volume Controller
 * Thanks to https://github.com/victornpb/Evc_pt2257/blob/master/Evc_pt2257.cpp
 */

#include "PT2257/PT2257.hpp"

#define FADEOUT_DURATION 8000  // ms
#define FADEOUT_STEPS 50

#define PT2257_ADDR 0x44        // Chip address
#define EVC_OFF     0b11111111  // Function OFF (-79dB)
#define EVC_2CH_1   0b11010000  // 2-Channel, -1dB/step
#define EVC_2CH_10  0b11100000  // 2-Channel, -10dB/step
#define EVC_L_1     0b10100000  // Left Channel, -1dB/step
#define EVC_L_10    0b10110000  // Left Channel, -10dB/step
#define EVC_R_1     0b00100000  // Right Channel, -1dB/step
#define EVC_R_10    0b00110000  // Right Channel, -10dB/step
#define EVC_MUTE    0b01111001  // 2-Channel MUTE
#define EVC_UNMUTE  0b01111000  // 2-Channel UNMUTE

uint8_t attenuation; // 音量調整 db

PT2257cls::PT2257cls() {
  muted = false;
  currentVolume = 79;
  fadeOutIsOn = false;
  attenuation = 0;
}

void PT2257cls::reset(uint8_t att) {
  attenuation = att;
  fadeOutIsOn = false;
  unmute();
  set_db(0);
}

void PT2257cls::begin() {
  attenuation = 0;
  fadeOutIsOn = false;
  mute();
  set_db(0);
}

// set new volume in dB
void PT2257cls::set_db(uint8_t volume) {
  volume += attenuation;  // Att
  if (currentVolume == volume) return;  // 変更なければ戻る

  currentVolume = volume;

  byte bbbaaaa = evc_level(volume);
  byte aaaa = bbbaaaa & 0b00001111;
  byte bbb = (bbbaaaa >> 4) & 0b00001111;

  Wire.beginTransmission(PT2257_ADDR);
  Wire.write(EVC_2CH_10 | bbb);
  Wire.write(EVC_2CH_1 | aaaa);
  Wire.endTransmission();
}

byte PT2257cls::evc_level(uint8_t dB) {
  if (dB > 79) dB = 79;

  uint8_t b = dB / 10;  // get the most significant digit (eg. 79 gets 7)
  uint8_t a = dB % 10;  // get the least significant digit (eg. 79 gets 9)

  b = b & 0b0000111;  // limit the most significant digit to 3 bit (7)

  return (b << 4) | a;  // return both numbers in one byte (0BBBAAAA)
}

// 音量設定 (Aカーブ)
// 0 = 最大音量
// FADEOUT_STEPS-1 = 最小音量
void PT2257cls::set_volume(uint8_t volume) {
  if (volume > FADEOUT_STEPS - 1) volume = FADEOUT_STEPS - 1;
  set_db(db[volume]);
}

void PT2257cls::mute() {
  Wire.beginTransmission(PT2257_ADDR);
  Wire.write(EVC_MUTE);
  Wire.endTransmission();
  muted = true;
}

void PT2257cls::unmute() {
  Wire.beginTransmission(PT2257_ADDR);
  Wire.write(EVC_UNMUTE);
  Wire.endTransmission();
  muted = false;
}

void PT2257cls::start_fadeout() {
  if (!fadeOutIsOn) {
    fadeOutIsOn = true;
    fadeOutTick = Tick.millis2();
  }
}

bool PT2257cls::process_fadeout() {
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

PT2257cls PT2257;