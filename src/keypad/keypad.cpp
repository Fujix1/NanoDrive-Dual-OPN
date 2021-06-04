
#include "keypad.hpp"

KeypadClass::KeypadClass() {
  rcu_periph_clock_enable(RCU_GPIOA);
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_6);

  RCU_CFG0 |= (0b10 << 14) | (1 << 28); // ADC clock = 108MHz / 8 = 13.5MHz(14MHz max.)
  rcu_periph_clock_enable(RCU_ADC0);
  ADC_CTL1(ADC0) |= ADC_CTL1_ADCON;
}

uint16_t KeypadClass::get_adc(int ch) {
  ADC_RSQ2(ADC0) = 0;
  ADC_RSQ2(ADC0) = ch;
  ADC_CTL1(ADC0) |= ADC_CTL1_ADCON;

  while( !(ADC_STAT(ADC0) & ADC_STAT_EOC) );

  uint16_t ret = ADC_RDATA(ADC0) & 0xFFFF;
  ADC_STAT(ADC0) &= ~ADC_STAT_EOC;
  return ret;
}

//----------------------------------------------------------------------
// ボタンの状態取得
byte KeypadClass::readButton() {
  /*
  ボタン 押して無いとき 4095
  ボタン1 2985 - 2995
  ボタン2 2051 - 2065
  ボタン3 1375 - 1385
  ボタン4 582 - 592
  ボタン5 8 - 20
  */
 
  //adc_software_trigger_enable(ADC0, ADC_INSERTED_CHANNEL);
  //int adcData = ADC_IDATA0(ADC0);
  uint16_t adcData = get_adc(6);

  //LCD_ShowNum(0, 64, adcData, 4, GREEN); // デバッグ用 実際の入力値表示
  if (adcData > 3900) return btnNONE;                           // 4095, 戻り値 5
  if (adcData < 200 && adcData >=0) return btnRIGHT;             // 0, 戻り値 0
  if (adcData < 800 && adcData >= 500)  return btnUP;           // 585 - 595 , 戻り値 1
  if (adcData < 1600 && adcData >= 1200) return btnDOWN;        // 1375 - 1385 , 戻り値 2
  if (adcData < 2250 && adcData >= 2000 ) return btnLEFT;       // 2051 - 2065 , 戻り値 3
  if (adcData < 3200 && adcData >= 2900 ) return btnSELECT;     // 2985 - 2995 , 戻り値 4
  return btnNONE;
}

byte KeypadClass::checkButton() {
  byte anaInput;
  uint32_t ms = Tick.millis2();
  if (ms > buttonLastTick + BUTTON_INTERVAL) {  // 最後に押したときから
                                                // BUTTON_INTERVAL 経過してるとき
    anaInput = readButton();                   // ボタン取得
    if (lastAnalogInput != anaInput) {         // 前回のボタンと違えば
      lastAnalogInput = anaInput;
      if (anaInput != btnNONE) {
        buttonRepeatStarted = ms;
        return anaInput;
      }
    } else if (anaInput != 0 && lastAnalogInput == anaInput &&
               ms - buttonRepeatStarted > REPEAT_DELAY) {
      // 同じキーの場合はリピートディレイを過ぎているとき
      return anaInput;
    }
    buttonLastTick = ms;
  }
  return btnNONE;
}

KeypadClass Keypad;
