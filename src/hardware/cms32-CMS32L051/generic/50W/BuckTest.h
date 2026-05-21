/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#ifndef BUCK_TEST_H_
#define BUCK_TEST_H_

/*
 * Manual buck-converter exerciser.
 *
 * Drives a hardware PWM on P15 via TM41 ch0/ch1 (TO11) at ~30 kHz.
 *  - INC / DEC  : ±1 % duty (auto-repeat / speed factor for fast scrolling)
 *  - START      : toggle output ON/OFF (gates PWM and OUTPUT_DISABLE_PIN)
 *  - STOP       : exit (PWM stopped, output disabled, duty reset)
 *
 * The test menu is HW-specific and lives in this port's generic-50W layer; it
 * is reached from OptionsMenu when ENABLE_BUCK_TEST is defined.
 */

namespace BuckTest {

void run();

} // namespace BuckTest

#endif /* BUCK_TEST_H_ */
