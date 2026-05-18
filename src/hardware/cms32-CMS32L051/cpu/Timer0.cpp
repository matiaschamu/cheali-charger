/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

/*
 * Periodic 500 µs tick using Cortex-M0+ SysTick.
 *
 * Reload = SystemCoreClock / (1 000 000 / TIMER_INTERRUPT_PERIOD_MICROSECONDS)
 *        = 48 000 000 / 2 000 = 24 000 counts  →  exactly 500 µs @ 48 MHz.
 *
 * The SysTick_Handler weak symbol defined in startup_CMS32L051.S is overridden
 * here with the real Time::callback() dispatch.
 */

#include "Time.h"
#include "irq_priority.h"

extern "C" {
#include "CMS32L051.h"   /* SystemCoreClock, SysTick_Config, NVIC_SetPriority */
}

extern "C" void SysTick_Handler(void)
{
    Time::callback();
}

void Time::initialize()
{
    uint32_t ticks = SystemCoreClock / (1000000u / TIMER_INTERRUPT_PERIOD_MICROSECONDS);
    SysTick_Config(ticks);                                  /* enables SysTick at requested period */
    NVIC_SetPriority(SysTick_IRQn, TIMER_IRQ_PRIORITY);    /* priority 2 out of 3 (0 = highest)   */
}
