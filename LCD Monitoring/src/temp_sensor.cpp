#include <Arduino.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "temp_sensor.h"

static OneWire          one_wire(PIN_TEMP_DAT);
static DallasTemperature sensors(&one_wire);

static float    last_temp_c         = NAN;
static bool     conversion_pending  = false;
static uint32_t last_request_ms     = 0;
static uint32_t conversion_start_ms = 0;

void temp_sensor_init(void)
{
    sensors.begin();
    sensors.setWaitForConversion(false);    /* non-blocking mode */
}

bool temp_sensor_update(void)
{
    uint32_t now = (uint32_t)millis();

    if (!conversion_pending)
    {
        if ((now - last_request_ms) >= TEMP_READ_INTERVAL_MS)
        {
            sensors.requestTemperatures();
            conversion_start_ms = now;
            conversion_pending  = true;
        }
        return false;
    }

    if ((now - conversion_start_ms) >= TEMP_CONVERSION_MS)
    {
        float t = sensors.getTempCByIndex(0);
        last_temp_c        = (t == DEVICE_DISCONNECTED_C) ? NAN : t;
        conversion_pending = false;
        last_request_ms    = now;
        return true;
    }

    return false;
}

float temp_sensor_get_celsius(void)
{
    return last_temp_c;
}
