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
#ifndef ANALOG_INPUTS_ADC_H_
#define ANALOG_INPUTS_ADC_H_

#include <stdint.h>

namespace AnalogInputsADC
{
    void initialize();

    /* Fast burst-average path used by the CMS power loop. The public
     * AnalogInputs::getADCValue() path is the slower 256-sample result. */
    uint16_t getFastADCValue(uint8_t name);
};

#endif /* ANALOG_INPUTS_ADC_H_ */
