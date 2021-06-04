
#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "Display/Display.hpp"
#include "tick.hpp"

class Displaycls {
 private:
  unsigned long startTime1, startTime2, startTime3;
  String dispText1, dispText2, dispText3;
  bool scroll1, scroll2, scroll3;
  unsigned int pos1, pos2, pos3;
  unsigned int len1, len2, len3;

 public:
  Displaycls();

  void update();

  void set1(String text);
  void set2(String text);
  void set3(String text);
  
};

extern Displaycls Display;

#endif