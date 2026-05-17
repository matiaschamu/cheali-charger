/*
    TxSoftSerial - Software serial library (transmit only)
    Copyright (c) 2014 Sasa Mihajlovic. All right reserved.
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#include <stdint.h>
#include "TxSoftSerial.h"

// CMS32L051 stub: bit-banged UART TX not wired yet. Re-add timer-based bit
// shifter once a CMS32L051 timer driver is in place.

namespace TxSoftSerial {

void initialize()              {}
void begin(unsigned long)      {}
void write(uint8_t /*c*/)      {}
void flush()                   {}
void end()                     {}

} // namespace TxSoftSerial
