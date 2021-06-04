/*
 * M62429 Serial Volume Controller
 *
 * Original information:
 * http://fanoutsendai-yagiyama.blogspot.com/2017/11/arduinofm62845.html
 */

#ifndef __M62429_H
#define __M62429_H

#include "Wire/Wire.hpp"
#include "tick.hpp"

class M62429cls {
 private:
  // Aカーブに傾斜した dB
  // 0 最大音量
  // 99 最小音量
  /*  uint8_t db[100] = {0,  1,  1,  2,  2,  2,  2,  3,  3,  4,  4,  4,  4,  5,
     6, 6,  6,  7,  7,  8,  8,  8,  9,  9,  9,  10, 10, 10, 10, 10, 11, 11, 12,
     12, 12, 12, 12, 13, 13, 14, 14, 14, 15, 15, 16, 16, 16, 17, 18, 18, 18, 19,
     20, 20, 20, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 26,
     26, 26, 27, 27, 28, 28, 29, 29, 29, 30, 31, 31, 32, 33, 33, 35, 35, 37, 37,
                       39, 40, 41, 43, 45, 48, 51, 56, 64, 83};*/
  uint8_t db[50] = {0,  1,  2,  2,  3,  4,  4,  6,  6,  7,  8,  9,  9,
                    10, 10, 11, 12, 12, 12, 13, 14, 15, 16, 16, 18, 18,
                    20, 20, 21, 22, 22, 23, 23, 24, 25, 26, 26, 27, 28,
                    29, 30, 31, 33, 35, 37, 39, 42, 46, 58, 83};
  bool muted;
  uint8_t currentVolume;
  bool fadeOutIsOn;           // フェードアウト中
  unsigned long fadeOutTick;  // フェードアウト開始

  byte evc_level(uint8_t dB);

 public:
  M62429cls();
  void begin();
  void reset();  // 音量、ステートリセット
  void set_volume(uint8_t);
  void set_db(uint8_t);
  void mute();
  void unmute();
  void start_fadeout();
  bool process_fadeout();
};

extern M62429cls M62429;

#endif