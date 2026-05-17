/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3. See the original IO.cpp header for details.
*/

#include "IO.h"

namespace IO {

volatile uint32_t _pin_storage[256] = {0};

void pinMode_(volatile uint32_t * /*pinAddress*/, uint8_t /*mode*/) {
    // TODO(cms32l051): configure GPIO port mode / digital path / ADC alt-func.
}

}
