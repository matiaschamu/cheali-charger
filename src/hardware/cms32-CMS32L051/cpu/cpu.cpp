/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

#include "cpu.h"
#include <stdint.h>

uint8_t __atomic_h_irq_count;

namespace cpu {
    void init() {
        __atomic_h_irq_count = 0;
        // TODO(cms32l051): configure system clock (HOCO/HXT) via CLK_*.
        // SystemInit() in system_CMS32L051.c already sets defaults.
    }
}
