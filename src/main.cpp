/*
 *  VGM player on Longan Nano
 *  Fujix
 */

#include <Arduino.h>
extern "C" {
#include "fatfs/tf_card.h"
#include "lcd/lcd.h"
}
#include "Display/Display.hpp"
#include "FM/FM.hpp"
#include "PT2257/PT2257.hpp"
#include "SI5351/SI5351.hpp"
#include "file.hpp"
#include "tick.hpp"

void setup() {
  Lcd_Init();        // LCD 初期化
  LCD_Clear(BLACK);  //

  gpio_pin_remap_config(GPIO_SWJ_NONJTRST_REMAP,
                        ENABLE);  // PB4 のリマップを有効化

  LCD_ShowString(0, 0, (u8 *)("Initializing I2C.        "), WHITE);
  Wire.begin();  // I2C 初期化

  Tick.delay_ms(64);

  LCD_ShowString(0, 0, (u8 *)("Initializing PT2257.     "), WHITE);
  PT2257.begin();  // PT2257 初期化

  LCD_ShowString(0, 0, (u8 *)("Initializing SI5351.     "), WHITE);
  SI5351.begin();  // SI5351 起動
  SI5351.setFreq(SI5351_3579);
  SI5351.enableOutputs(true);

  LCD_ShowString(0, 0, (u8 *)("Starting FM.             "), WHITE);
  FM.begin();
  FM.reset();  // FMリセット

  Tick.delay_ms(200);  // SDカード安定用
  sd_init();           // ファイル初期化

  vgmPlay(0);  // 再生開始
}

byte lastButton;
boolean flag = false;

void loop() {
  Display.update();  // LCDの文字表示更新

  // VGMの稼働
  if (vgmLoaded) {
    vgmProcess();
  }
}