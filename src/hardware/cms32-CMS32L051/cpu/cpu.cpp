/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

extern "C" {
#include "CMS32L051.h"
#include "clk.h"
}
#include "cpu.h"

uint8_t __atomic_h_irq_count;

namespace cpu {
    void init() {
        __atomic_h_irq_count = 0;

        /*
         * Use the internal HOCO oscillator (fIH = 48 MHz, set by option byte
         * at 0xC2 = 0xE0).  X1/X2 and XT1/XT2 pins are released as GPIO.
         */
        CLK_Osc_Setting(OSC_PORT, OSC_PORT);
        CLK_Fclk_Select(MAINCLK_FIH);
        SystemCoreClockUpdate();   /* update SystemCoreClock = 48 000 000 Hz */
    }
}
