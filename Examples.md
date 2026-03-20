# Examples

This page details some software examples and wiring diagrams to use with the baord.

## Arduino

### Libraries

To use the baord with the Arduino framework, we use these libraries:

**NOTE: at present we need to use the MCQN forks of both of these libraries for it to work with the ST25R3916B chipset**

 * [NFC-RFAL](https://github.com/mcqn/NFC-RFAL)
 * [ST25R3916](https://github.com/mcqn/ST25R3916)

### Wiring

#### ESP32-S3 dev module

| Description | Pin on ESP32-S3 board | Pin on Vicino NFC board |
| --- | --- | --- |
| 3V3 power |  3V3    |  1-VDD   |
| IRQ       |  14     |  2-IRQ   |
| Ground    |  GND    |  3-GND   |
| LED       |  4      |  4-LED   |
| MISO      |  37     |  5-MISO  |
| MOSI      |  35     |  6-MOSI  |
| SCLK      |  36     |  7-SCLK  |
| SS        |  34     |  8-SS    |

#### ESP32-S2 Saola 1

| Description | Pin on Saola board | Pin on Vicino NFC board |
| --- | --- | --- |
| 3V3 power |  3V3    |  1-VDD   |
| IRQ       |  14     |  2-IRQ   |
| Ground    |  GND    |  3-GND   |
| LED       |  4      |  4-LED   |
| MISO      |  37     |  5-MISO  |
| MOSI      |  35     |  6-MOSI  |
| SCLK      |  36     |  7-SCLK  |
| SS        |  34     |  8-SS    |

#### ESP32 LyraT

| Description | Pin on LyraT board | Pin on Vicino NFC board |
| --- | --- | --- |
| 3V3 power |  Uart-3V3        |  1-VDD   |
| IRQ       |  I2C-SDA / 18    |  2-IRQ   |
| Ground    |  I2C-GND         |  3-GND   |
| LED       |  I2C-SCL / 23    |  4-LED   |
| MISO      |  JTAG-MTMS / 14  |  5-MISO  |
| MOSI      |  JTAG-MTCK / 13  |  6-MOSI  |
| SCLK      |  JTAG-MTDI / 12  |  7-SCLK  |
| SS        |  JTAG-MTDO / 15  |  8-SS    |

#### Mapping between Vicino and X-NUCLEO-NFC06A

The Vicino board is functionally equivalent to the ST [X-NUCLEO-NFC06A](https://www.st.com/en/ecosystems/x-nucleo-nfc06a1.html#documentation) dev board.  In case you need to translate any resources for that to work with the Vicino board the pin mappings between the two are below:

| Description | Pin on Vicino board | Pin on X-NUCLEO-NFC06A board |
| --- | --- | --- |
| 3V3 power |  1-VDD    |  CN6-4  |
| IRQ       |  2-IRQ    |  CN8-1  |
| Ground    |  3-GND    |  CN6-6  |
| LED       |  4-LED    |  CN8-2  |
| MISO      |  5-MISO   |  CN5-5  |
| MOSI      |  6-MOSI   |  CN5-4  |
| SCLK      |  7-SCLK   |  CN5-6  |
| SS        |  8-SS     |  CN5-3  |

