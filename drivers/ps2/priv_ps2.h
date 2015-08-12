#ifndef _PRIV_PS2_CTRL_H_
#define _PRIV_PS2_CTRL_H_

#include "types.h"

#define PS2_CTRL_PORT 0x64
#define PS2_DATA_PORT 0x60

enum {
        PS2_RETVAL_BUFFER_OVERRUN = 0x00,
        PS2_RETVAL_ECHO = 0xEE,
        PS2_RETVAL_ACK = 0xFA,
        PS2_RETVAL_BAT_FAIL = 0xFC,
        PS2_RETVAL_BAT_SUCCESS = 0xAA,
        PS2_RETVAL_RESEND_REQ = 0xFE,
        PS2_RETVAL_KEY_ERR = 0xFF
} PS2_RETVAL;

typedef enum {
        PS2_READ_CONFIG = 0x20,
        PS2_WRITE_CONFIG = 0x60,
        PS2_CTRL_TEST = 0xAA,
        PS2_P1_DISABLE = 0xAD,
        PS2_P1_ENABLE = 0xAE,
        PS2_P1_TEST = 0xAB,
        PS2_P2_DISABLE = 0xA7,
        PS2_P2_ENABLE = 0xA8,
        PS2_P2_TEST = 0xA9
}PS2_CMD;


uint8_t PS2_GetStatus ();
#define IS_OUTPUTBUF_FULL (PS2_GetStatus() & 1)
#define IS_INPUTBUF_FULL ((PS2_GetStatus() >> 1) & 1)
#define IS_SELFTEST_PASS ((PS2_GetStatus() >> 2) & 1)
#define CTRL_CMD ((PS2_GetStatus() >> 3) & 1)
#define TIMED_OUT ((PS2_GetStatus() >> 6) & 1)
#define IS_PARITY_ERROR ((PS2_GetStatus() >> 7) & 1)

uint8_t PS2_SendCommand(uint16_t port, uint8_t val, uint8_t *response);
uint8_t PS2_ReadData(uint16_t port);

void PS2_SetEnabled(uint8_t deviceIndex, uint8_t status);
uint8_t PS2_TestDevice(uint8_t deviceIndex);
uint8_t PS2_SetControllerConfig(uint8_t P1_IRQ, uint8_t P2_IRQ, uint8_t translation);
uint8_t PS2_IsDualChannel();

#endif /* end of include guard: _PRIV_PS2_CTRL_H_ */
