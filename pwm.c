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
    0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 
    2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 
    5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 
    9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 
    14, 14, 15, 15, 16, 16, 17, 18, 18, 19, 
    20, 20, 21, 22, 22, 23, 24, 25, 26, 26, 
    27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 
    37, 38, 39, 40, 42, 43, 44, 45, 47, 48, 
    49, 51, 52, 54, 55, 57, 59, 60, 62, 64, 
    65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 
    86, 88, 90, 93, 95, 98, 100, 103, 106, 109, 
    112, 115, 118, 121, 124, 127, 130, 134, 137, 141, 
    145, 148, 152, 156, 160, 164, 169, 173, 178, 182, 
    187, 192, 197, 202, 207, 212, 218, 223, 229, 235, 
    241, 247, 253, 260, 266, 273, 280, 287, 294, 302, 
    309, 317, 325, 333, 342, 350, 359, 368, 378, 387, 
    397, 407, 417, 428, 438, 449, 461, 472, 484, 496, 
    509, 522, 535, 548, 562, 576, 590, 605, 620, 636, 
    651, 668, 684, 701, 719, 737, 755, 774, 793, 813, 
    833, 854, 876, 897, 920, 943, 966, 990, 1015, 1040, 
    1066, 1092, 1119, 1147, 1176, 1205, 1235, 1266, 1297, 1329, 
    1362, 1396, 1431, 1466, 1502, 1540, 1578, 1617, 1657, 1698, 
    1740, 1783, 1828, 1873, 1919, 1967, 2016, 2066, 2117, 2169, 
    2223, 2278, 2334, 2392, 2451, 2512, 2574, 2638, 2703, 2770, 
    2839, 2909, 2981, 3055, 3130, 3208, 3287, 3368, 3451, 3537, 
    3624, 3714, 3806, 3900, 3996, 4095, 
};

#ifndef HOST 
void initPWM() {
  uint8_t sreg_tmp = SREG;
  cli();

  /* prescale / 8 */
  TCCR1B = _BV(CS11) | _BV(WGM12);
  TCCR1A = 0;
  OCR1A = 1;
  SET_BIT(TIMSK, OCIE1A);
  TCNT1 = 0;

  SREG = sreg_tmp;
}

/* prescale  / 8 -> 48 cycles per PWM_MUL */
#define PWM_MUL 6

static const uint16_t delayCycles[12] = {
  1 * PWM_MUL, 2 * PWM_MUL, 4 * PWM_MUL, 8 * PWM_MUL, /* 4 lower ones are handled directly from irq */
  16 * PWM_MUL, 32 * PWM_MUL, 64 * PWM_MUL, 128 * PWM_MUL,
  256 * PWM_MUL, 512 * PWM_MUL, 1024 * PWM_MUL, 2048 * PWM_MUL
};

/* pwm irq */
SIGNAL(TIMER1_COMPA_vect) {
  TOGGLE_BIT(PORTD, LED_PIN);

  /* stop timer */
  TCCR1B = _BV(WGM12);
  CLEAR_BIT(TIMSK, OCIE1A);
  
  uint8_t curVal;
  if (curPwmIdx == 0) {
    /* handle first 4 values directly from irq, including delay. */
    curVal = PORTD & ~(_BV(PWM1_PIN) | _BV(PWM2_PIN) | _BV(PWM3_PIN) | _BV(PWM4_PIN));
    PORTD = curVal | curPwmIrqData[0];
    _delay_loop_1(4);

    curVal = PORTD & ~(_BV(PWM1_PIN) | _BV(PWM2_PIN) | _BV(PWM3_PIN) | _BV(PWM4_PIN));
    PORTD = curVal| curPwmIrqData[1];
    _delay_loop_1(40);

    curPwmIdx = 2;
  }

  curVal = PORTD & ~(_BV(PWM1_PIN) | _BV(PWM2_PIN) | _BV(PWM3_PIN) | _BV(PWM4_PIN));
  PORTD = (curVal) | curPwmIrqData[curPwmIdx];
  OCR1A = delayCycles[curPwmIdx];
  
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

  TCNT1 = 0;
  /* enable timer */
  TCCR1B = _BV(CS11) | _BV(WGM12);
  SET_BIT(TIMSK, OCIE1A);
  
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
