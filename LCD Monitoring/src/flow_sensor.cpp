#include <Arduino.h>
#include "flow_sensor.h"

static volatile uint16_t pulse_count  = 0;
static float             last_flow_lpm = 0.0f;
static uint32_t          last_calc_ms  = 0;

static void flow_pulse_isr(void)
{
    pulse_count++;
}

void flow_sensor_init(void)
{
    pinMode(PIN_FLOW_SIGNAL, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_FLOW_SIGNAL), flow_pulse_isr, RISING);
}

bool flow_sensor_update(void)
{
    uint32_t now = (uint32_t)millis();

    if ((now - last_calc_ms) < FLOW_CALC_INTERVAL_MS)
    {
        return false;
    }

    /* Snapshot and reset the pulse counter atomically */
    noInterrupts();
    uint16_t count = pulse_count;
    pulse_count    = 0;
    interrupts();

    float interval_s = (float)FLOW_CALC_INTERVAL_MS / 1000.0f;
    last_flow_lpm = (float)count / (FLOW_CALIBRATION_FACTOR * interval_s);
    last_calc_ms  = now;

    return true;
}

float flow_sensor_get_lpm(void)
{
    return last_flow_lpm;
}

bool flow_sensor_is_flowing(void)
{
    return last_flow_lpm >= FLOW_MIN_LMIN;
}
