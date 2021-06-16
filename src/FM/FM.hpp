#ifndef FM_H
#define FM_H

#include <Arduino.h>

#include "tick.hpp"

#define CS0 0
#define CS1 1
#define CS2 2

class FMChip {
 public:
  void begin();
  void reset();
  void set_register(byte addr, byte value, uint8_t chipno);
  void write(byte data, byte chipno);
  void writeRaw(byte data, byte chipno);

 private:
  unsigned char psgFrqLowByte = 0;
};

extern FMChip FM;

#endif
