/*
 * tick関係
 */

#include "tick.hpp"

unsigned long TickClass::micros2() {
  return (unsigned long)(get_timer_value() / 27);
}

unsigned long TickClass::millis2() {
  return (unsigned long)(get_timer_value() / 27000);
}
void TickClass::delay_500ns() {
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
}

void TickClass::delay_us(uint32_t us) {
  /*  unsigned long tick;
    uint32_t us27 = us * 27;
    tick = micros2() * 27;
    while (get_timer_value() - tick < us27);
  */
  while (us > 0) {
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    us--;
  }
}

void TickClass::delay_ms(uint32_t ms) { delay_us(ms * 1000); }

void TickClass::start() { _tick = micros2(); }

unsigned long TickClass::stop() { return micros2() - _tick; }

TickClass Tick;