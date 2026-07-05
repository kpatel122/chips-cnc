# Mesa 7i96S TB3 Isolated Outputs

## Output Types

### OUT0-OUT3 (High Current DC MOSFET)
- 2A max, 0-36VDC, polarised, low side switch
- Use for: switching DC control circuits e.g. contactor coils
- Fuse: 1A slow blow (or 1A DIN MCB)

### OUT4-OUT5 (AC/DC SSR, High Isolation)
- 100mA max, 0-24VAC / 0-36VDC, non-polarised, 300V isolated
- Use for: VFDs, plasma torches, electrically noisy devices
- Fuse: 250mA slow blow glass fuse

---

## Application 1: VFD Enable Signal (DI1)

Uses OUT4 or OUT5 — high isolation SSR output.

```
VFD pin 1 (+24V internal) → OUT4A → OUT4B → VFD pin 2 (DI1)
```

- Non-polarised — either way around is fine
- VFD's internal 24V is self-contained, no connection to external PSU
- LinuxCNC HAL: drop output to disable VFD on e-stop
- VFD will ramp to stop per P-04 deceleration time (P-05 = 0)

---

## Application 2: Switching AC Devices via Contactor

Uses OUT0-OUT3 — high current MOSFET output (low side switch).

The Mesa output controls a 24VDC contactor coil, whose contacts switch the AC supply.

```
PSU 24V+ → Fuse → Contactor coil A1 → Coil A2 → Mesa OUT0+ → Mesa OUT0- → PSU GND
```

AC switching circuit (through contactor contacts):
```
AC Mains → MCB → Contactor contacts → AC load (e.g. stepper driver transformer)
```

- Contactor: Siemens LOGO! Contact 24 (6ED1057-4CA00-0AA0), 24VDC coil, 170mA hold / 605mA pull-in, 20A contacts
- Always cut AC on the supply side of the transformer — never cut DC side of stepper drivers (risks driver damage)
- LinuxCNC HAL: de-energise coil to cut AC supply on e-stop
- Fail-safe: coil de-energises automatically on Mesa/PC power loss

---

## Application 3a: Switching AC Devices via External SSR

Uses OUT4 or OUT5 — drives an external AC SSR directly.

```
PSU 24V+ → SSR control input + → SSR control input - → Mesa OUT4A → Mesa OUT4B → PSU GND
```

AC switching circuit (through SSR):
```
AC Mains Live → SSR AC input → SSR AC output → AC load (e.g. pump socket Live)
AC Mains Neutral → AC load Neutral (unswitched)
```

- SSR control input typically 3-32VDC — compatible with OUT4/OUT5
- Choose SSR rated for load current plus headroom (e.g. 10A SSR for a pump)
- SSRs generate heat at higher currents — use a heatsink above ~3A
- No flyback diode needed (no coil)
- Silent, no moving parts — good for frequently switched loads

---

## Application 3b: Switching AC Devices via Mechanical Relay

Uses OUT0-OUT3 — drives a 24VDC relay coil, relay contacts switch AC load.

```
PSU 24V+ → Fuse → Relay coil + → Relay coil - → Mesa OUT0+ → Mesa OUT0- → PSU GND
```

AC switching circuit (through relay contacts):
```
AC Mains Live → Relay COM → Relay NO → AC load (e.g. pump socket Live)
AC Mains Neutral → AC load Neutral (unswitched)
```

- Use a relay with a 24VDC coil rated within 2A draw
- **Flyback diode required** across the relay coil — see section below
- Good for occasional switching — audible click, moving parts wear over time

---

## Flyback Diode Explained

A relay coil is an inductor — when current flows through it, it builds up a magnetic field. When the Mesa output switches off and current stops suddenly, that magnetic field collapses and the coil tries to keep current flowing by generating a voltage spike. This spike can be many times higher than the supply voltage and will damage or destroy the Mesa MOSFET output.

A flyback diode gives that spike a safe path to circulate and dissipate harmlessly instead of going through the Mesa output.

**Wiring:**
```
PSU 24V+ ←──[diode]──── Coil + (A1)
                              |
                         Coil - (A2)
                              |
                         Mesa OUT0+
                              |
                         Mesa OUT0-
                              |
                         PSU GND
```

The diode is wired in reverse across the coil — it blocks current during normal operation but conducts the spike when the coil collapses.

- **Diode orientation**: anode to coil - (A2/Mesa side), cathode to coil + (PSU 24V+ side)
- Any general purpose diode works — e.g. **1N4007** (very common, cheap, rated 1A/1000V)
- Mount as close to the coil terminals as possible
- Not needed for SSRs or contactors with built-in suppression (check datasheet)
