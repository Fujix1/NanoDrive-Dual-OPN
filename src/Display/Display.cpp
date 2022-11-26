/*
 * Utility class for LCD drawing
 */

#include "Display/Display.hpp"
extern "C" {
#include "lcd/lcd.h"
}

#define DISPLAY_SCROLLDELAY 1000  // スクロールが開始するまで ms
#define DISPLAY_SCROLLSPEED 300   // スクロール速度 ms

#define DISPLAY_SET1COLOR RED // 1行目の色
#define DISPLAY_SET1DIGITS 19

#define DISPLAY_SET2COLOR WHITE // 2行目の色
#define DISPLAY_SET2DIGITS 38

#define DISPLAY_SET3COLOR GRAYBLUE  // 3行目の色
#define DISPLAY_SET3DIGITS 38

Displaycls::Displaycls() { 

}

// 1行目の初期化
void Displaycls::set1(String text) {
  dispText1 = text;
  dispText1.concat("                                      ");
  text = dispText1.substring(0, DISPLAY_SET1DIGITS);
  LCD_ShowString(0, 0, (u8 *)(text.c_str()), DISPLAY_SET1COLOR);
}

// 2行目の初期化
void Displaycls::set2(String text) {
  dispText2 = text;
  pos2 = 0;
  startTime2 = Tick.millis2();

  // スクロールさせるか判断
  if (text.length() <= DISPLAY_SET2DIGITS) {
    scroll2 = false;
    dispText2.concat("                                      ");
    dispText2 = dispText2.substring(0, DISPLAY_SET2DIGITS);
  } else {
    scroll2 = true;
    dispText2.concat("  ***  ");
    len2 = dispText2.length();
    dispText2.concat(text);
  }
  text = dispText2.substring(0, DISPLAY_SET2DIGITS);
  LCD_ShowString(0, 16, (u8 *)(text.c_str()), DISPLAY_SET2COLOR);
}

// 3行目の初期化
void Displaycls::set3(String text) {
  dispText3 = text;
  dispText3.concat("                                      ");
  text = dispText3.substring(0, DISPLAY_SET3DIGITS);
  LCD_ShowString(0, 48, (u8 *)(text.c_str()), DISPLAY_SET3COLOR);
}

boolean Displaycls::update() {
  boolean updated = false;
  uint32_t now = Tick.millis2();
  unsigned int newPos2;

  if (scroll2) {
    if (startTime2 + DISPLAY_SCROLLDELAY < now) {
      newPos2 = (now - startTime2 - DISPLAY_SCROLLDELAY) / DISPLAY_SCROLLSPEED;
      if (pos2 != newPos2) {
        pos2 = newPos2;
        String text = dispText2.substring(pos2, DISPLAY_SET2DIGITS + pos2);
        LCD_ShowString(0, 16, (u8 *)(text.c_str()), DISPLAY_SET2COLOR);
        if ( pos2 == len2 ) {
          pos2 = 0;
          startTime2 = Tick.millis2();
          updated = true;
        }
      }
    }
  }

  return updated;
}

Displaycls Display;