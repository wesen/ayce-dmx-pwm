#include <avr/io.h>
#include <avr/interrupt.h>

#include "board.h"
#include "dmx.h"

#ifdef DMX_RECEIVE
dmx_rx_state_t dmxRxState = DMX_IDLE;
static uint16_t dmxAddress = 0;
static uint16_t dmxSize = 0;
static uint8_t *dmxData = NULL;

#define UART_BAUDRATE_REG (((F_CPU / 16L) / (DMX_BAUDRATE)) - 1)

void initDMXRX(uint16_t _dmxAddress, uint8_t *_dmxData, uint16_t _dmxSize) {
  UBRRH = ((UART_BAUDRATE_REG) >> 8) & 0xFF;
  UBRRL = UART_BAUDRATE_REG & 0xFF;
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) | _BV(USBS); /* 8n2 */
  UCSRB = _BV(RXEN) | _BV(RXCIE); /* enabled receive */
  SET_BIT(DDRD, PD2);
  CLEAR_BIT(PORTD, PD2); /* read enable */
  dmxRxState = DMX_IDLE;
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

SIGNAL(USART_RXC_vect) {
  static uint16_t dmxCount;
  uint8_t uartState = UCSRA;
  uint8_t dmxByte = UDR;
  
  /* check for framing error -> BREAK */
  if (IS_BIT_SET(uartState, FE)) {
    CLEAR_BIT(UCSRA, FE);
    dmxCount = dmxAddress;
    dmxRxState = DMX_BREAK;
  } else {
    switch (dmxRxState) {
    case DMX_BREAK:
      if (dmxByte == 0) {
        dmxRxState = DMX_START_BYTE;
      } else {
        dmxRxState = DMX_IDLE;
      }
      break;

    case DMX_START_BYTE:
      if (--dmxCount == 0) {
        /* start address reached. */
        if (dmxData != NULL) {
          dmxCount = 1;
          dmxRxState = DMX_START_ADDR;
          dmxData[0] = dmxByte;
          SET_LED();
        } else {
          dmxRxState = DMX_IDLE;
        }
      }
      break;

    case DMX_START_ADDR:
      dmxData[dmxCount++] = dmxByte;
      if (dmxCount >= dmxSize) {
        dmxRxState = DMX_IDLE;
        CLEAR_LED();
      }
      break;

    case DMX_IDLE:
      break;
    }
  }
}
#endif /* DMX_RECEIVE */
