# Optidrive E3 — mb2hal Setup (LinuxCNC Host)

Goal: replace the manual `mbpoll` commands in `commands.txt` with a HAL component
(`mb2hal`) that runs continuously inside LinuxCNC, exposing VFD run/stop, speed,
and status as HAL pins. These pins then get netted to the existing spindle
signals in `printnc-test1.hal` (`spindle.0.on`, `spindle.0.speed-out-abs`, etc.)
so the Axis GUI's existing spindle controls drive the VFD over Modbus.

Do all of this on the LinuxCNC controller machine, not the dev machine.

Syntax below is pulled directly from the LinuxCNC 2.9 docs
(`linuxcnc.org/docs/2.9/html/drivers/mb2hal.html`), matching your 2.9.8 install.

---

## 0. Before you start

- Confirm the RS485 connection works exactly as tested with `mbpoll` in
  `modbus-testing-guide.md` (same port, same 115200/8N1, slave ID 1).
- Have that file open — the register map (1 = control word, 2 = speed, 6 =
  status) and RPM-to-register scaling (register = RPM ÷ 6) come from there.
- **DI1 (hardware enable) is wired separately to Mesa OUT5 and is NOT part of
  this Modbus setup.** Don't remove or bypass that wiring — it's the fail-safe
  e-stop path and must keep working independently of whatever mb2hal is doing.

---

## 1. mb2hal.ini syntax reference

Two section types:

- `[MB2HAL_INIT]` — one global block
- `[TRANSACTION_00]`, `[TRANSACTION_01]`, ... — one per Modbus register you
  want to read or write

**`[MB2HAL_INIT]` keys:**

| Key | Required | Notes |
|---|---|---|
| `TOTAL_TRANSACTIONS` | Yes | Count of `[TRANSACTION_NN]` blocks below |
| `SLOWDOWN` | No | Seconds delay between transactions in the scan loop; `0.0` = as fast as possible |
| `HAL_MODULE_NAME` | No | Defaults to `mb2hal` — this is the prefix in all pin names |
| `INIT_DEBUG` | No | 0–4, startup log verbosity |

**`[TRANSACTION_NN]` keys:**

| Key | Required | Notes |
|---|---|---|
| `LINK_TYPE` | Yes | `serial` for our RS485 link |
| `SERIAL_PORT` | Only on first serial transaction | e.g. `/dev/ttyUSB0` |
| `SERIAL_BAUD` | Only on first serial transaction | `115200` |
| `SERIAL_BITS` | Yes | `8` |
| `SERIAL_PARITY` | Yes | `none` |
| `SERIAL_STOP` | Yes | `1` |
| `MB_SLAVE_ID` | Yes | `1` |
| `FIRST_ELEMENT` | Yes | Starting register **address** — see addressing warning below |
| `NELEMENTS` | Yes (unless using `PIN_NAMES`) | `1` for a single register |
| `MB_TX_CODE` | Yes | function code string, see table below |
| `HAL_TX_NAME` | No | custom name used in the resulting pin, max 28 chars |
| `MAX_UPDATE_RATE` | No | Hz cap on how often this transaction runs; `0.0` = unlimited |

**Function codes we need:**

| Purpose | `MB_TX_CODE` |
|---|---|
| Write control word / speed (FC6) | `fnct_06_write_single_register` |
| Read status word (FC3) | `fnct_03_read_holding_registers` |

**Resulting HAL pin names:** `mb2hal.<HAL_TX_NAME or transaction number>.<element index>.<type>`,
type being `.int` or `.float` for register transactions. With `NELEMENTS = 1`
the element index is always `00`.

**✅ Addressing offset — confirmed correct on this install:**
The manual warns some Modbus masters use zero-based addressing and register
numbers may need `-1`. Confirmed empirically: with `FIRST_ELEMENT = 5`,
`vfd_status.00.int` read `64` (`0x0040`) when DI1 was enabled and `0` when
disabled — exactly the "No fault, Drive Ready" value documented in
`modbus-testing-guide.md` for register 6, and exactly matching the manual's
"bit 6 Drive Ready requires DI1 present" behavior. So `FIRST_ELEMENT =
register_number - 1` is correct:

| Manual register # | `mbpoll -r` | `FIRST_ELEMENT` |
|---|---|---|
| 1 (control word) | 1 | 0 |
| 2 (speed) | 2 | 1 |
| 6 (status) | 6 | 5 — confirmed live |

libmodbus applies this same zero-based convention uniformly across function
codes, so the two write transactions (registers 1 and 2) inherit the same
confirmed offset.

---

## 2. Draft `mb2hal.ini`

Create this next to `printnc-test1.hal`, e.g.
`/home/kp/Documents/Do_Backup/Projects/chips-cnc/LinuxCNC/mb2hal.ini`:

```ini
[MB2HAL_INIT]
TOTAL_TRANSACTIONS = 3
SLOWDOWN = 0.0
HAL_MODULE_NAME = mb2hal

[TRANSACTION_00]
LINK_TYPE = serial
SERIAL_PORT = /dev/ttyUSB0
SERIAL_BAUD = 115200
SERIAL_BITS = 8
SERIAL_PARITY = none
SERIAL_STOP = 1
MB_SLAVE_ID = 1
FIRST_ELEMENT = 0
NELEMENTS = 1
MB_TX_CODE = fnct_06_write_single_register
HAL_TX_NAME = vfd_control_word
MAX_UPDATE_RATE = 5.0

[TRANSACTION_01]
LINK_TYPE = serial
MB_SLAVE_ID = 1
FIRST_ELEMENT = 1
NELEMENTS = 1
MB_TX_CODE = fnct_06_write_single_register
HAL_TX_NAME = vfd_speed
MAX_UPDATE_RATE = 5.0

[TRANSACTION_02]
LINK_TYPE = serial
MB_SLAVE_ID = 1
FIRST_ELEMENT = 5
NELEMENTS = 1
MB_TX_CODE = fnct_03_read_holding_registers
HAL_TX_NAME = vfd_status
MAX_UPDATE_RATE = 5.0
```

Note `SERIAL_PORT`/`SERIAL_BAUD` only appear once, on the first serial
transaction — the rest share that link.

`MAX_UPDATE_RATE = 5.0` (every ~200ms) keeps all three transactions well
under the VFD's 3-second Modbus watchdog (SC-F01 trip), same reasoning as the
manual `mbpoll` watchdog loop you were running before. Because mb2hal re-writes
the control word and speed registers every cycle (not just on change), those
two writes are *also* independently satisfying the watchdog, on top of the
status read.

---

## 3. Test mb2hal standalone — do NOT touch `printnc-test1.hal` yet

Same principle as validating with `mbpoll` before wiring anything: prove the
component works in isolation before touching the real config.

```bash
halrun -I
loadusr -W mb2hal config=/path/to/mb2hal.ini
halcmd show pin mb2hal
```

You should see (confirmed against a real load — write transactions and read
transactions name their pins differently):
```
mb2hal.vfd_control_word.00        (float IN  — fnct_06 write, no type suffix)
mb2hal.vfd_speed.00               (float IN  — fnct_06 write, no type suffix)
mb2hal.vfd_status.00.float        (fnct_03 read — both .float and .int exist)
mb2hal.vfd_status.00.int
mb2hal.<name>.num_errors          (u32 OUT, per transaction — watch this, see below)
```

**Do not run a competing `mbpoll` on the same port at the same time.** A
single RS485 adapter can't be shared by two Modbus masters talking
simultaneously — if you leave a manual `mbpoll` watchdog running in another
terminal while `mb2hal` is also polling, every transaction on both sides will
fail. Check nothing else has the port open before loading mb2hal:
```bash
fuser -v /dev/ttyUSB0
```
Kill anything shown there first. If `num_errors` on any pin is climbing every
cycle with the VFD powered on, this — or a wrong `SERIAL_PORT`/wiring fault —
is the thing to rule out. (On this install, `num_errors` stayed at 0 and
`vfd_status.00.int` correctly read `0`/`64` with DI1 disabled/enabled,
confirming both comms and the register addressing offset are good.)

**Confirm the write side: set RPM / start / stop.** With DI1/OUT5 enabled,
run this full sequence with halcmd:

```bash
# 1. Set speed (~1000 RPM)
halcmd setp mb2hal.vfd_speed.00 167

# 2. Start
halcmd setp mb2hal.vfd_control_word.00 1
halcmd getp mb2hal.vfd_status.00.int
# expect 65 (64 Ready + 1 Running) instead of 64. Watch the spindle
# physically spin up too. Confirmed live on this install: 0 (disabled) ->
# 64 (enabled, stopped) -> 65 (enabled, running) -> 64 (stopped again).

# 3. Stop
halcmd setp mb2hal.vfd_control_word.00 0
halcmd getp mb2hal.vfd_status.00.int
# expect bit 0 clears — back to 64 (Ready, not Running). Watch the spindle
# ramp down per P-04 deceleration time.

exit
```

Don't move to step 4 until set-RPM/start/stop all behave exactly as they did
under manual `mbpoll` control.

---

## 4. Wire it into `custom.hal` (not `printnc-test1.hal`)

Your `printnc-test1.ini` already loads `custom.hal` after the PNCconf-generated
file:

```ini
[HAL]
HALFILE = printnc-test1.hal
HALFILE = custom.hal
```

Put all mb2hal + Modbus wiring in `custom.hal` so a future PNCconf regeneration
doesn't wipe it out. Add (pin names confirmed against a real `mb2hal` load in
step 3 — write pins have no `.int`/`.float` suffix, only reads do):

```hal
# --- VFD Modbus bridge (mb2hal) ---
loadusr -W mb2hal config=/path/to/mb2hal.ini

# RPM -> Optidrive register scaling (register = RPM / 6, per modbus-testing-guide.md)
loadrt scale names=spindle-rpm-to-reg
addf spindle-rpm-to-reg servo-thread
setp spindle-rpm-to-reg.gain 0.16667

net spindle-vel-cmd-rpm-abs => spindle-rpm-to-reg.in
net spindle-reg-speed       <= spindle-rpm-to-reg.out
net spindle-reg-speed       => mb2hal.vfd_speed.00

# spindle.0.on is a bit signal; mb2hal's write pin is float — HAL does NOT
# auto-convert between types on a net, so bridge it through conv_bit_float
loadrt conv_bit_float count=1
addf conv-bit-float.0 servo-thread

net spindle-enable          => conv-bit-float.0.in
net spindle-enable-float    <= conv-bit-float.0.out
net spindle-enable-float    => mb2hal.vfd_control_word.00

# --- VFD hardware enable (DI1) — independent of Modbus, fails safe on e-stop ---
net machine-is-enabled      => hm2_7i96s.0.outm.00.out-05
```

`spindle-vel-cmd-rpm-abs`, `spindle-enable`, and `machine-is-enabled` already
exist as signals in `printnc-test1.hal` — you're adding new destinations for
them, not redefining them. `machine-is-enabled` and `hm2_7i96s.0.outm.00.out-05`
are both bit type already, so that net doesn't need conversion — only
`spindle-enable` → mb2hal's float pin does.

`conv-bit-float.0.out` gives 0.0/1.0, matching control word values you
already validated by hand (bit 0 = run enable). Fault-reset (control word
value 4, bit 2) isn't wired here; add a separate momentary HAL bit + pyvcp
button later if you want that exposed too. Direction (FWD/REV) isn't in the
Optidrive's control word at all per the manual's register map — this spindle
only supports single-direction run via Modbus.

---

## 5. Full end-to-end test

1. Start LinuxCNC normally with this config.
2. `halcmd show pin mb2hal` — confirm it loaded and `vfd_status` is updating
   continuously (this is your watchdog — if it stops polling for >3s the
   drive will trip SC-F01).
3. In Axis, home all axes first — MDI (including M3/M5) won't execute on
   this config until the machine is homed. Then use MDI:
   ```
   M3 S1000
   ```
   Confirm the VFD ramps up to ~1000 RPM.
   ```
   M5
   ```
   Confirm it ramps down and stops.

   While a command is active, open a **separate terminal** (don't use
   `halrun` — LinuxCNC already owns the running HAL instance, so plain
   `halcmd` connects straight to it) and check the signals are actually
   propagating, not just coincidentally working:
   ```bash
   halcmd show pin mb2hal              # num_errors flat, vfd_status = 65 while running, 64 when stopped
   halcmd show pin spindle-rpm-to-reg   # .in = commanded RPM, .out = RPM / 6
   halcmd show sig spindle-enable       # toggles 1/0 with M3/M5
   ```
4. Try the Axis manual spindle controls (forward/stop buttons) directly.
5. Trigger e-stop and confirm the VFD drops out (DI1 / OUT5 path) regardless
   of what mb2hal's control word pin says — this is the fail-safe check, don't
   skip it.
6. Leave it running for several minutes at a stop/idle state and confirm no
   SC-F01 trips occur (proves the continuous poll is satisfying the watchdog
   even when not actively commanding speed changes).

---

## 6. Rollback

If anything misbehaves, comment out the mb2hal/scale/net block added in step 4
from `custom.hal` — `printnc-test1.hal` is untouched, so the machine's axes,
homing, jogging, and e-stop remain fully functional without spindle control.
