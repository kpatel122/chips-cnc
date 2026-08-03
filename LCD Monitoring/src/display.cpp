#include <Arduino.h>
#include <math.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "display.h"

static Adafruit_ST7789 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

static void clear_row(uint16_t y)
{
    tft.fillRect(0, y, tft.width(), DISPLAY_ROW_HEIGHT, COLOUR_BG);
}

static void set_cursor(uint16_t y)
{
    tft.setTextSize(DISPLAY_TEXT_SIZE);
    tft.setTextColor(COLOUR_TEXT);
    tft.setCursor(DISPLAY_TEXT_X, y);
}

void display_init(void)
{
    pinMode(PIN_TFT_BLK, OUTPUT);
    digitalWrite(PIN_TFT_BLK, HIGH);

    tft.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    tft.setRotation(DISPLAY_ROTATION);
    tft.fillScreen(COLOUR_BG);

    set_cursor(DISPLAY_ROW_MSG_Y);
    tft.print("Waiting...");

    set_cursor(DISPLAY_ROW_TEMP_Y);
    tft.print("Temp: --.- C");

    set_cursor(DISPLAY_ROW_FLOW_Y);
    tft.print("Flow: --.- L/m");
}

void display_show_message(const char *msg)
{
    clear_row(DISPLAY_ROW_MSG_Y);
    set_cursor(DISPLAY_ROW_MSG_Y);
    tft.print(msg);
}

void display_show_temp(float celsius)
{
    clear_row(DISPLAY_ROW_TEMP_Y);
    set_cursor(DISPLAY_ROW_TEMP_Y);

    if (isnan(celsius))
    {
        tft.print("Temp: ERR");
        return;
    }

    tft.print("Temp: ");
    tft.print(celsius, 1);
    tft.print(" C");
}

void display_show_flow(float lpm, bool flow_ok)
{
    clear_row(DISPLAY_ROW_FLOW_Y);

    /* Colour-code by flow status: green = flowing, red = no flow */
    tft.setTextSize(DISPLAY_TEXT_SIZE);
    tft.setTextColor(flow_ok ? COLOUR_GREEN : COLOUR_RED);
    tft.setCursor(DISPLAY_TEXT_X, DISPLAY_ROW_FLOW_Y);

    if (flow_ok)
    {
        tft.print("Flow: ");
        tft.print(lpm, 1);
        tft.print(" L/m");
    }
    else
    {
        tft.print("Flow: NO FLOW");
    }
}
