/*
 * PT2257 I2C Volume Controller
 * https://github.com/victornpb/Evc_pt2257/blob/master/Evc_pt2257.cpp
 */

#ifndef __PT2257_H
#define __PT2257_H

#include "Wire/Wire.hpp"
#include "tick.hpp"

class PT2257cls {
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
  uint8_t db[50] = {0,  1,  1,  2,  3,  4,  5,  6,  7,  8,  10, 10, 11,
                    12, 12, 13, 13, 15, 15, 17, 17, 18, 20, 22, 23, 24,
                    25, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 33,
                    34, 35, 36, 37, 39, 41, 44, 47, 51, 59, 79};
  bool muted;
  uint8_t currentVolume;
  bool fadeOutIsOn;           // フェードアウト中
  unsigned long fadeOutTick;  // フェードアウト開始

  byte evc_level(uint8_t dB);

 public:
  PT2257cls();
  void begin();
  void reset( uint8_t );  // 音量、ステートリセット, 引数: 減衰db
  void set_volume(uint8_t);
  void set_db(uint8_t);
  void mute();
  void unmute();
  void start_fadeout();
  bool process_fadeout();
};

extern PT2257cls PT2257;

#endif