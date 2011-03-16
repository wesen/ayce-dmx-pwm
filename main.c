#include <avr/io.h>

#include "common.h"
#include "board.h"

/*
 * Initialize the board
 *
 * - Configure output ports: LED, PWM
 * - Configure input ports: ADDRESS
 * - Configure ADC for voltage and temperature
 */
void initBoard() {
  SET_BIT(DDRD, LED_PIN);
  SET_BIT(DDRD, PWM1_PIN);
  SET_BIT(DDRD, PWM2_PIN);
  SET_BIT(DDRD, PWM3_PIN);
  SET_BIT(DDRD, PWM4_PIN);
}

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

/*
 * Reads out the DMX address set on the pin header.
 */
uint8_t readAddress() {
}

/*
 * Reads out the current temperature using the ntc.
 */
uint16_t readTemperature() {
}

/*
 * Reads out the current LED current using the 0.1 ref resistor.
 */
uint16_t readLEDCurrent() {
}

int main(void) {
  initBoard();
  
  for (;;) {
    SET_PWM1();
    SET_PWM2();
    SET_PWM3();
    SET_PWM4();
    SET_LED();
    delay(100);
    CLEAR_PWM1();
    CLEAR_PWM2();
    CLEAR_PWM3();
    CLEAR_PWM4();
    CLEAR_LED();
    delay(100);
  }
  return 0;
}
