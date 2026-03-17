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
#ifndef IO_H_
#define IO_H_

#include <stdint.h>
extern "C" {
#include "CMS32L051.h"
#include "gpio.h"
}

#define OUTPUT 4 // Match gpio.h PIN_ModeDef
#define INPUT  0 // Match gpio.h PIN_ModeDef
#define ANALOG_INPUT 3 // Match gpio.h PIN_ModeDef
#define HIGH 1
#define LOW 0

#define INLINE_ATTR __attribute__((always_inline))
//#define INLINE_ATTR

namespace IO
{
    inline void digitalWrite(uint8_t pinNumber, uint32_t value) {
        uint8_t port = pinNumber >> 3;
        uint8_t pin = pinNumber & 0x07;
        if (value) {
            PORT_SetBit((PORT_TypeDef)port, (PIN_TypeDef)pin);
        } else {
            PORT_ClrBit((PORT_TypeDef)port, (PIN_TypeDef)pin);
        }
        __DSB();
    };

    inline uint8_t digitalRead(uint8_t pinNumber) {
        uint8_t port = pinNumber >> 3;
        uint8_t pin = pinNumber & 0x07;
        return PORT_GetBit((PORT_TypeDef)port, (PIN_TypeDef)pin) ? 1 : 0;
    }

    inline void enableFuncADC(uint32_t adc) {
        // TODO: implement
    }
    inline void disableFuncADC(uint32_t adc) {
        // TODO: implement
    }

    void pinMode_(uint8_t port, uint8_t pin, uint8_t mode);

    inline void pinMode(uint8_t pinNumber, uint8_t mode) {
        pinMode_(pinNumber >> 3, pinNumber & 0x7, mode);
    };

    inline uint32_t getADCChannel(uint8_t pinNumber) {
        // TODO: implement ADC channel mapping
        return pinNumber & 0x7; // dummy
    }

    extern uint32_t dummyPin;
    inline volatile uint32_t* getPinAddress_(uint8_t pinNumber) {
        // This is problematic on CMS32 as we don't have bit-addressable IO
        // For now return a pointer to a dummy
        return &dummyPin;
    }
}
#endif /* IO_H_ */

