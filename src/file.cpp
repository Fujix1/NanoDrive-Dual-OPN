/*
 * Open file and process VGM data
 */
#include "file.hpp"

#include "Display/Display.hpp"
#include "FM/FM.hpp"
#include "PT2257/PT2257.hpp"
#include "SI5351/SI5351.hpp"
#include "keypad/keypad.hpp"

#define BUFFERCAPACITY 2048  // VGMの読み込み単位（バイト）
#define MAXLOOP 2            // 次の曲いくループ数
#define ONE_CYCLE 612u       // ソーサリアンで調整
                             // 22.67573696145125 * 27 = 612.24  // 1000000 / 44100

boolean mount_is_ok = false;
uint8_t currentDir;                 // 今のディレクトリインデックス
uint8_t currentFile;                // 今のファイルインデックス
uint8_t numDirs = 0;                // ルートにあるディレクトリ数
char **dirs;                        // ルートにあるディレクトリの配列
uint8_t *attenuations;              // 各ディレクトリの減衰量 (デシベル)
char ***files;                      // 各ディレクトリ内の vgm ファイル名配列
uint8_t *numFiles;                  // 各ディレクトリ内の vgm ファイル数

boolean fileOpened = false;          // ファイル開いてるか
uint8_t vgmBuffer1[BUFFERCAPACITY];  // バッファ
uint16_t bufferPos = 0;              // バッファ内の位置
uint16_t filesize = 0;               // ファイルサイズ
UINT bufferSize = 0;                 // 現在のバッファ容量

// File handlers
FATFS fs;
FIL fil;
FILINFO fno;
FRESULT fr;

// VGM Status
uint32_t vgmLoopOffset;     // ループデータの戻る場所
boolean vgmLoaded = false;  // VGM データが読み込まれた状態か
uint8_t vgmLoop = 0;        // 現在のループ回数
uint64_t startTime;
uint32_t duration;
int32_t vgmDelay = 0;

uint32_t compensation = 0;

//---------------------------------------------------------------
// Init and open SD card
// 初期化とSDオープン
// ファイル構造の読み込み
boolean sd_init() {
  int i, j, n;

  DIR dir;
  FILINFO fno;

  currentDir = 0;
  currentFile = 0;

  // Try until SD card is mounted
  // SD マウントするまで試行
  LCD_ShowString(0, 0, (u8 *)("Checking SD card."), WHITE);
  for (i = 0; i < 18; i++) {
    Tick.delay_ms(200);
    fr = f_mount(&fs, "", 1);
    // LCD_ShowNum(0, 64, i, 2, GREEN);
    LCD_ShowString(i * 8, 16, (u8 *)("."), GREEN);
    if (fr == 0) break;
  }

  if (fr == 0) {
    mount_is_ok = true;
    LCD_ShowString(0, 0, (u8 *)("Reading directories..."), GRAY);

    //-------------------------------------------------------
    // ルートのフォルダ一覧を取得する
    // 数える
    fr = f_findfirst(&dir, &fno, "", "*");
    LCD_ShowString(0, 16, (u8 *)("ROOT"), CYAN);

    while (fr == FR_OK && fno.fname[0]) {  // 数える
      if (!(fno.fattrib & AM_SYS) && !(fno.fattrib & AM_HID) &&
          (fno.fattrib & AM_DIR)) {
        // システムじゃない && 隠しじゃない && ディレクトリ
        numDirs++;
      }
      f_findnext(&dir, &fno);
    }

    // ディレクトリが無い
    if (numDirs == 0) {
      LCD_ShowString(0, 0, (u8 *)("No directory."), WHITE);
      return false;
    }

    // 配列初期化
    dirs = (char **)malloc(sizeof(char *) * numDirs);
    for (i = 0; i < numDirs; i++) {
      dirs[i] = (char *)malloc(sizeof(char) * 13);  // メモリ確保
    }

    n = 0;
    fr = f_findfirst(&dir, &fno, "", "*");
    while (fr == FR_OK && fno.fname[0]) {
      if (!(fno.fattrib & AM_SYS) && !(fno.fattrib & AM_HID) &&
          (fno.fattrib & AM_DIR)) {
        // システムじゃない && 隠しじゃない && ディレクトリ
        strcpy(dirs[n++], fno.fname);
        LCD_ShowString(0, 32, (u8 *)(dirs[n - 1]), CYAN);
      }
      f_findnext(&dir, &fno);
    }

    // 各フォルダ内のファイル数に応じて確保
    files = (char ***)malloc(sizeof(char **) * numDirs);
    numFiles = new uint8_t[numDirs];
    attenuations = new uint8_t[numDirs];

    //-------------------------------------------------------
    // フォルダ内ファイル情報取得（数える）
    LCD_ShowString(0, 0, (u8 *)("READING FILES..."), GRAY);

    for (i = 0; i < numDirs; i++) {
      LCD_ShowString(0, 16, (u8 *)("             "), CYAN);
      LCD_ShowString(0, 32, (u8 *)("             "), CYAN);
      LCD_ShowString(0, 16, (u8 *)(dirs[i]), CYAN);

      fr = f_findfirst(&dir, &fno, dirs[i], "*.VGM");
      n = 0;
      while (fr == FR_OK && fno.fname[0]) {
        n++;
        f_findnext(&dir, &fno);
      }

      // ファイル名保持用配列メモリ確保
      if (n > 0) {
        files[i] = (char **)malloc(sizeof(char *) * n);
        for (j = 0; j < n; j++) {
          files[i][j] = (char *)malloc(sizeof(char) * 13);
        }
      }
      // フォルダ内のファイル数保持配列設定
      numFiles[i] = n;

      // フォルダノーマライズ
      // 以下の名前を含むファイルがあれば全部 att* dB 下げる
      attenuations[i] = 0;
      fr = f_findfirst(&dir, &fno, dirs[i], "att2");
      if (fr == FR_OK && fno.fname[0]) {
        attenuations[i] = 2;
      }
      fr = f_findfirst(&dir, &fno, dirs[i], "att4");
      if (fr == FR_OK && fno.fname[0]) {
        attenuations[i] = 4;
      }
      fr = f_findfirst(&dir, &fno, dirs[i], "att6");
      if (fr == FR_OK && fno.fname[0]) {
        attenuations[i] = 6;
      }
      fr = f_findfirst(&dir, &fno, dirs[i], "att8");
      if (fr == FR_OK && fno.fname[0]) {
        attenuations[i] = 8;
      }
      fr = f_findfirst(&dir, &fno, dirs[i], "att10");
      if (fr == FR_OK && fno.fname[0]) {
        attenuations[i] = 10;
      }
      fr = f_findfirst(&dir, &fno, dirs[i], "att12");
      if (fr == FR_OK && fno.fname[0]) {
        attenuations[i] = 12;
      }
      fr = f_findfirst(&dir, &fno, dirs[i], "att14");
      if (fr == FR_OK && fno.fname[0]) {
        attenuations[i] = 14;
      }
    }

    // ファイル名取得
    if (n > 0) {
      for (i = 0; i < numDirs; i++) {
        fr = f_findfirst(&dir, &fno, dirs[i], "*.VGM");
        n = 0;
        while (fr == FR_OK && fno.fname[0]) {
          strcpy(files[i][n], fno.fname);
          // LCD_ShowString(0, 32, (u8 *)(files[i][n]), CYAN);
          // Tick.delay_ms(1000);
          n++;
          f_findnext(&dir, &fno);
        }
      }
    }

    return true;

  } else {
    mount_is_ok = false;
    LCD_ShowString(0, 0, (u8 *)("SD card mount Error"), WHITE);
    return false;
  }
}

boolean closeFile() {
  FRESULT fr;

  if (!mount_is_ok) return false;

  if (fileOpened) {
    fr = f_close(&fil);
    if (fr == FR_OK) {
      fileOpened = false;
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------
// 1バイト返す
uint8_t get_vgm_ui8() {
  if (bufferPos == bufferSize) {  // バッファの終端を超えた
    if (bufferSize == BUFFERCAPACITY) {
      bufferPos = 0;
      f_read(&fil, vgmBuffer1, BUFFERCAPACITY, &bufferSize);
    } else {
      LCD_ShowString(0, 64, (u8 *)("EOF"), WHITE);
      return 0;
    }
  }
  return vgmBuffer1[bufferPos++];
}

//----------------------------------------------------------------------
// 16 bit 返す
uint16_t get_vgm_ui16() { return get_vgm_ui8() + (get_vgm_ui8() << 8); }

//----------------------------------------------------------------------
// 指定場所の 8 bit 返す
uint8_t get_vgm_ui8_at(uint32_t pos) {
  uint32_t currentPos = fil.fptr;  // 一時退避
  uint8_t buffer[1];
  UINT br;

  f_lseek(&fil, pos);
  f_read(&fil, buffer, 1, &br);
  f_lseek(&fil, currentPos);

  return buffer[0];
}

//----------------------------------------------------------------------
// 指定場所の 16 bit 返す
uint16_t get_vgm_ui16_at(uint32_t pos) {
  uint32_t currentPos = fil.fptr;  // 一時退避
  uint8_t buffer[2];
  UINT br;

  f_lseek(&fil, pos);
  f_read(&fil, buffer, 2, &br);
  f_lseek(&fil, currentPos);

  return (uint32_t(buffer[0])) + (uint32_t(buffer[1]) << 8);
}

//----------------------------------------------------------------------
// 指定場所の 32 bit 返す
uint32_t get_vgm_ui32_at(uint32_t pos) {
  uint32_t currentPos = fil.fptr;  // 一時退避
  uint8_t buffer[4];
  UINT br;

  f_lseek(&fil, pos);
  f_read(&fil, buffer, 4, &br);
  f_lseek(&fil, currentPos);

  return (uint32_t(buffer[0])) + (uint32_t(buffer[1]) << 8) +
         (uint32_t(buffer[2]) << 16) + (uint32_t(buffer[3]) << 24);
}

//----------------------------------------------------------------------
// ファイルを開く
boolean openFile(char *path) {
  String sPath;

  if (!mount_is_ok) return false;

  fileOpened = false;
  filesize = 0;

  // LCD_Clear(BLACK);
  sPath = path;
  sPath.concat("              ");
  sPath = sPath.substring(0, 19);

  Display.set1(sPath);
  // LCD_ShowString(0, 0, (u8 *)(sPath.c_str()), RED);

  fr = f_open(&fil, path, FA_READ);
  if (fr == FR_OK) {
    fr = f_stat(
        path,
        &fno);  // ファイルサイズ取得 ※外すと f_seek が正しく動かないので注意
    filesize = fno.fsize;
    fileOpened = true;
    return true;
  } else {
    return false;
  }
}

//----------------------------------------------------------------------
// オープンした VGM ファイルを解析して再生準備する
void vgmReady() {
  uint8_t gd3buffer[512];
  UINT i, p;
  char c[2];
  UINT br;
  String gd3[10];

  vgmLoop = 0;
  vgmDelay = 0;

  // VGM Version
  uint32_t vgm_version = get_vgm_ui32_at(8);

  // VGM Loop offset
  vgmLoopOffset = get_vgm_ui32_at(0x1c);

  // VGM gd3 offset
  uint32_t vgm_gd3_offset = get_vgm_ui32_at(0x14) + 0x14;
  // LCD_ShowNum(0,32, vgm_gd3_offset, 6,RED);

  // GD3
  if (vgm_gd3_offset != 0) {
    f_lseek(&fil, vgm_gd3_offset + 12);
    f_read(&fil, gd3buffer, 512, &br);

    p = 0;
    i = 0;
    c[1] = '\0';
    for (p = 0; p < br; p += 2) {
      if (gd3buffer[p] == 0 && gd3buffer[p + 1] == 0) {
        i++;
        if (i == 10) break;
      } else {
        c[0] = gd3buffer[p];
        gd3[i].concat(c);
      }
    }

    /* GD3 の情報配列
      0 "Track name (in English characters)\0"
      1 "Track name (in Japanese characters)\0"
      2 "Game name (in English characters)\0"
      3 "Game name (in Japanese characters)\0"
      4 "System name (in English characters)\0"
      5 "System name (in Japanese characters)\0"
      6 "Name of Original Track Author (in English characters)\0"
      7 "Name of Original Track Author (in Japanese characters)\0"
      8 "Date of game's release written in the form yyyy/mm/dd, or just yyyy/mm
      or yyyy if month and day is not known\0" 9 "Name of person who converted
      it to a VGM file.\0" 10 "Notes\0"
    */

    gd3[0].concat(" / ");
    gd3[0].concat(gd3[2]);
    Display.set2(gd3[0]);  // 曲名表示＋ゲーム名

    gd3[4].concat(" / ");
    gd3[4].concat(gd3[8]);
    Display.set3(gd3[4]);  // システム名＋リリース日
  }

  // Data offset
  // v1.50未満は 0x40、v1.50以降は 0x34 からの相対位置
  uint32_t vgm_data_offset =
      (vgm_version >= 0x150) ? get_vgm_ui32_at(0x34) + 0x34 : 0x40;
  f_lseek(&fil, vgm_data_offset);

  // Clock
  uint32_t vgm_ay8910_clock = (vgm_version >= 0x151 && vgm_data_offset >= 0x78)
                                  ? get_vgm_ui32_at(0x74)
                                  : 0;
  if (vgm_ay8910_clock) {
    switch (vgm_ay8910_clock) {
      case 1250000:  // 1.25MHz
        SI5351.setFreq(SI5351_1250, 0);
        break;
      case 1500000:  // 1.5 MHz
        SI5351.setFreq(SI5351_3000, 0);
        break;
      case 1789772:
      case 1789773:
        SI5351.setFreq(SI5351_3579, 0);
        break;
      case 2000000:
        SI5351.setFreq(SI5351_4000, 0);
        break;
      default:
        SI5351.setFreq(SI5351_3579, 0);
        break;
    }
  }

  // SN76489
  /*  uint32_t vgm_sn76489_clock = get_vgm_ui32_at(0x0C);
    if (vgm_sn76489_clock) {
      switch (vgm_sn76489_clock) {
        case 1500000:  // 1.5 MHz
        case 1076741824: // dual 1.5 MHz -> 3MHz
          SI5351.setFreq(SI5351_3000, 1);
          break;
        case 1536000:  // 1.536 MHz
          SI5351.setFreq(SI5351_1536, 1);
          break;
        case 3579580:  // 3.579MHz
        case 3579545:
          SI5351.setFreq(SI5351_3579, 1);
          break;
        case 4000000:
          SI5351.setFreq(SI5351_4000, 1);
          break;
        default:
          SI5351.setFreq(SI5351_3579, 1);
          break;
      }
    }
  */
  uint32_t vgm_ym2203_clock = get_vgm_ui32_at(0x44);
  if (vgm_ym2203_clock) {
    switch (vgm_ym2203_clock) {
      case 1076741824:  // Dual 1.5MHz
      case 1075241824:  // Dual 1.5MHz
        SI5351.setFreq(SI5351_1500);
        break;
      case 3000000:  // 3MHz
        SI5351.setFreq(SI5351_3000);
        break;
      case 3072000:  // 3.072MHz
        SI5351.setFreq(SI5351_3072);
        break;
      case 3579580:  // 3.579MHz
      case 3579545:
        SI5351.setFreq(SI5351_3579);
        break;
      case 3993600:
      case 4000000:
      case 1077741824:  // Dual 4MHz
        SI5351.setFreq(SI5351_4000);
        break;
      case 4500000:     // 4.5MHz
      case 1078241824:  // Dual 4.5MHz
        SI5351.setFreq(SI5351_4500);
        break;
      case 1250000:  // 1.25MHz
        SI5351.setFreq(SI5351_1250);
        break;
      default:
        SI5351.setFreq(SI5351_3579);
        break;
    }
  }
  /*
    uint32_t vgm_ym2151_clock = get_vgm_ui32_at(0x30);
    if (vgm_ym2151_clock) {
      switch (vgm_ym2151_clock) {
        case 3579580:
        case 3579545:
          SI5351.setFreq(SI5351_3579);
          break;
        case 4000000:
          SI5351.setFreq(SI5351_4000);
          break;
        case 3375000:
          SI5351.setFreq(SI5351_3375);
          break;
        default:
          SI5351.setFreq(SI5351_3579);
          break;
      }
    }
  /*
    uint32_t vgm_ym3812_clock = get_vgm_ui32_at(0x50);
    if (vgm_ym3812_clock) {
      switch (vgm_ym3812_clock) {
        case 3000000:  // 3MHz
          SI5351.setFreq(SI5351_3000);
          break;
        case 3500000:  // 3.5MHz
          SI5351.setFreq(SI5351_3500);
          break;
        case 3072000:  // 3.072MHz
          SI5351.setFreq(SI5351_3072);
          break;
        case 3579580:  // 3.579MHz
        case 3579545:
          SI5351.setFreq(SI5351_3579);
          break;
        case 4000000:
          SI5351.setFreq(SI5351_4000);
          break;
        case 4500000:  // 4.5MHz
          SI5351.setFreq(SI5351_4500);
          break;
        default:
          SI5351.setFreq(SI5351_3579);
          break;
      }
    }
  */

  // 初期バッファ補充
  fr = f_read(&fil, vgmBuffer1, BUFFERCAPACITY, &bufferSize);
  bufferPos = 0;
  vgmLoaded = true;
  compensation = 0;
}

void vgmProcess() {
  while (vgmLoaded) {
    if (PT2257.process_fadeout()) {  // フェードアウト完了なら次の曲
      if (numFiles[currentDir] - 1 == currentFile)
        openDirectory(1);
      else
        vgmPlay(1);
    }

    uint8_t reg;
    uint8_t dat;
    startTime = get_timer_value();
    byte command = get_vgm_ui8();

    switch (command) {
      case 0xA0:  // AY8910, YM2203 PSG, YM2149, YMZ294D
        dat = get_vgm_ui8();
        reg = get_vgm_ui8();
        FM.set_register(dat, reg, CS1);
        vgmDelay -= 1;
        break;
      case 0x30:  // SN76489 CHIP 2
        //FM.write(get_vgm_ui8(), CS2);
        break;
      case 0x50:  // SN76489 CHIP 1
        //FM.write(get_vgm_ui8(), CS0);
        break;
      case 0x54:  // YM2151
      case 0xa4:
        reg = get_vgm_ui8();
        dat = get_vgm_ui8();
        //FM.set_register(reg, dat, CS0);
        //vgmDelay -= 1;
        break;
      case 0x55:  // YM2203_0
        reg = get_vgm_ui8();
        dat = get_vgm_ui8();
        FM.set_register(reg, dat, CS0);
        vgmDelay -= 1;
        break;
      case 0xA5:  // YM2203_1
        reg = get_vgm_ui8();
        dat = get_vgm_ui8();
        FM.set_register(reg, dat, CS1);
        vgmDelay -= 1;
        break;
      case 0x5A:  // YM3812
        reg = get_vgm_ui8();
        dat = get_vgm_ui8();
        //FM.set_register(reg, dat, CS0);
        //vgmDelay -= 1;
        break;

      // Wait n samples, n can range from 0 to 65535 (approx 1.49 seconds)
      case 0x61:
        vgmDelay += get_vgm_ui16();
        break;

      // wait 735 samples (60th of a second)
      case 0x62:
        vgmDelay += 735;
        break;

      // wait 882 samples (50th of a second)
      case 0x63:
        vgmDelay += 882;
        break;
      case 0x66:
        if (!vgmLoopOffset) {  // ループしない曲

          if (numFiles[currentDir] - 1 == currentFile)
            openDirectory(1);
          else
            vgmPlay(1);
        } else {
          vgmLoop++;
          if (vgmLoop == MAXLOOP) {  // 既定ループ数ならフェードアウトON
            PT2257.start_fadeout();
          }
          f_lseek(&fil, vgmLoopOffset + 0x1C);  // ループする曲
          bufferPos = 0;  // ループ開始位置からバッファを読む
          f_read(&fil, vgmBuffer1, BUFFERCAPACITY, &bufferSize);
        }
        break;
      case 0x70:
      case 0x71:
      case 0x72:
      case 0x73:
      case 0x74:
      case 0x75:
      case 0x76:
      case 0x77:
      case 0x78:
      case 0x79:
      case 0x7A:
      case 0x7B:
      case 0x7C:
      case 0x7D:
      case 0x7E:
      case 0x7F:
        vgmDelay += (command & 15) + 1;
        break;
      default:
        break;
    }

    if (vgmDelay > 0) {
      bool flag = false;
      while ((get_timer_value() - startTime) <= vgmDelay * ONE_CYCLE) {

        if (vgmDelay>=compensation) {
          vgmDelay -= compensation;
          compensation = 0;
        } else {
          compensation -= vgmDelay;
          vgmDelay = 0;
          break;
        }

        if (flag == false && vgmDelay > 3) {
          flag = true;
          // handle key input
          switch (Keypad.checkButton()) {
            case Keypad.btnSELECT:  // ◯－－－－
              openDirectory(1);
              break;
            case Keypad.btnLEFT:  // －◯－－－
              openDirectory(-1);
              break;
            case Keypad.btnDOWN:  // －－◯－－
              vgmPlay(+1);
              break;
            case Keypad.btnUP:  // －－－◯－
              vgmPlay(-1);
              break;
            case Keypad.btnRIGHT:  // －－－－◯
              PT2257.start_fadeout();
              break;
          }
          // LCD の長い曲名をスクロールする
          // タイミングずれるので無効
          //if (Display.update()) { // LCDの文字表示更新
          //  compensation += 2000;
          //}
        }
      }

      vgmDelay = 0;
    }
    
  }
}

//----------------------------------------------------------------------
// ディレクトリ番号＋ファイル番号で VGM を開く
void vgmOpen(int d, int f) {
  char st[64];

  PT2257.mute();
  sprintf(st, "%s/%s", dirs[d], files[d][f]);
  if (openFile(st)) {
    vgmReady();

    // Silent YM2203
    FM.write(0x08, CS0);
    FM.write(0x00, CS0);
    FM.write(0x09, CS0);
    FM.write(0x00, CS0);
    FM.write(0x0A, CS0);
    FM.write(0x00, CS0);

    FM.write(0x08, CS1);
    FM.write(0x00, CS1);
    FM.write(0x09, CS1);
    FM.write(0x00, CS1);
    FM.write(0x0A, CS1);
    FM.write(0x00, CS1);

    FM.write(0x40, CS0);
    FM.write(127, CS0);
    FM.write(0x41, CS0);
    FM.write(127, CS0);
    FM.write(0x42, CS0);
    FM.write(127, CS0);
    FM.write(0x48, CS0);
    FM.write(127, CS0);
    FM.write(0x49, CS0);
    FM.write(127, CS0);
    FM.write(0x4a, CS0);
    FM.write(127, CS0);
    FM.write(0x4c, CS0);
    FM.write(127, CS0);
    FM.write(0x4d, CS0);
    FM.write(127, CS0);
    FM.write(0x4e, CS0);
    FM.write(127, CS0);

    FM.write(0x40, CS1);
    FM.write(127, CS1);
    FM.write(0x41, CS1);
    FM.write(127, CS1);
    FM.write(0x42, CS1);
    FM.write(127, CS1);
    FM.write(0x48, CS1);
    FM.write(127, CS1);
    FM.write(0x49, CS1);
    FM.write(127, CS1);
    FM.write(0x4a, CS1);
    FM.write(127, CS1);
    FM.write(0x4c, CS1);
    FM.write(127, CS1);
    FM.write(0x4d, CS1);
    FM.write(127, CS1);
    FM.write(0x4e, CS1);
    FM.write(127, CS1);
    Tick.delay_ms(16);

    FM.reset();

    Tick.delay_ms(32);
    PT2257.reset(attenuations[d]);
  }
}

//----------------------------------------------------------------------
// ディレクトリ内の count 個あとの曲再生。マイナスは前の曲
void vgmPlay(int count) {
  currentFile = mod(currentFile + count, numFiles[currentDir]);
  vgmOpen(currentDir, currentFile);
}

//----------------------------------------------------------------------
// count
// 個あとのディレクトリを開いて最初のファイルを再生。マイナスは前のディレクトリ
void openDirectory(int count) {
  currentFile = 0;
  currentDir = mod(currentDir + count, numDirs);
  vgmOpen(currentDir, currentFile);
}

int mod(int i, int j) {
  return (i % j) < 0 ? (i % j) + 0 + (j < 0 ? -j : j) : (i % j + 0);
}