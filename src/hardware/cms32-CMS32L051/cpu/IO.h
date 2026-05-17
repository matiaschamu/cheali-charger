/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// CMS32L051 stub: no-op GPIO layer. All functions compile and link, but do
// nothing in hardware. Replace bodies with real CMS32L051 GPIO driver calls.

#ifndef IO_H_
#define IO_H_

#include <stdint.h>

#define OUTPUT                 0
#define INPUT                  1
#define ANALOG_INPUT           200
#define ANALOG_INPUT_DISCHARGE 201
#define HIGH 1
#define LOW  0

// Compatibility token referenced from a Nuvoton-era code path in TxSoftSerial.
#define GPIO_PMD_OUTPUT 0

#define INLINE_ATTR __attribute__((always_inline))

namespace IO
{
    // Storage backing the pin pseudo-address table. One word per pin number.
    // Real implementation should map pinNumber -> port/bit and access the
    // CMS32L051 GPIO peripheral.
    extern volatile uint32_t _pin_storage[256];

    inline volatile uint32_t * getPinAddress_(uint8_t pinNumber) INLINE_ATTR;
    inline uint32_t getPinBit_(volatile uint32_t * pinAddress) INLINE_ATTR;
    inline uint32_t getADCChannel(uint8_t pinNumber) INLINE_ATTR;
    inline void digitalWrite(uint8_t pinNumber, uint32_t value) INLINE_ATTR;
    inline uint8_t digitalRead(uint8_t pinNumber) INLINE_ATTR;
    inline void enableFuncADC(uint32_t adc) INLINE_ATTR;
    inline void disableFuncADC(uint32_t adc) INLINE_ATTR;
    inline void pinMode(uint8_t pinNumber, uint8_t mode) INLINE_ATTR;

    inline volatile uint32_t * getPinAddress_(uint8_t pinNumber) {
        return &_pin_storage[pinNumber];
    }

    inline uint32_t getPinBit_(volatile uint32_t * pinAddress) {
        return ((uintptr_t)pinAddress >> 2) & 7;
    }

    inline uint32_t getADCChannel(uint8_t pinNumber) {
        return getPinBit_(getPinAddress_(pinNumber));
    }

    inline void digitalWrite(uint8_t pinNumber, uint32_t value) {
        _pin_storage[pinNumber] = value ? 1 : 0;
    }

    inline uint8_t digitalRead(uint8_t pinNumber) {
        return (uint8_t) _pin_storage[pinNumber];
    }

    inline void enableFuncADC(uint32_t /*adc*/)  {}
    inline void disableFuncADC(uint32_t /*adc*/) {}

    void pinMode_(volatile uint32_t * pinAddress, uint8_t mode);

    inline void pinMode(uint8_t pinNumber, uint8_t mode) {
        pinMode_(getPinAddress_(pinNumber), mode);
    }
}

#endif /* IO_H_ */
