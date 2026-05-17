/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

#include "outputPWM.h"
#include <stdint.h>

// CMS32L051 stub: no PWM channels wired. The Nuvoton implementation drives
// two PWM units (PWMA/PWMB) for buck/boost. Port to vendor TIM4 (R_TIM4_*)
// PWM mode when adapting to real hardware.

namespace outputPWM {

void initialize(void)
{
}

void setPWM(uint8_t /*pin*/, uint32_t /*value*/)
{
}

void disablePWM(uint8_t /*pin*/)
{
}

} // namespace outputPWM
