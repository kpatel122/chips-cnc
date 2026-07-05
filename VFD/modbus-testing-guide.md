# Optidrive E3 — Modbus RTU Testing Guide

## Connection

| Setting | Value |
|---------|-------|
| Port | `/dev/ttyUSB0` (FT232R USB adapter) |
| Baud rate | 115200 |
| Format | 8N1 (8 data bits, no parity, 1 stop bit) |
| Slave address | 1 |
| Protocol | Modbus RTU |

**RJ45 wiring (front of drive):**
- Pin 3 → USB adapter GND
- Pin 7 → USB adapter B- (RS485-)
- Pin 8 → USB adapter A+ (RS485+)

**Install mbpoll:** `sudo apt install mbpoll`

**Add user to dialout group:** `sudo usermod -a -G dialout $USER` then `newgrp dialout`

---

## Before You Start

- Set **P-12 = 3** (Modbus Network Control) — drive ignores Modbus run/speed commands in keypad mode
- **DI1 (control terminal pin 2) must still be held high** — hardware enable is always required regardless of P-12
- Ensure cooling pump is running before spinning the spindle

---

## Watchdog — Important

The drive requires a valid Modbus message at least every **3 seconds** (P-36 Index 3 default = 3000ms). If communication stops while the spindle is running, the drive trips with fault **SC-F01**.

**Always run a continuous watchdog poll in a separate terminal before sending run commands:**

```bash
mbpoll -a 1 -b 115200 -P none -t 4 -r 6 /dev/ttyUSB0
```

This polls register 6 (drive status) every second — keeps the watchdog alive and monitors for faults simultaneously. Leave this running for the entire session.

---

## RPM Conversion

Register 2 (speed setpoint) uses Hz × 10:

`register value = RPM ÷ 24000 × 400 × 10`

| RPM | Register value |
|-----|---------------|
| 24000 | 4000 |
| 18000 | 3000 |
| 12000 | 2000 |
| 6000 | 1000 |
| 1000 | 167 |

---

## Control Commands

Always set speed before sending run command.

```bash
# Set speed (e.g. 12000 RPM = 2000)
mbpoll -a 1 -b 115200 -P none -t 4 -r 2 /dev/ttyUSB0 -- 2000

# Run
mbpoll -a 1 -b 115200 -P none -t 4 -r 1 /dev/ttyUSB0 -- 1

# Stop
mbpoll -a 1 -b 115200 -P none -t 4 -r 1 /dev/ttyUSB0 -- 0

# Fault reset
mbpoll -a 1 -b 115200 -P none -t 4 -r 1 /dev/ttyUSB0 -- 4
```

---

## Status Reads

Add `-1` flag to read once, omit for continuous polling.

```bash
# Drive status and fault code (register 6)
mbpoll -a 1 -b 115200 -P none -t 4 -r 6 -1 /dev/ttyUSB0

# Output frequency — Hz × 10, value × 6 = RPM (e.g. 167 = ~1000 RPM)
mbpoll -a 1 -b 115200 -P none -t 4 -r 7 -1 /dev/ttyUSB0

# Motor current — Amps × 10 (e.g. 10 = 1.0A)
mbpoll -a 1 -b 115200 -P none -t 4 -r 8 -1 /dev/ttyUSB0

# Speed setpoint (only non-zero when P-12 = 3 and speed has been written)
mbpoll -a 1 -b 115200 -P none -t 4 -r 2 -1 /dev/ttyUSB0

# Speed reference (shows current target regardless of control mode)
mbpoll -a 1 -b 115200 -P none -t 4 -r 22 -1 /dev/ttyUSB0

# DC bus voltage (~328V is normal from 240VAC supply)
mbpoll -a 1 -b 115200 -P none -t 4 -r 23 -1 /dev/ttyUSB0

# Heatsink temperature (°C)
mbpoll -a 1 -b 115200 -P none -t 4 -r 24 -1 /dev/ttyUSB0

# Digital input status — bit 0 = DI1 enable (1=high, 0=low)
mbpoll -a 1 -b 115200 -P none -t 4 -r 11 -1 /dev/ttyUSB0
```

---

## Decoding Register 6

Register 6 returns a 16-bit value encoding fault code and drive status.

**High byte = fault/error code** (see fault codes table in manual section 10)
**Low byte = drive status bits:**

| Bit | Meaning |
|-----|---------|
| 0 | Drive Running |
| 1 | Drive Tripped |
| 5 | Standby Mode |
| 6 | Drive Ready |

**Common values:**

| Value | Hex | Meaning |
|-------|-----|---------|
| 64 | 0x0040 | No fault, Drive Ready |
| 256 | 0x0100 | No fault, Drive Running |
| 12802 | 0x3202 | Fault SC-F01 (Modbus comms loss), Drive Tripped |
