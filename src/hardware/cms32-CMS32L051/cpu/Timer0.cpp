/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#include "Time.h"

// CMS32L051 stub: no timer interrupt set up — Time::callback() will never fire.
// TODO(cms32l051): use TIM4 or another available timer to call Time::callback()
// every TIMER_INTERRUPT_PERIOD_MICROSECONDS microseconds.

void Time::initialize()
{
}
