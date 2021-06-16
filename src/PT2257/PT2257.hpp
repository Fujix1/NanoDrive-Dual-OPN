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
  // Aカーブ傾斜した dB
  // 0 最大音量
  // 99 最小音量
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
  void reset(uint8_t);  // 音量、ステートリセット, 引数: 減衰db
  void set_volume(uint8_t);
  void set_db(uint8_t);
  void mute();
  void unmute();
  void start_fadeout();
  bool process_fadeout();
};

extern PT2257cls PT2257;

#endif