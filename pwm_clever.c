#include "board.h"

/* prescale  / 8 -> 48 cycles per PWM_MUL */
#define PWM_MUL 1

/* slightly higher than 2 factor =~ 2.01 for each step */
static const
uint16_t delayCycles_12[12] = {
  1, 2, 3, 4,
  15, 68,
  217, 510, 1100, 2290, 4700, 9400, 
};

static const
uint16_t delayCycles_10[10] = {
  1, 2, 3, 4,
  15, 68,
  217, 510, 1100, 2290,
};

#define SET_PWM(val) { \
  PORTD = (PORTD & ~(_BV(PWM1_PIN) | _BV(PWM2_PIN) | _BV(PWM3_PIN) | _BV(PWM4_PIN))) | (val); \
  asm("nop"); \
  asm("nop"); \
  asm("nop"); \
  asm("nop"); \
  asm("nop"); \
}

#define SET_PWM_2(val) { \
  PORTD = curVal | val; \
}


static inline void pwmClever() {
  
  /* stop timer */
  TCCR1B = _BV(WGM12);
  //  CLEAR_BIT(TIMSK, OCIE1A);
  
  uint8_t curVal;
  if (curPwmIdx == 0) {
    SET_LED();
    uint8_t curVal = (PORTD & ~(_BV(PWM1_PIN) | _BV(PWM2_PIN) | _BV(PWM3_PIN) | _BV(PWM4_PIN)));
    /* handle first 4 values directly from irq, including delay. */
    SET_PWM(curPwmIrqData[0]);
    asm("nop");
    SET_PWM(curPwmIrqData[1]);
    asm("nop");
    asm("nop");
    SET_PWM(curPwmIrqData[2]);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    SET_PWM(curPwmIrqData[3]);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    
    for (curPwmIdx = 4; curPwmIdx < 5; curPwmIdx++) {
      SET_PWM(curPwmIrqData[curPwmIdx]);
      _delay_loop_1(delayCycles[curPwmIdx]);
    }
    CLEAR_LED();
  }

  SET_PWM(curPwmIrqData[curPwmIdx]);

  OCR1A = delayCycles[curPwmIdx];
  /* enable timer */
  TCNT1 = 0;
  //  SET_BIT(TIMSK, OCIE1A);
  TCCR1B = _BV(CS10) | _BV(WGM12);

  curPwmIdx++;
  if (curPwmIdx >= PWM_BITS) {
    curPwmIdx = 0;
    if (hasNextIrqData) {
      uint8_t *tmpPwmIrqData = curPwmIrqData;
      curPwmIrqData = nextPwmIrqData;
      nextPwmIrqData = tmpPwmIrqData;
      hasNextIrqData = 0;
    }
  }
}

void setRGBWColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
  uint16_t rpwm = exptTable[red];
  uint16_t gpwm = exptTable[green];
  uint16_t bpwm = exptTable[blue];
  uint16_t wpwm = exptTable[white];

  uint8_t i;
  for (i = 0; i < PWM_BITS; i++) {
    nextPwmIrqData[i] =
      ((rpwm & 1) << PWM4_PIN) |
      ((gpwm & 1) << PWM3_PIN) |
      ((bpwm & 1) << PWM2_PIN) |
      ((wpwm & 1) << PWM1_PIN);
    /* invert */
    //    nextPwmIrqData[i] = ~nextPwmIrqData[i] & 0x78; /* 0b01111000 */
    rpwm >>= 1;
    bpwm >>= 1;
    gpwm >>= 1;
    wpwm >>= 1;
  }

  /* swap buffers */
  uint8_t sreg_tmp = SREG;
  cli();
  hasNextIrqData = 1;
  SREG = sreg_tmp;
}

void setRGBWColorImmediate(uint16_t red, uint16_t green, uint16_t blue, uint16_t white) {
  uint16_t rpwm = red;
  uint16_t gpwm = green;
  uint16_t bpwm = blue;
  uint16_t wpwm = white;

  uint8_t i;
  for (i = 0; i < PWM_BITS; i++) {
    nextPwmIrqData[i] =
      ((rpwm & 1) << PWM4_PIN) |
      ((gpwm & 1) << PWM3_PIN) |
      ((bpwm & 1) << PWM2_PIN) |
      ((wpwm & 1) << PWM1_PIN);
    /* invert */
    //    nextPwmIrqData[i] = ~nextPwmIrqData[i] & 0x78; /* 0b01111000 */
    rpwm >>= 1;
    bpwm >>= 1;
    gpwm >>= 1;
    wpwm >>= 1;
  }

  /* swap buffers */
  uint8_t sreg_tmp = SREG;
  cli();
  hasNextIrqData = 1;
  SREG = sreg_tmp;
}

#define delayCycles concat(delayCycles_, str(PWM_BITS))

uint8_t pwmIrqData[2][PWM_BITS] = { { 0 } };
uint8_t curPwmIdx = 0;
uint8_t *curPwmIrqData = pwmIrqData[0];
uint8_t *nextPwmIrqData = pwmIrqData[1];
uint8_t hasNextIrqData = 0;

