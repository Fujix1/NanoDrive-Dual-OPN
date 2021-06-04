/*
 * ファイル関係
 */

#ifndef __FILER_H
#define __FILER_H

#include <Arduino.h>
extern "C" {
  #include "fatfs/tf_card.h"
  #include "lcd/lcd.h"
}
#include "tick.hpp"

extern boolean vgmLoaded;  // VGM データが読み込まれた状態か
extern boolean vgmEnd;     // 曲終了

boolean     sd_init();
boolean     closeFile();
uint8_t     get_vgm_ui8();
uint8_t     get_vgm_ui8_at(FSIZE_t pos);
uint16_t    get_vgm_ui16();
uint16_t    get_vgm_ui16_at(FSIZE_t pos);
uint32_t    get_vgm_ui32_at(FSIZE_t pos);
void        pause(uint32_t samples);

boolean     openFile(char *);
void        vgmReady();
void        vgmProcess();
void        vgmOpen(int, int);
void        vgmPlay(int);
void        openDirectory(int);
int         mod(int i, int j);
#endif