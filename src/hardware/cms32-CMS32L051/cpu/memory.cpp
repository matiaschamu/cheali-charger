/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#include "memory.h"

// CMS32L051 stub: EEPROM-emulation via on-chip flash not implemented.
// TODO(cms32l051): use vendor flash.h (R_FLASH_*) erase/program APIs to
// persist data into the data-flash region declared in the linker script.

namespace eeprom {

void write_impl(uint8_t * /*addressE*/, const uint8_t * /*data*/, int /*size*/)
{
}

} // namespace eeprom
