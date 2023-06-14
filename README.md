# 灌水制御用 M304 プログラム

Version 1.3.0

## インストール方法

### 必要なライブラリ集

Arduinoライブラリ管理から以下のライブラリを準備します。

|ライブラリ名  |Version  |ディレクトリ名           |
|--------------|---------|-------------------------|
|M304 Library  |1.3.0    | libraries/M304-lib      |
|LiquidCrystal |1.0.7    | libraries/LiquidCrystal |
|SPI           |1.0 標準 | libraries/SPI           |
|Ethernet2     |1.0.4    | libraries/Ethernet2     |
|EEPROM        |2.0 標準 | libraries/EEPROM        |
|AT24Cx        |         | libraries/AT24Cx        |
|Wire          |1.0 標準 | libraries/Wire          |
|DS1307RTC     |1.4.1    | libraries/DS1307RTC     |
|Time          |1.6.1    | libraries/Time          |


### Arduino IDEの設定

* ボードをMEGAにする

### 書き込む前のM304設定

* リレーを接続している場合には、リレーの予期せぬMAKEを防ぐために[手動]にして、[遮断]側にスイッチを設定する。
* 有効なLANを接続する。
