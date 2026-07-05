# Optidrive E3 Parameters — Water Cooled Spindle (220V / 8A / 24000 RPM / 400Hz)

## Unit: ODE-3-220105-1F42 (2.2kW, single phase 240V input)

---

## Standard Parameters

| Parameter | Description | Value | Notes |
|-----------|-------------|-------|-------|
| P-01 | Maximum Speed Limit | 24000 | RPM (because P-10 > 0) |
| P-02 | Minimum Speed Limit | 0 | RPM |
| P-03 | Acceleration Ramp Time | 8 | seconds |
| P-04 | Deceleration Ramp Time | 8 | seconds |
| P-05 | Stopping Mode | 0 | Ramp to stop |
| P-07 | Motor Rated Voltage | 220 | V |
| P-08 | Motor Rated Current | 8 | A (reseller recommendation, nameplate says 8.5A) |
| P-09 | Motor Rated Frequency | 400 | Hz |
| P-10 | Motor Rated Speed | 24000 | RPM — setting this switches all speed parameters to RPM display |
| P-11 | Low Frequency Torque Boost | 0 | Must be 0 — any boost causes overcurrent at low frequency |
| P-12 | Primary Command Source | 2 | Bi-directional keypad control |

---

## Advanced Parameters

To access advanced parameters, first set **P-14 = 201**, then navigate to the parameter.

| Parameter | Description | Value | Notes |
|-----------|-------------|-------|-------|
| P-51 | Motor Control Mode | 1 | V/F mode — critical, default (0) is vector mode which requires autotune and will not work correctly |

---

## Key Notes

- **P-10 = 24000** causes the drive to display and accept all speed-related parameters (P-01, P-02 etc.) in **RPM not Hz**
- **P-51 = 1** (V/F mode) is essential for this spindle. The default P-51 = 0 (vector mode) requires autotune data — without it the drive outputs nothing when P-11 = 0
- **P-11 must be 0** — torque boost causes severe overcurrent at low frequency on high-Hz spindle motors due to near-zero winding impedance below ~50 Hz
- **P-09 must be set before P-10** — changing P-09 resets P-10 to 0
- DI1 (control terminal pin 2) must always be held high as hardware enable — required even in keypad mode
