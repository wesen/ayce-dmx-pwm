#ifndef HOST
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay_basic.h>
#endif

#include "common.h"
#include "board.h"

/* rotated irq pwm data. */
#define PWM_BITS 12
#define concat(s, y) x ## y
#define xstr(s) str(s)
#define str(s) #s
#define exptTable concat(exptTable_, str(PWM_BITS))

uint16_t exptTable_12[256] = {
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

uint16_t exptTable_10[256] = {
    0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 
    3, 4, 4, 5, 5, 5, 6, 6, 7, 7, 
    7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 
    12, 13, 13, 14, 14, 15, 15, 16, 17, 17, 
    18, 18, 19, 20, 20, 21, 21, 22, 23, 23, 
    24, 25, 25, 26, 27, 28, 28, 29, 30, 31, 
    32, 32, 33, 34, 35, 36, 37, 37, 38, 39, 
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 
    50, 51, 53, 54, 55, 56, 57, 58, 60, 61, 
    62, 63, 65, 66, 67, 69, 70, 72, 73, 74, 
    76, 77, 79, 80, 82, 84, 85, 87, 89, 90, 
    92, 94, 96, 97, 99, 101, 103, 105, 107, 109, 
    111, 113, 115, 117, 119, 121, 123, 126, 128, 130, 
    133, 135, 137, 140, 142, 145, 147, 150, 153, 155, 
    158, 161, 164, 166, 169, 172, 175, 178, 181, 184, 
    188, 191, 194, 197, 201, 204, 208, 211, 215, 218, 
    222, 226, 230, 234, 238, 242, 246, 250, 254, 258, 
    262, 267, 271, 276, 280, 285, 290, 295, 299, 304, 
    309, 315, 320, 325, 330, 336, 341, 347, 353, 358, 
    364, 370, 376, 382, 389, 395, 401, 408, 415, 421, 
    428, 435, 442, 449, 456, 464, 471, 479, 487, 495, 
    503, 511, 519, 527, 536, 544, 553, 562, 571, 580, 
    589, 599, 608, 618, 628, 638, 648, 658, 669, 680, 
    690, 701, 713, 724, 735, 747, 759, 771, 783, 796, 
    808, 821, 834, 847, 861, 875, 888, 902, 917, 931, 
    946, 961, 976, 991, 1007, 1023, 
};

#ifndef HOST 
void initPWM() {
  uint8_t sreg_tmp = SREG;
  cli();

  /* prescale / 1 */
  TCCR1B = _BV(CS10) | _BV(WGM12);
  TCCR1A = 0;
  TCNT1 = 0;
  OCR1A = F_CPU / (100L * 1024L); // for 10 bits
  SET_BIT(TIMSK, OCIE1A);

  SREG = sreg_tmp;
}

static inline void pwmStandard();

/* pwm irq */
SIGNAL(TIMER1_COMPA_vect) {
  pwmStandard();
}
#endif

uint16_t redPwmCount = 0;
uint16_t greenPwmCount = 0;
uint16_t bluePwmCount = 0;
uint16_t whitePwmCount = 0;

static inline void pwmStandard() {
  static uint16_t pwmCount = 1023;

  uint8_t pwmVal = 0;

  if (pwmCount < redPwmCount) {
    SET_BIT(pwmVal, PWM_RED_PIN);
  }
  if (pwmCount < bluePwmCount) {
    SET_BIT(pwmVal, PWM_BLUE_PIN);
  }
  if (pwmCount < greenPwmCount) {
    SET_BIT(pwmVal, PWM_GREEN_PIN);
  }
  if (pwmCount < whitePwmCount) {
    SET_BIT(pwmVal, PWM_WHITE_PIN);
  }
  pwmCount = (pwmCount - 1) & 1023;

  PORTD = (PORTD & ~(_BV(PWM1_PIN) | _BV(PWM2_PIN) | _BV(PWM3_PIN) | _BV(PWM4_PIN))) | (pwmVal);
}

void setRGBWColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
  uint8_t sreg_tmp = SREG;
  cli();
  redPwmCount = exptTable_10[red];
  greenPwmCount = exptTable_10[green];
  bluePwmCount = exptTable_10[blue];
  whitePwmCount = exptTable_10[white];
  SREG = sreg_tmp;
}

void setRGBWColorImmediate(uint16_t red, uint16_t green, uint16_t blue, uint16_t white) {
  uint8_t sreg_tmp = SREG;
  cli();
  redPwmCount = red;
  greenPwmCount = green;
  bluePwmCount = blue;
  whitePwmCount = white;
  SREG = sreg_tmp;
}
