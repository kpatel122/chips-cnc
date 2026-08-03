#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

#define SERIAL_BAUD_RATE  115200
#define SERIAL_BUF_SIZE   64

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */
void        serial_comms_init(void);
const char *serial_comms_poll(void);         /* returns message string or NULL */
void        serial_comms_send(const char *msg);
void        serial_comms_clear_terminal(void);

#endif /* SERIAL_COMMS_H */
