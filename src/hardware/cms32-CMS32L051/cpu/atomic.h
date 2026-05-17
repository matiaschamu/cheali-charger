/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#ifndef ATOMIC_H_
#define ATOMIC_H_

#include <inttypes.h>

// CMSIS core intrinsics for __disable_irq / __enable_irq on Cortex-M0+.
extern "C" {
#include "CMS32L051.h"
}

extern uint8_t __atomic_h_irq_count;

static __inline__ uint8_t __iCliRetVal(void)
{
    __disable_irq();
    __asm__ volatile ("" ::: "memory");
    __atomic_h_irq_count++;
    return __atomic_h_irq_count;
}

static __inline__ void __iRestore(uint8_t * /*s*/)
{
    __atomic_h_irq_count--;
    if (__atomic_h_irq_count == 0) {
        __asm__ volatile ("" ::: "memory");
        __enable_irq();
    }
}

#define ATOMIC_BLOCK(type) for ( type = __iCliRetVal(), __ToDo = 1; \
                                 __ToDo; __ToDo = 0 )

#define ATOMIC_RESTORESTATE uint8_t sreg_save \
    __attribute__((__cleanup__(__iRestore)))

#endif /* ATOMIC_H_ */
