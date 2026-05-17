/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

// CMS32L051 stub: no ADC sampling loop wired yet. The full Nuvoton-M051
// implementation drives a mux + ADC burst sequence — port to vendor adc.h
// (R_ADC_*) when adapting to real hardware.

#include "atomic.h"
#include "Hardware.h"
#include "Utils.h"
#include "memory.h"
#include "Settings.h"
#include "AnalogInputsPrivate.h"
#include "IO.h"
#include "SMPS.h"
#include "Discharger.h"

namespace AnalogInputsADC {

void initialize()
{
    // TODO(cms32l051): configure ADC + sampling timer.
}

} // namespace AnalogInputsADC

namespace adc {
    void debug() {}
}
