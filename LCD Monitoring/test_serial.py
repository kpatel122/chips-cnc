"""
Serial test script for CNC monitor display.
Sends a series of test strings to the Arduino over USB serial and confirms
each was transmitted. Requires: pip install pyserial
"""

import time
import serial

# --------------------------------------------------------------------------
# Configuration
# --------------------------------------------------------------------------
SERIAL_PORT      = "/dev/ttyUSB0"
BAUD_RATE        = 115200
CONNECT_DELAY_S  = 2      # time to wait after opening port (Arduino resets on connect)
MESSAGE_DELAY_S  = 3      # time between messages to allow display to update
LINE_TERMINATOR  = "\r\n"

# --------------------------------------------------------------------------
# Test strings
# --------------------------------------------------------------------------
TEST_MESSAGES = [
    "Hello World",
    "Short",
    "CNC Machine",
    "X:100 Y:200 Z:50",
    "Feed rate: 1500",
    "Spindle: ON",
    "Status: IDLE",
    "A longer string to test wrapping behaviour on the display",
]

# --------------------------------------------------------------------------

def send_message(port: serial.Serial, msg: str) -> None:
    payload = (msg + LINE_TERMINATOR).encode("utf-8")
    port.write(payload)
    port.flush()
    print(f"  sent: {msg!r}")


def run_tests(port: serial.Serial) -> None:
    print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud")
    print(f"Waiting {CONNECT_DELAY_S}s for Arduino to initialise...\n")
    time.sleep(CONNECT_DELAY_S)

    for i, msg in enumerate(TEST_MESSAGES, start=1):
        print(f"[{i}/{len(TEST_MESSAGES)}]", end=" ")
        send_message(port, msg)
        time.sleep(MESSAGE_DELAY_S)

    print("\nAll messages sent.")


def main() -> None:
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as port:
            run_tests(port)
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\nAborted.")


if __name__ == "__main__":
    main()
