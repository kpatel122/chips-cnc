#ifndef DISPLAY_H
#define DISPLAY_H

/* --------------------------------------------------------------------------
 * Pin assignments
 *
 * Hardware SPI pins (fixed by the ATmega328P):
 *   SCK  -> D13
 *   MOSI -> D11
 * -------------------------------------------------------------------------- */
#define PIN_TFT_CS   10
#define PIN_TFT_DC    9
#define PIN_TFT_RST   8
#define PIN_TFT_BLK   7

/* --------------------------------------------------------------------------
 * Display geometry
 * Physical panel is always 170 x 320; rotation remaps the logical axes.
 * Rotation: 0=portrait, 1=landscape, 2=portrait flipped, 3=landscape flipped
 * -------------------------------------------------------------------------- */
#define DISPLAY_WIDTH      170
#define DISPLAY_HEIGHT     320
#define DISPLAY_ROTATION     1

/* --------------------------------------------------------------------------
 * Layout — row Y positions in logical landscape coordinates (320 x 170)
 * -------------------------------------------------------------------------- */
#define DISPLAY_ROW_MSG_Y    10
#define DISPLAY_ROW_TEMP_Y   55
#define DISPLAY_ROW_FLOW_Y  100    /* reserved for flow sensor */

#define DISPLAY_ROW_HEIGHT   30    /* clear-rectangle height per row */
#define DISPLAY_TEXT_SIZE     2
#define DISPLAY_TEXT_X       10

/* --------------------------------------------------------------------------
 * Colours — RGB565 encoding: RRRRRGGGGGGBBBBB
 * -------------------------------------------------------------------------- */
#define COLOUR_BLACK    0x0000
#define COLOUR_WHITE    0xFFFF
#define COLOUR_RED      0xF800
#define COLOUR_GREEN    0x07E0
#define COLOUR_BLUE     0x001F
#define COLOUR_YELLOW   0xFFE0
#define COLOUR_CYAN     0x07FF
#define COLOUR_MAGENTA  0xF81F
#define COLOUR_ORANGE   0xFC00
#define COLOUR_GREY     0x8410

#define COLOUR_BG       COLOUR_BLACK
#define COLOUR_TEXT     COLOUR_WHITE

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */
void display_init(void);
void display_show_message(const char *msg);
void display_show_temp(float celsius);
void display_show_flow(float lpm, bool flow_ok);

#endif /* DISPLAY_H */
