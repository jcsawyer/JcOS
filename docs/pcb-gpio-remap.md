# GPIO Remap for Concurrent HD44780 LCD and SPI TFT

This PCB remap removes the GPIO conflict between the HD44780 character LCD and
the SPI TFT display so both can be enabled at the same time in the kernel.

## Final Pin Map

| Function | GPIO | 40-pin header pin |
| --- | --- | --- |
| SPI0 CE1 | GPIO 7 | Pin 26 |
| SPI0 CE0 | GPIO 8 | Pin 24 |
| SPI0 MISO | GPIO 9 | Pin 21 |
| SPI0 MOSI | GPIO 10 | Pin 19 |
| SPI0 SCLK | GPIO 11 | Pin 23 |
| TFT RESET | GPIO 24 | Pin 18 |
| TFT D/C | GPIO 25 | Pin 22 |
| Future touch SDA | GPIO 2 | Pin 3 |
| Future touch SCL | GPIO 3 | Pin 5 |
| Future touch RST | GPIO 22 | Pin 15 |
| Future touch IRQ | GPIO 23 | Pin 16 |
| HD44780 RS | GPIO 16 | Pin 36 |
| HD44780 E | GPIO 17 | Pin 11 |
| HD44780 D4 | GPIO 18 | Pin 12 |
| HD44780 D5 | GPIO 19 | Pin 35 |
| HD44780 D6 | GPIO 20 | Pin 38 |
| HD44780 D7 | GPIO 21 | Pin 40 |

## PCB Remap List

- GPIO 11 (Pin 23) to GPIO 16 (Pin 36)
- GPIO 10 (Pin 19) to GPIO 17 (Pin 11)
- GPIO 4 (Pin 7) to GPIO 18 (Pin 12)
- GPIO 5 (Pin 29) to GPIO 19 (Pin 35)
- GPIO 6 (Pin 31) to GPIO 20 (Pin 38)
- GPIO 7 (Pin 26) to GPIO 21 (Pin 40)

## TFT Module Pin Mapping

This section maps the TFT module pin numbers from the panel pinout sheet to the
Raspberry Pi 40-pin header.

| TFT module pin | Label | Raspberry Pi connection | Notes |
| --- | --- | --- | --- |
| 1 | VCC | 3.3V, Pin 1 or Pin 17 | Recommended 3.3V supply unless your exact module revision explicitly requires 5V |
| 2 | GND | GND, for example Pin 6 | Common ground |
| 3 | LCD_CS | GPIO 8, Pin 24 | SPI0 CE0 |
| 4 | LCD_RST | GPIO 24, Pin 18 | TFT reset |
| 5 | LCD_RS | GPIO 25, Pin 22 | TFT D/C, high = data, low = command |
| 6 | SDI (MOSI) | GPIO 10, Pin 19 | SPI0 MOSI |
| 7 | SCK | GPIO 11, Pin 23 | SPI0 SCLK |
| 8 | LED | Optional, tie to 3.3V or route to spare GPIO/PWM | Backlight control is not implemented in code today |
| 9 | SDO (MISO) | GPIO 9, Pin 21 | SPI0 MISO |
| 10 | CTP_SCL | GPIO 3, Pin 5 | I2C1 SCL, future touch only |
| 11 | CTP_RST | GPIO 22, Pin 15 | Reserved for future touch reset |
| 12 | CTP_SDA | GPIO 2, Pin 3 | I2C1 SDA, future touch only |
| 13 | CTP_INT | GPIO 23, Pin 16 | Reserved for future touch interrupt |
| 14 | SD_CS | Not connected | Leave unconnected unless you plan to use the module's SD slot |

### TFT Module Pins Used By Current Code

- `LCD_CS` to `GPIO 8 (Pin 24)`
- `LCD_RST` to `GPIO 24 (Pin 18)`
- `LCD_RS` to `GPIO 25 (Pin 22)`
- `SDI(MOSI)` to `GPIO 10 (Pin 19)`
- `SCK` to `GPIO 11 (Pin 23)`
- `SDO(MISO)` to `GPIO 9 (Pin 21)`

### TFT Module Pins Reserved For Future Touch

- `CTP_SDA` to `GPIO 2 (Pin 3)`
- `CTP_SCL` to `GPIO 3 (Pin 5)`
- `CTP_RST` to `GPIO 22 (Pin 15)`
- `CTP_INT` to `GPIO 23 (Pin 16)`

### TFT Module Pins Left Optional / Unconnected

- `LED`: optional backlight control or tie-on
- `SD_CS`: leave unconnected unless SD-card support is added

## Notes

- TFT SPI0 pins remain unchanged on GPIO 7-11.
- TFT control signals remain unchanged on GPIO 24 and GPIO 25.
- Lock touch to I2C1 on GPIO 2 and GPIO 3, with GPIO 22 for reset and GPIO 23 for interrupt.
- Leave GPIO 26 and GPIO 27 unassigned as spare expansion pins.
- Relevant unchanged TFT header pins are:
  - SPI0 CE1 on Pin 26
  - SPI0 CE0 on Pin 24
  - SPI0 MISO on Pin 21
  - SPI0 MOSI on Pin 19
  - SPI0 SCLK on Pin 23
  - TFT RESET on Pin 18
  - TFT D/C on Pin 22
