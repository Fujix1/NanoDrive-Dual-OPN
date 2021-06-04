/*
 * tick関係
 */

#ifndef __TICK_H
#define __TICK_H

#include <Arduino.h>

class TickClass {
 private:
  unsigned long _tick;
 public:
  unsigned long micros2();
  unsigned long millis2();
  void delay_us(uint32_t);
  void delay_ms(uint32_t);
  void start();
  unsigned long stop();
};

extern TickClass Tick;

#endif