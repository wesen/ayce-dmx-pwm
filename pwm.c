#ifndef HOST
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay_basic.h>
#endif

#include "common.h"
#include "board.h"

/* rotated irq pwm data. */
uint8_t pwmIrqData[2][12] = { { 0 } };
uint8_t curPwmIdx = 0;
uint8_t *curPwmIrqData = pwmIrqData[0];
uint8_t *nextPwmIrqData = pwmIrqData[1];
uint8_t hasNextIrqData = 0;

uint16_t exptTable[256] = {
  0, 0, 0, 1, 1, 1, 1, 2, 2, 2,  /* 0 - 10 */
  2, 3, 3, 3, 3, 4, 4, 4, 5, 5,  /* 10 - 20 */
  5, 5, 6, 6, 7, 7, 7, 8, 8, 8,  /* 20 - 30 */
  9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 
  14, 14, 15, 15, 16, 16, 17, 18, 18, 19, 
  20, 20, 21, 22, 22, 23, 24, 25, 26, 26, /* 50 - 60 */
  27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 
  37, 38, 39, 40, 42, 43, 44, 45, 47, 48, 
  49, 51, 52, 54, 55, 57, 59, 60, 62, 64, 
  65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 
  86, 88, 90, 93, 95, 98, 100, 103, 106, 109, /* 100 - 110 */
  112, 115, 118, 121, 124, 127, 130, 134, 137, 141, 
  145, 148, 152, 156, 160, 164, 169, 173, 178, 182, 
  187, 192, 197, 202, 207, 212, 218, 223, 229, 235, 
  241, 247, 253, 260, 266, 273, 280, 287, 294, 302, 
  309, 317, 325, 333, 342, 350, 359, 368, 378, 387, /* 150 - 160 */
  397, 407, 417, 428, 438, 449, 461, 472, 484, 496, 
  509, 522, 535, 548, 562, 576, 590, 605, 620, 636, 
  651, 668, 684, 701, 719, 737, 755, 774, 793, 813, 
  833, 854, 876, 897, 920, 943, 966, 990, 1015, 1040, 
  1066, 1092, 1119, 1147, 1176, 1205, 1235, 1266, 1297, 1329, /* 200 */
  1362, 1396, 1431, 1466, 1502, 1540, 1578, 1617, 1657, 1698, 
  1740, 1783, 1828, 1873, 1919, 1967, 2016, 2066, 2117, 2169, 
  2223, 2278, 2334, 2392, 2451, 2512, 2574, 2638, 2703, 2770, 
  2839, 2909, 2981, 3055, 3130, 3208, 3287, 3368, 3451, 3537, 
  3624, 3714, 3806, 3900, 3996, 4095, /* 250 */
};

#ifndef HOST 
void initPWM() {
  uint8_t sreg_tmp = SREG;
  cli();

  /* prescale / 1 */
  TCCR1B = _BV(CS10) | _BV(WGM12);
  TCCR1A = 0;
  TCNT1 = 0;
  OCR1A = 1;
  SET_BIT(TIMSK, OCIE1A);

  SREG = sreg_tmp;
}

/* prescale  / 8 -> 48 cycles per PWM_MUL */
#define PWM_MUL 1

/* slightly higher than 2 factor =~ 2.01 for each step */
static const
uint16_t delayCycles[12] = {
  1, 2, 3, 4,
  15, 68,
  217, 510, 1100, 2290, 4700, 9400, 
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

/* pwm irq */
SIGNAL(TIMER1_COMPA_vect) {
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
  if (curPwmIdx >= 12) {
    curPwmIdx = 0;
    if (hasNextIrqData) {
      uint8_t *tmpPwmIrqData = curPwmIrqData;
      curPwmIrqData = nextPwmIrqData;
      nextPwmIrqData = tmpPwmIrqData;
      hasNextIrqData = 0;
    }
  }

  
}
#endif

void setRGBWColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
  uint16_t r12 = exptTable[red];
  uint16_t g12 = exptTable[green];
  uint16_t b12 = exptTable[blue];
  uint16_t w12 = exptTable[white];

  uint8_t i;
  for (i = 0; i < 12; i++) {
    nextPwmIrqData[i] =
      ((r12 & 1) << PWM4_PIN) |
      ((g12 & 1) << PWM3_PIN) |
      ((b12 & 1) << PWM2_PIN) |
      ((w12 & 1) << PWM1_PIN);
    /* invert */
    //    nextPwmIrqData[i] = ~nextPwmIrqData[i] & 0x78; /* 0b01111000 */
    r12 >>= 1;
    b12 >>= 1;
    g12 >>= 1;
    w12 >>= 1;
  }

  /* swap buffers */
  uint8_t sreg_tmp = SREG;
  cli();
  hasNextIrqData = 1;
  SREG = sreg_tmp;
}

void setRGBWColorImmediate(uint16_t red, uint16_t green, uint16_t blue, uint16_t white) {
  uint16_t r12 = red;
  uint16_t g12 = green;
  uint16_t b12 = blue;
  uint16_t w12 = white;

  uint8_t i;
  for (i = 0; i < 12; i++) {
    nextPwmIrqData[i] =
      ((r12 & 1) << PWM4_PIN) |
      ((g12 & 1) << PWM3_PIN) |
      ((b12 & 1) << PWM2_PIN) |
      ((w12 & 1) << PWM1_PIN);
    /* invert */
    //    nextPwmIrqData[i] = ~nextPwmIrqData[i] & 0x78; /* 0b01111000 */
    r12 >>= 1;
    b12 >>= 1;
    g12 >>= 1;
    w12 >>= 1;
  }

  /* swap buffers */
  uint8_t sreg_tmp = SREG;
  cli();
  hasNextIrqData = 1;
  SREG = sreg_tmp;
}
