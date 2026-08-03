#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

/* --------------------------------------------------------------------------
 * Wiring
 *   VCC -> 5V
 *   GND -> GND
 *   DAT -> PIN_TEMP_DAT  (with 4.7k pull-up resistor to VCC)
 * -------------------------------------------------------------------------- */
#define PIN_TEMP_DAT             2

#define TEMP_READ_INTERVAL_MS 2000    /* how often to take a reading */
#define TEMP_CONVERSION_MS     750    /* DS18B20 conversion time at 12-bit resolution */

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */
void  temp_sensor_init(void);
bool  temp_sensor_update(void);       /* call each loop; returns true on new reading */
float temp_sensor_get_celsius(void);  /* returns NAN if sensor is disconnected */

#endif /* TEMP_SENSOR_H */
