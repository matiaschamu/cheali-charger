/*
    TxHardSerial - Hardware serial library (transmit only)
    Copyright (c) 2014 Sasa Mihajlovic. All right reserved.
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#include <stdint.h>
#include "TxHardSerial.h"

// CMS32L051 stub: hardware UART TX not wired yet. Adapt to vendor SCI driver.

namespace TxHardSerial {

void initialize()              {}
void begin(unsigned long)      {}
void write(uint8_t /*c*/)      {}
void flush()                   {}
void end()                     {}

} // namespace TxHardSerial
