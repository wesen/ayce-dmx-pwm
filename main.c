#include <avr/io.h>
#include <avr/interrupt.h>

#include "common.h"
#include "board.h"
#include "dmx.h"

/* global variables. */
uint8_t pwmData[4];

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

  CLEAR_BIT(DDRC, ADDRESS1_PIN);
  CLEAR_BIT(DDRC, ADDRESS2_PIN);
  CLEAR_BIT(DDRC, ADDRESS3_PIN);
  CLEAR_BIT(DDRC, ADDRESS4_PIN);
  CLEAR_BIT(DDRC, ADDRESS5_PIN);

  CLEAR_BIT(DDRB, ADDRESS6_PIN);
  CLEAR_BIT(DDRB, ADDRESS7_PIN);
  CLEAR_BIT(DDRB, ADDRESS8_PIN);

  /* set pull up resistors. */
  SET_BIT(PORTC, ADDRESS1_PIN);
  SET_BIT(PORTC, ADDRESS2_PIN);
  SET_BIT(PORTC, ADDRESS3_PIN);
  SET_BIT(PORTC, ADDRESS4_PIN);
  SET_BIT(PORTC, ADDRESS5_PIN);

  SET_BIT(PORTB, ADDRESS6_PIN);
  SET_BIT(PORTB, ADDRESS7_PIN);
  SET_BIT(PORTB, ADDRESS8_PIN);

  //  initDMXRX(readAddress(), pwmData, 4);
}

/*
 * Reads out the DMX address set on the pin header.
 */
uint8_t readAddress() {
  uint8_t address = 0;

  address |= IS_BIT_SET(PINC, ADDRESS1_PIN);
  address |= (IS_BIT_SET(PINC, ADDRESS2_PIN)) << 1;
  address |= (IS_BIT_SET(PINC, ADDRESS3_PIN)) << 2;
  address |= (IS_BIT_SET(PINC, ADDRESS4_PIN)) << 3;
  address |= (IS_BIT_SET(PINC, ADDRESS5_PIN)) << 4;
  address |= (IS_BIT_SET(PINB, ADDRESS6_PIN)) << 5;
  address |= (IS_BIT_SET(PINB, ADDRESS7_PIN)) << 6;
  address |= (IS_BIT_SET(PINB, ADDRESS8_PIN)) << 7;

  return address + 1;
}

/*
 * Reads out the current temperature using the ntc.
 */
uint16_t readTemperature() {
  return 0;
}

/*
 * Reads out the current LED current using the 0.1 ref resistor.
 */
uint16_t readLEDCurrent() {
  return 0;
}

void mainDMX(void) {
  for (;;) {
    setDMXAddress(readAddress());
    setRGBWColor(pwmData[0], pwmData[1], pwmData[2], pwmData[3]);
    delay(10);
  }
}

/* pulse. */
void main1(void) {
  setRGBWColor(1, 1, 1, 0);

  int duration = 100;

  int low = 0;
  int high = 2558;
  
  int i = 0;
  int direction = 1;
  for (;;) {
    if (i < low) {
      i = low;
      direction = 1;
    }
    if (i > high) {
      i = high;
      direction = -1;
    }
    setRGBWColorImmediate(i, i, i, i);
    i += direction;
    delay(duration);
  }
}

void main2(void) {
  for (;;) {
  uint16_t address = readAddress() << 2;
  //  address = 0xAAAA;
  setRGBWColorImmediate(address, address, address, address);
    delay(100);
  }
}

void main4(void) {
  for (;;) {
  uint8_t address = readAddress();
  //  address = 0xAAAA;
  setRGBWColor(address, address, address, address);
    delay(100);
  }
}

void main3(void) {
  int duration = 1000;
  int low = 1;
  int high = low;

  /*
  duration = 100;
  low = 0;
  high = 15;
  */

  CLEAR_LED();
  int i = low;
  int direction = 1;
  for (;;) {
    if (i <= low) {
      i = low;
      direction = 1;
    }
    if (i >= high) {
      i = high;
      direction = -1;
      //      SET_LED();
    }
    setRGBWColorImmediate(i, i, i, i);
    i += direction;
    delay(duration);
  }
}

int main(void) {
  initBoard();
  initPWM();
  initDMXRX(0, pwmData, 4);
  sei();

  mainDMX();
  //  main4();
  return 0;
}
