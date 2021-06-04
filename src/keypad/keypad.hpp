/*
   キーパッド用ライブラリ
   基本構成は Arduino 用 5つボタンキーパッドと同じ構成

   ADC info thanks to
   http://blueeyes.sakura.ne.jp/2019/10/26/2681/

  ボタン 押して無いとき 4095 -> 電圧降下時 4070
  ボタン1 2985 - 2995 -> 電圧降下時 3020
  ボタン2 2051 - 2065 -> 電圧降下時 2120
  ボタン3 1375 - 1385 -> 電圧降下時 1470
  ボタン4 582 - 592 -> 電圧降下時 720
  ボタン5 8 - 20  -> 電圧降下時 170

  回路
           3.3V
            │
          [2kΩ]
   (ADCへ)──┼───(ボタン1)── GND
          [330Ω]
            ├───(ボタン2)── GND
          [620Ω]
            ├───(ボタン3)── GND
          [1kΩ]
            ├───(ボタン4)── GND
          [3.3kΩ]
            └───(ボタン5)── GND
 */

#ifndef __KEYPAD_H
#define __KEYPAD_H

#include <Arduino.h>
extern "C" {
#include "lcd/lcd.h"
}
#include "tick.hpp"

#define BUTTON_INTERVAL 64   // ボタン取得間隔 ms
#define REPEAT_DELAY 500     // ボタンリピート開始まで ms

class KeypadClass {
 private:
  uint32_t buttonLastTick = 0;         // 最後にボタンが押された時間
  uint32_t buttonRepeatStarted = 0;    // リピート開始時間
  byte lastAnalogInput = btnNONE;
  byte readButton();
  uint16_t get_adc(int ch);
 public:
  enum Button { btnNONE, btnRIGHT, btnUP, btnDOWN, btnLEFT, btnSELECT };
  KeypadClass();
  byte checkButton();
};

extern KeypadClass Keypad;

#endif
