#ifndef DMX_H__
#define DMX_H__

#define DMX_BAUDRATE 250000L

#ifdef DMX_RECEIVE
typedef enum dmx_rx_state_e {
  DMX_IDLE = 0,
  DMX_BREAK,
  DMX_START_BYTE,
  DMX_START_ADDR
} dmx_rx_state_t;

void initDMXRX(uint16_t _dmxAddress, uint8_t *_dmxData, uint16_t _dmxSize);
void setDMXAddress(uint16_t _dmxAddress);
void setDMXData(uint8_t *_dmxData, uint16_t _dmxSize);
#endif /* DMX_RECEIVE */

#ifdef DMX_SEND
typedef enum dmx_tx_state_e {
} dmx_tx_state_t;

void initDMXTX(uint8_t *_dmxData, uint16_t _dmxSize);

#endif /* DMX_SEND */

#endif /* DMX_H__ */
