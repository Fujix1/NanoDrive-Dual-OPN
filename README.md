# VGM Player powered by Longan Nano で動く VGM プレーヤー (デュアル YM2203 版)

## これはなに? / What's this?

何かと話題の RISC-V マイコン Longan Nano (GD32V) を使って VGM ファイルを再生させる試みです。SD カード内のフォルダに保存した vgm 拡張子のファイルを順番に再生します。可能な限り表面実装部品を使ってコンパクトにまとめることを目標にしています。<br>
このバージョンは YM2203 を 2 個搭載しているので、PC8801 や普通のアーケードタイトルだけでなく、オールドカプコンや YM2203 + PSG を使うタイトル、さらに MSX も単体で鳴らすことができる世界で唯一のアレです。<br>
デジタルボリュームを搭載しているので、フェードアウトや音量のノーマライゼーションができます。
<br>
<br>
This is a vgm player working with a Longan Nano RISC-V microcomputer. This version supports dual YM2203 OPN chips which is capable to play old Campcom arcades (1943, GnG, Commando etc.), MSX, PC-8801 and PC-9801 musics. VGM data is stored in a SD card.<br>
<br>
**DIP Version**<br>
<img src="https://user-images.githubusercontent.com/13434151/195284256-cfec6f33-9b92-4837-a669-27ec8e1c7c0f.jpg" width="800"><br>
<br>
<br>
**SOP Version**<br>
<img src="https://user-images.githubusercontent.com/13434151/120786795-9b240880-c569-11eb-9b5f-49e75440f9e1.jpg" width="800"><br>
<br>

## コンパイルとマイコンへの書き込み / Compile and Upload to Longan Nano

Visual Studio Code + PlatformIO IDE を使用します。具体的な使用法については以下を参照してください。<br>
Use Visual Studio Code and Platform IO IDE.

- https://beta-notes.way-nifty.com/blog/2019/12/post-b8455c.html<br>
- http://kyoro205.blog.fc2.com/blog-entry-666.html
  <br>
  <br>

## 配線図 / Schematics

**【注】音量の増幅率を決める R6 と R7 が音量小さめの 20kΩ になっていますが、通常は 10kΩ 以下（ヘッドホンによっては 4.7kΩ くらい）をおすすめします。20kΩ はアーケードゲーム（カプコンなど）の音割れを防ぐための設定ですが、現在はフォルダ単位の音量調整ができるので音割れを防ぐことができます。**

** Note: R6 and R7 are 20kohm but it was originally to avoid clipping in some arcade titles. Use 4.7k - 10k ohm for normal use.**


![schematic](https://user-images.githubusercontent.com/13434151/195317404-62c582b9-1e1b-45d7-914e-4e40b31b5efd.png)
<a href="https://github.com/Fujix1/NanoDrive-Dual-OPN/files/9763906/NanoDriverYM2203.pdf">PDF ダウンロード</a>

## 使用部品の説明 / Parts

- Longan Nano: マイコン。128KB フラッシュメモリ版（64KB 版では足りません）。IC ソケットに入るように細いピンヘッダで実装のこと。 / 128KB version. Use thin pin headers.
- AE-Si5351A: I2C 制御の可変周波数発信 IC を使った秋月電子で売られているモジュール。 / I2C controlled variable frequency generator. Available at Akiduki Denshi.
- YM2203C: FM 音源チップ。アリエクなどで入手可能。2021 年頃から値上がり傾向。 / FM + PSG sound generator.
- Y3014(B): DAC チップ。アリエクなどで入手可能。Y3014 と Y3014B の 2 種類があるが大きな違いはない。 / DAC chip. No bid difference between Y3014 and Y3014B.
- オペアンプ / OPAMP: 4 個入り SOP14、速めのものがオススメ。 / Use fast one. SOP14.
- ミキシング用オペアンプ / Mixing OPAMP : なぜかみんな使ってる新日本無線 NJM3414。 / NJM3414 or its equivalent.
- PT2257: I2C 制御の音量調整 IC 表面実装 SOP8 版。5V 動作。アリエクなどで入手可能。 / I2C controlled digital volume controller. SOP8 version.
- 1000uF: 電源安定用 OS-CON。秋月で入手可能。 / OS-CON. Avaialble at Akiduki.
  <br>
  <br>

## VGM データの保存方法など

SD カードにディレクトリを作って、その中に VGM フォーマットファイルを保存します。ファイルには「.vgm」の拡張子が必要です。ZIP 圧縮された VGM ファイルである「\*.vgz」は認識しません。解凍して .vgm の拡張子をつけてください。不要なファイル、空のディレクトリは削除してください。

## フォルダ単位の音量減衰設定

YM2203 はアーケードとパソコンでは音量レベルが全然異なるため、このプログラムではフォルダ単位で音量低減率を指定できます。<br>
各フォルダに「att4」「att6」「att8」「att10」「att12」「att14」という名前の空ファイルを配置すると、それぞれ 4 ～ 14dB 音量を低減できます。例えば、PC-8801 版「YsII」とアーケード版「戦場の狼」を同じくらいの音量にしたいなら、戦場の狼のフォルダを att8 ～ att10 くらいにすればいい感じになります。

## ノイズについて

### SD カードのアクセスノイズ

SD カードにデータ読み込みをするときにブツブツとノイズが出ることがあります。これはアクセス時に大きく電圧降下が起こるのが原因で、SD カードの種類によってノイズが目立つものと目立たないものがあります。いろいろ試してノイズの少ないものを使うのがおすすめです。

### PC 電源のノイズ

PC から USB で電源供給を行い、さらに音声を PC に入力すると大きなグランドのループができて、ノイズが増幅されることがあります。電源は可能な限りクリーンなもの（モバイルバッテリーなど）を使ってください。
<br>
<br>

## 既知の問題点

- YM2203 は案外熱を持つ。
<br><br>
### 黒バージョン

<img src="https://user-images.githubusercontent.com/13434151/120786824-a1b28000-c569-11eb-9812-0c7c2944c75d.jpg" width="640">

