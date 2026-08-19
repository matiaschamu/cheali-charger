/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#ifndef BUCK_TEST_H_
#define BUCK_TEST_H_

/*
 * CMS32L051 laboratory power-test suite.
 *
 * Includes manual P15 PWM, the combined buck/boost production transition,
 * live power ADC, P20 charger/discharger routing, P00 cutoff and individual
 * balance outputs. All test exits force the board back to safe idle.
 *
 * The suite is hardware-specific and lives only in this port. It is reached
 * through the existing Options -> buck test hook when ENABLE_BUCK_TEST is
 * defined, so adding submenus does not add CMS knowledge to the shared core.
 */

namespace BuckTest {

void run();

} // namespace BuckTest

#endif /* BUCK_TEST_H_ */
