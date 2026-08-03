#include <Arduino.h>
#include "display.h"
#include "serial_comms.h"
#include "temp_sensor.h"
#include "flow_sensor.h"

void setup(void)
{
    serial_comms_init();
    display_init();
    temp_sensor_init();
    flow_sensor_init();
}

void loop(void)
{
    const char *msg = serial_comms_poll();
    if (msg != NULL)
    {
        display_show_message(msg);
        serial_comms_clear_terminal();
    }

    if (temp_sensor_update())
    {
        display_show_temp(temp_sensor_get_celsius());
    }

    if (flow_sensor_update())
    {
        display_show_flow(flow_sensor_get_lpm(), flow_sensor_is_flowing());
    }
}
