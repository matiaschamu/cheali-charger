/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013  Paweł Stawicki. All right reserved.

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
#include "Time.h"
#include "Hardware.h"

extern "C" {
#include "CMS32L051.h"
#include "tim4.h"
}

extern "C" {
void TM00_Handler(void)
{
    if (TM40->TSR00 & _0001_TM4_OVERFLOW_OCCURS) {
        // Clear flag
        INTC_ClearPendingIRQ(TM00_IRQn);
    }
    Time::callback();
}
}


void Time::initialize()
{
    // Assume SystemCoreClock is already set (e.g. 64MHz)
    CGC->PER0 |= CGC_PER0_TM40EN_Msk;    /* enables input clock supply */
    
    // fCLK = 64MHz. Use CK00 = fCLK/1 = 64MHz
    TM40->TPS0 = _0000_TM4_CKM0_fCLK_0; 

    /* stop channel 0 */
    TM40->TT0 |= TM4_CHANNEL_0;    
    
    /* Channel 0 is used as interval timer */
    TM40->TMR00 = _0000_TM4_CLOCK_SELECT_CKM0 | _0000_TM4_CLOCK_MODE_CKS | _0000_TM4_TRIGGER_SOFTWARE |
                  _0000_TM4_MODE_INTERVAL_TIMER | _0000_TM4_START_INT_UNUSED;
    
    // Period = 1ms -> 64000 ticks
    TM40->TDR00 = (SystemCoreClock / (1000000/TIMER_INTERRUPT_PERIOD_MICROSECONDS)) - 1;
    
    TM40->TOE0 &= ~_0001_TM4_CH0_OUTPUT_ENABLE;
    
    /* enable interrupt */
    NVIC_ClearPendingIRQ(TM00_IRQn);
    NVIC_EnableIRQ(TM00_IRQn);
    // NVIC_SetPriority(TM00_IRQn, TIMER_IRQ_PRIORITY);
        
    /* Start channel 0 */
    TM40->TS0 |= TM4_CHANNEL_0;
}
