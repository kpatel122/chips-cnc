#include <Arduino.h>
#include "serial_comms.h"

static char    serial_buf[SERIAL_BUF_SIZE];
static uint8_t serial_buf_len = 0;
static bool    in_escape_seq  = false;

/* --------------------------------------------------------------------------
 * unwrap
 *
 * Parses a raw message into a null-terminated string suitable for display.
 * Returns a pointer to the display string, or NULL if the message is invalid.
 *
 * Currently passes through unchanged.
 * TODO: replace with real protocol parsing once the wire format is defined.
 * -------------------------------------------------------------------------- */
static const char *unwrap(const char *raw, uint8_t len)
{
    (void)len;
    return raw;
}

/* --------------------------------------------------------------------------
 * serial_read
 *
 * Accumulates incoming bytes into serial_buf one call at a time.
 * Returns true when a message terminated by \r or \n is ready.
 * -------------------------------------------------------------------------- */
static bool serial_read(void)
{
    while (Serial.available())
    {
        char c = (char)Serial.read();

        /* Strip ANSI/VT100 escape sequences (ESC [ ... <letter>) */
        if (c == 0x1B)
        {
            in_escape_seq = true;
            continue;
        }
        if (in_escape_seq)
        {
            if (isalpha(c))
            {
                in_escape_seq = false;
            }
            continue;
        }

        /* \r or \n terminates a message; guard skips the trailing \n of \r\n */
        if (c == '\r' || c == '\n')
        {
            if (serial_buf_len == 0)
            {
                continue;
            }
            serial_buf[serial_buf_len] = '\0';
            return true;
        }

        /* Backspace (0x08) and DEL (0x7F) — pop last character */
        if (c == 0x08 || c == 0x7F)
        {
            if (serial_buf_len > 0)
            {
                serial_buf_len--;
            }
            continue;
        }

        /* Guard against overflow — drop bytes beyond buffer capacity */
        if (serial_buf_len < (SERIAL_BUF_SIZE - 1))
        {
            serial_buf[serial_buf_len++] = c;
        }
    }

    return false;
}

void serial_comms_init(void)
{
    Serial.begin(SERIAL_BAUD_RATE);
}

const char *serial_comms_poll(void)
{
    if (serial_read())
    {
        const char *msg = unwrap(serial_buf, serial_buf_len);
        serial_buf_len = 0;
        return msg;
    }
    return NULL;
}

void serial_comms_send(const char *msg)
{
    Serial.print(msg);
}

void serial_comms_clear_terminal(void)
{
    serial_comms_send("\x1B[2J\x1B[H");
}
