#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

/* --------------------------------------------------------------------------
 * Wiring
 *   VCC    -> 5V
 *   GND    -> GND
 *   Signal -> PIN_FLOW_SIGNAL  (interrupt-capable pin)
 * -------------------------------------------------------------------------- */
#define PIN_FLOW_SIGNAL          3     /* D3 — INT1 on ATmega328P */

/* YF-S201 datasheet: F (Hz) = FLOW_CALIBRATION_FACTOR * Q (L/min)
 * Update this value if a different sensor is substituted. */
#define FLOW_CALIBRATION_FACTOR  7.5f

/* How often to calculate flow rate */
#define FLOW_CALC_INTERVAL_MS    1000

/* Below this threshold the flow is considered absent */
#define FLOW_MIN_LMIN            0.5f

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */
void  flow_sensor_init(void);
bool  flow_sensor_update(void);        /* call each loop; returns true on new reading */
float flow_sensor_get_lpm(void);       /* latest flow rate in L/min */
bool  flow_sensor_is_flowing(void);    /* true if flow is above FLOW_MIN_LMIN */

#endif /* FLOW_SENSOR_H */
