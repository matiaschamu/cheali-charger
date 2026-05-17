/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

#include "Utils.h"
#include <stdint.h>

// Busy-loop delay. Cycle count is core-clock dependent; calibrate when the
// real CMS32L051 clock setup lands.
namespace Utils
{
    void Delay(uint32_t x)
    {
        asm volatile(
         ".syntax unified;"
         "ldr r3, %[in];"
         "1:; "
         "subs r3, #1;"
         "bne 1b;"
         ".syntax divided;"
         :
         :[in] "m" (x)
         :"r3","cc","memory"
        );
    }

    // This method should be only used by the LCD.
    void delayMicroseconds(uint16_t value)
    {
        uint32_t x = value;
        x *= 26982;
        x /= 4096;
        Delay(x);
    }
}
