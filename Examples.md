# Examples

This page details some software examples and wiring diagrams to use with the baord.

## Arduino

### Libraries

To use the baord with the Arduino framework, we use these libraries:

 * [NFC-RFAL](https://github.com/stm32duino/NFC-RFAL) (due to a compilation clash, when building for ESP32 use [this fork instead](https://github.com/mcqn/NFC-RFAL)
 * [ST25R3916](https://github.com/stm32duino/ST25R3916)

### Wiring

NOTE: For now, the "Pin on NFC board" sections refer to connections on the [X-NUCLEO-NFC06A](https://www.st.com/en/ecosystems/x-nucleo-nfc06a1.html#documentation) board we're using during development (particularly `Figure 1` in the datasheet/data brief.

#### ESP32-S2 Saola 1

| Description | Pin on Saola board | Pin on NFC board |
| --- | --- | --- |
| SS        |  34     |  CN5-3  |
| MOSI      |  35     |  CN5-4  |
| SCLK      |  36     |  CN5-6  |
| MISO      |  37     |  CN5-5  |
| IRQ       |  14     |  CN8-1  |
| LED       |  4      |  CN8-2  |
| 3V3 power |  3V3    |  CN6-4  |
| Ground    |  GND    |  CN6-6  |

#### ESP32 LyraT

| Description | Pin on LyraT board | Pin on NFC board |
| --- | --- | --- |
| SS        |  JTAG-MTDO / 15  |  CN5-3  |
| MOSI      |  JTAG-MTCK / 13  |  CN5-4  |
| SCLK      |  JTAG-MTDI / 12  |  CN5-6  |
| MISO      |  JTAG-MTMS / 14  |  CN5-5  |
| IRQ       |  I2C-SDA / 18    |  CN8-1  |
| LED       |  I2C-SCL / 23    |  CN8-2  |
| 3V3 power |  Uart-3V3        |  CN6-4  |
| Ground    |  I2C-GND         |  CN6-6  |


