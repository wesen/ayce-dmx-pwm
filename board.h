#ifndef BOARD_H__
#define BOARD_H__

#include "common.h"

extern uint8_t dmxAddress;
extern uint8_t *curPwmIrqData;
extern uint8_t *nextPwmIrqData;

void initBoard();
uint8_t readAddress();
uint16_t readTemperature();
uint16_t readLEDCurrent();

void initPWM();
void setRGBWColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t white);
void setRGBWColorImmediate(uint16_t red, uint16_t green, uint16_t blue, uint16_t white);

#define LED_PIN PD7

#define PWM1_PIN PD3
#define PWM2_PIN PD4
#define PWM3_PIN PD5
#define PWM4_PIN PD6

#define DMX_RE_PIN PD2

#define ADDRESS1_PIN PC1
#define ADDRESS2_PIN PC2
#define ADDRESS3_PIN PC3
#define ADDRESS4_PIN PC4
#define ADDRESS5_PIN PC5
#define ADDRESS6_PIN PB0
#define ADDRESS7_PIN PB1
#define ADDRESS8_PIN PB2

#ifndef HOST

/* set LED. */
#define SET_LED() { SET_BIT(PORTD, LED_PIN); }
#define CLEAR_LED() { CLEAR_BIT(PORTD, LED_PIN); }

/* set pwm. */
#define SET_PWM1() { SET_BIT(PORTD, PWM1_PIN); }
#define CLEAR_PWM1() { CLEAR_BIT(PORTD, PWM1_PIN); }

#define SET_PWM2() { SET_BIT(PORTD, PWM2_PIN); }
#define CLEAR_PWM2() { CLEAR_BIT(PORTD, PWM2_PIN); }

#define SET_PWM3() { SET_BIT(PORTD, PWM3_PIN); }
#define CLEAR_PWM3() { CLEAR_BIT(PORTD, PWM3_PIN); }

#define SET_PWM4() { SET_BIT(PORTD, PWM4_PIN); }
#define CLEAR_PWM4() { CLEAR_BIT(PORTD, PWM4_PIN); }

#endif /* !HOST */

#endif /* BOARD_H__ */
