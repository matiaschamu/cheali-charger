/*
    TxSoftSerial - Software serial library (transmit only)
    Copyright (c) 2014 Sasa Mihajlovic.  All right reserved.

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
#include <atomic>
#include "Hardware.h"
#include "TxSoftSerial.h"
#include "irq_priority.h"
#include "Serial.h"

#include "IO.h"

#ifdef CMS32L051
extern "C" {
#include "CMS32L051.h"
#include "tim4.h"
}
#define GPIO_PMD_OUTPUT OUTPUT
#define TMR2_MODULE 0 // Placeholder
#define CLK_EnableModuleClock(x) do { CGC->PER0 |= CGC_PER0_TM40EN_Msk; } while(0)
#define CLK_SetModuleClock(a,b,c) 
#define CLK_CLKSEL1_TMR2_S_HCLK 0
#define CLK_CLKDIV_UART(x) 0
#define TMR2_IRQn TM03_IRQn
#define TMR2_IRQHandler TM03_Handler
#define TIMER2 TM40
#define TIMER_PERIODIC_MODE 0
#define TIMER_Open(a,b,c) do { \
    a->TPS0 &= ~0x000F; \
    a->TPS0 |= 0x0000; \
    a->TMR03 = _0000_TM4_CLOCK_SELECT_CKM0 | _0000_TM4_CLOCK_MODE_CKS | _0000_TM4_TRIGGER_SOFTWARE | \
               _0000_TM4_MODE_INTERVAL_TIMER | _0000_TM4_START_INT_UNUSED; \
    a->TDR03 = (SystemCoreClock / (c)) - 1; \
} while(0)
#define TIMER_EnableInt(a) NVIC_EnableIRQ(TMR2_IRQn)
#define TIMER_Start(a) a->TS0 |= (1 << 3)
#define TIMER_Stop(a) a->TT0 |= (1 << 3)
#define TIMER_DisableInt(a) NVIC_DisableIRQ(TMR2_IRQn)
#define TIMER_ClearIntFlag(a) INTC_ClearPendingIRQ(TMR2_IRQn)
#ifndef UART_TX_PIN
#define UART_TX_PIN ((1 << 3) | 2) // Placeholder P1.2
#endif
#endif

namespace TxSoftSerial {

#define Tx_BUFFER_SIZE  256

#define START_BIT 0
#define STOP_BIT 512

uint8_t  *txBuffer_=Serial::txBuffer;

uint16_t txData_;
volatile uint32_t *txPin_;

std::atomic<uint16_t> tail_(0);
std::atomic<uint16_t> head_(0);

void disableTxPin() {
    //we set TX pin to ANALOG_INPUT for ext. temp.
    IO::pinMode(UART_TX_PIN, ANALOG_INPUT);
}
void enableTxPin() {
    IO::disableFuncADC(IO::getADCChannel(UART_TX_PIN));
    IO::pinMode(UART_TX_PIN, GPIO_PMD_OUTPUT);
}

void initialize()
{
    CLK_EnableModuleClock(TMR2_MODULE);
    CLK_SetModuleClock(TMR2_MODULE,CLK_CLKSEL1_TMR2_S_HCLK,CLK_CLKDIV_UART(1));
    NVIC_SetPriority(TMR2_IRQn, SOFTWARE_SERIAL_IRQ_PRIORITY);
    NVIC_EnableIRQ(TMR2_IRQn);
    txPin_ = IO::getPinAddress_(UART_TX_PIN);

    disableTxPin();
}

void begin(unsigned long baud)
{
    enableTxPin();

    TIMER_Open(TIMER2, TIMER_PERIODIC_MODE, baud);
    TIMER_EnableInt(TIMER2);

    IO::digitalWrite(UART_TX_PIN, 1);     // Tx pin high (IDLE)
    txData_ = 0;

    TIMER_Start(TIMER2);
}


void write(uint8_t ucData)
{
    uint16_t i = (head_.load(std::memory_order_relaxed) + 1) % Tx_BUFFER_SIZE;

    while(i == tail_.load(std::memory_order_acquire));

    txBuffer_[i] = ucData;
    head_.store(i,  std::memory_order_release);
}


void flush()
{
    bool empty;
    do {
        uint16_t i = tail_.load(std::memory_order_acquire);
        empty = (i == head_.load(std::memory_order_relaxed)) && (txData_ == 0);
    } while(!empty);
}

void end()
{
    TIMER_Stop(TIMER2);
    TIMER_DisableInt(TIMER2);

    disableTxPin();
}

inline void getNewData() {
    uint16_t i = tail_.load(std::memory_order_relaxed);
    if(i == head_.load(std::memory_order_acquire)) return;

    i = (i + 1) % Tx_BUFFER_SIZE;
    txData_ = START_BIT + ((txBuffer_[i]) << 1) + STOP_BIT;
    tail_.store(i, std::memory_order_release);
}

extern "C"
{
void TMR2_IRQHandler(void) {
    if(txData_) {
        IO::digitalWrite(UART_TX_PIN, txData_ & 1);
        txData_ >>= 1;
    } else {
        getNewData();
    }
    TIMER_ClearIntFlag(TIMER2);
}
}


} // namespace TxSoftSerial



