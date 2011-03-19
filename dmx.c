#include <avr/io.h>
#include <avr/interrupt.h>

#include "board.h"
#include "dmx.h"

dmx_state_t dmxState = DMX_IDLE;
static uint16_t dmxAddress = 0;
static uint16_t dmxSize = 0;
static uint8_t *dmxData = NULL;

void initDMX(uint16_t _dmxAddress, uint8_t *_dmxData, uint16_t _dmxSize) {
  UBRRH = ((F_CPU / (16 * DMX_BAUDRATE)) - 1) >> 8;
  UBRRL = ((F_CPU / (16 * DMX_BAUDRATE)) - 1) & 0xFF;
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) | _BV(USBS); /* 8n2 */
  UCSRB = _BV(RXEN) | _BV(RXCIE); /* enabled receive */
  dmxState = DMX_IDLE;
  setDMXAddress(_dmxAddress);
  setDMXData(_dmxData, _dmxSize);
}

void setDMXAddress(uint16_t _dmxAddress) {
  dmxAddress = _dmxAddress;
}

void setDMXData(uint8_t *_dmxData, uint16_t _dmxSize) {
  dmxData = _dmxData;
  dmxSize = _dmxSize;
}  

SIGNAL(UART_RX_vect) {
  static uint16_t dmxCount;
  uint8_t uartState = UCSRA;
  uint8_t dmxByte = UDR;
  
  /* check for framing error -> BREAK */
  if (IS_BIT_SET(uartState, FE)) {
    CLEAR_BIT(UCSRA, FE);
    dmxCount = dmxAddress;
    dmxState = DMX_BREAK;
  } else {
    switch (dmxState) {
    case DMX_BREAK:
      if (dmxByte == 0) {
        dmxState = DMX_START_BYTE;
      } else {
        dmxState = DMX_IDLE;
      }
      break;

    case DMX_START_BYTE:
      if (--dmxCount == 0) {
        /* start address reached. */
        if (dmxData != NULL) {
          dmxCount = 1;
          dmxState = DMX_START_ADDR;
          dmxData[0] = dmxByte;
        } else {
          dmxState = DMX_IDLE;
        }
      }
      break;

    case DMX_START_ADDR:
      dmxData[dmxCount++] = dmxByte;
      if (dmxCount >= dmxSize) {
        dmxState = DMX_IDLE;
      }
      break;

    case DMX_IDLE:
      break;
    }
  }
}
