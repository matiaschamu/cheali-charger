/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

#ifndef IO_H_
#define IO_H_

#include <stdint.h>

/*
 * CMS32L051 pin encoding
 *
 * A single uint8_t encodes both port and bit:
 *   encoded = port * 8 + bit
 *
 * Valid ports: 0-7, 12, 13, 14.  All encoded values ≤ 119 < 128.
 * Values ≥ 128 are reserved for virtual/internal pins (e.g. T_INTERNAL_PIN).
 *
 * Usage in imaxB6-pins.h:
 *   #define MY_PIN   CMS32_PIN(3, 0)   // P30
 */
#define CMS32_PIN(port, bit)   ((uint8_t)((uint8_t)(port) * 8u + (uint8_t)(bit)))

/* cheali-charger mode constants — intentionally different numeric values from
 * the vendor gpio.h PIN_ModeDef enum (which uses OUTPUT=4, INPUT=0).
 * The translation happens inside IO.cpp. */
#define OUTPUT                  0
#define INPUT                   1
#define ANALOG_INPUT            200
#define ANALOG_INPUT_DISCHARGE  201
#define HIGH 1
#define LOW  0

/* Compatibility: some shared code references this after including IO.h */
#define GPIO_PMD_OUTPUT 0

namespace IO
{
    /* Configure pin direction/mode.
     * mode: OUTPUT(0), INPUT(1), ANALOG_INPUT(200/201) */
    void pinMode(uint8_t pinNumber, uint8_t mode);

    /* Drive a digital output high (value != 0) or low (value == 0). */
    void digitalWrite(uint8_t pinNumber, uint32_t value);

    /* Read the current logic level of a pin. */
    uint8_t digitalRead(uint8_t pinNumber);

    /* Return the ANI channel number for an analog-capable pin.
     * Returns 0xFF if the pin has no ADC channel. */
    uint8_t getADCChannel(uint8_t pinNumber);

    /* ADC function enable/disable stubs (full ADC init is in AnalogInputsADC.cpp). */
    inline void enableFuncADC(uint32_t /*adc*/)  {}
    inline void disableFuncADC(uint32_t /*adc*/) {}
}

#endif /* IO_H_ */
