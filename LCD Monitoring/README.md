# CNC Control Monitoring — Arduino Uno

Monitors water temperature and flow rate for a CNC water-cooled spindle. Displays readings on a 1.9" IPS LCD and reports over USB serial.

---

## Hardware

| Component | Model |
|---|---|
| Microcontroller | Arduino Uno (ATmega328P) |
| Display | 1.9" IPS LCD — ST7789 driver, 170×320px |
| Temperature sensor | DS18B20 TO-92 waterproof probe module |
| Flow sensor | YF-S201 G1/2" hall effect flow meter |

---

## Wiring

### Display — ST7789 (SPI)

| Module pin | Arduino pin | Notes |
|---|---|---|
| SCL | D13 | Hardware SPI clock (fixed) |
| SDA | D11 | Hardware SPI MOSI (fixed) |
| CS | D10 | |
| DC | D9 | |
| RST | D8 | |
| BLK | D7 | Backlight enable |
| VCC | 3.3V or 5V | Check module label |
| GND | GND | |

### Temperature Sensor — DS18B20

| Module pin | Arduino pin | Notes |
|---|---|---|
| VCC | 5V | |
| GND | GND | |
| DAT | D2 | Pull-up resistor built into module |

> No external resistor required — the breakout board has a 4.7kΩ pull-up on the DAT line.

### Flow Sensor — YF-S201

| Module pin | Arduino pin | Notes |
|---|---|---|
| VCC (red) | 5V | |
| GND (black) | GND | |
| Signal (yellow) | D3 | Interrupt pin (INT1) |

> The sensor has G1/2" BSP fittings. Use 8mm × G1/2" Female BSP adapters to connect to 8mm cooling hose.

---

## Pin Summary

| Arduino pin | Allocated to |
|---|---|
| D2 | DS18B20 DAT |
| D3 | YF-S201 signal |
| D7 | Display backlight |
| D8 | Display RST |
| D9 | Display DC |
| D10 | Display CS |
| D11 | SPI MOSI (display) |
| D13 | SPI SCK (display) |
| D0/D1 | USB serial (reserved) |
| D4–D6, D12 | Free |

---

## Software Setup

### Prerequisites

- [PlatformIO](https://platformio.org/) — VS Code extension or CLI
- Python 3 with `python3-serial` for the test script

```bash
sudo apt install python3-serial
```

### Build and Flash

```bash
# Build
pio run

# Flash to connected Arduino Uno
pio run --target upload
```

### Serial Monitor

Connect via PuTTY or any serial terminal:

| Setting | Value |
|---|---|
| Port | /dev/ttyUSB0 |
| Baud rate | 115200 |
| Local echo | Force on |

> In PuTTY: Terminal → Local echo → Force on

### Test Script

Sends a series of test strings over serial to verify the display pipeline:

```bash
python3 test_serial.py
```

Edit `SERIAL_PORT` at the top of the script if your device enumerates on a different port (e.g. `/dev/ttyACM0`).

---

## Display Layout

The display is landscape (320×170). Rows from top to bottom:

| Row | Content | Colour |
|---|---|---|
| 1 | Serial message / status | White |
| 2 | Water temperature (°C) | White |
| 3 | Flow rate (L/min) | Green = flowing, Red = no flow |

---

## Dependencies

Managed automatically by PlatformIO via `platformio.ini`:

| Library | Purpose |
|---|---|
| Adafruit ST7735 and ST7789 Library | Display driver |
| Adafruit GFX Library | Graphics primitives |
| OneWire | 1-Wire bus protocol |
| DallasTemperature | DS18B20 sensor driver |
