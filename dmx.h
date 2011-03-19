#ifndef DMX_H__
#define DMX_H__

typedef enum dmx_state_e {
  DMX_IDLE = 0,
  DMX_BREAK,
  DMX_START_BYTE,
  DMX_START_ADDR
} dmx_state_t;

#define DMX_BAUDRATE 250000L

void initDMX(uint16_t _dmxAddress, uint8_t *_dmxData, uint16_t dmxSize);
void setDMXAddress(uint16_t _dmxAddress);
void setDMXData(uint8_t *_dmxData, uint16_t _dmxSize);

#endif /* DMX_H__ */
