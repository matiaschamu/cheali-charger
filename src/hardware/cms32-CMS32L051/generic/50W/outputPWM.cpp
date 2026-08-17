/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

#include "outputPWM.h"
#include "IO.h"
#include "imaxB6-pins.h"
#include <stdint.h>

extern "C" {
#include "CMS32L051.h"
#include "tim4.h"
}

/*
 * Single hardware PWM channel on the imaxB6-80W (CMS32L051):
 *
 *   TM41 ch0 (master, period) + ch1 (slave, duty) → TO11 → P15
 *
 * Both buck and boost share this single output; the topology selector
 * is on P21 (driven outside outputPWM via hardware::setTopology()).
 * Discharge current modulation also uses the same P15 — P20 routes the
 * current to either the charger path (HIGH) or the discharger (LOW).
 *
 * The `pin` argument to setPWM/disablePWM is ignored for now (kept in
 * the signature for cross-port API compatibility). A future revision
 * may use it to dispatch to a second PWM channel if/when needed.
 */

namespace outputPWM {

/* Experimental TM41 PWM period: 48 MHz / 800 = 60 kHz. This frequency was
 * taken from another charger and still requires oscilloscope and thermal
 * validation on this board. Resolution = 1/800 = 0.125 % per tick. */
static constexpr uint16_t TM41_PWM_PERIOD_TICKS = 800;

/* External value scale exposed to SMPS_PID is 0..OUTPUT_PWM_PRECISION_PERIOD. */
static constexpr uint32_t VALUE_FULL_SCALE = OUTPUT_PWM_PRECISION_PERIOD;

static bool pwm_running_ = false;

/* Convert the public 0..VALUE_FULL_SCALE input to actual high-time in
 * timer ticks (0..TM41_PWM_PERIOD_TICKS). 0 → 0, full-scale → period. */
static uint16_t value_to_ticks(uint32_t value)
{
    if (value >= VALUE_FULL_SCALE) return TM41_PWM_PERIOD_TICKS;
    return (uint16_t)((value * TM41_PWM_PERIOD_TICKS) / VALUE_FULL_SCALE);
}

static void p15_drive(uint8_t high)
{
    PORT->P15CFG  = 0x00;         /* alt fn GPIO */
    PORT->PMC1   &= ~(1u << 5);   /* digital     */
    PORT->PM1    &= ~(1u << 5);   /* output      */
    if (high) PORT->P1 |=  (1u << 5);
    else      PORT->P1 &= ~(1u << 5);
}

static void p15_drive_low()  { p15_drive(0); }
static void p15_drive_high() { p15_drive(1); }

/* Route P15 to TM41/TO11 (vendor macro from userdefine.h is `static` so
 * we replicate it here). */
static void p15_route_pwm()
{
    PORT->P15CFG  = 0x02;             /* allocate TO11 to P15 */
    PORT->P1     &= ~(1u << 5);       /* output low default   */
    PORT->PM1    &= ~(1u << 5);       /* output mode          */
    PORT->POM1   &= ~(1u << 5);       /* normal output        */
    PORT->PMC1   &= ~(1u << 5);       /* digital              */
}

void initialize(void)
{
    pwm_running_ = false;
    p15_drive_low();
}

/*
 * Set output PWM duty.
 *
 * The slave channel's TDR is "high-time minus one tick" on this chip:
 * the timer underflow that ends the high pulse fires after TDR+1
 * counts. A naive TDR = high_ticks therefore stretches the pulse by
 * one tick (~21 ns with the 48 MHz timer clock) and TDR = 0 still emits a
 * 1-tick spike
 * instead of a true zero.
 *
 * Handle 0 % and full-scale explicitly — disable the PWM output and
 * drive P15 as a plain GPIO at the requested level. For the actual
 * generator gets used in between (1..period-1 high ticks) compensate
 * the +1 offset so the measured duty matches the input value.
 */
void setPWM(uint8_t /*pin*/, uint32_t value)
{
    uint16_t high = value_to_ticks(value);

    if (high == 0) {
        if (pwm_running_) {
            TM41_Channel_Stop((tm4_channel_t)(TM4_CHANNEL_0 | TM4_CHANNEL_1));
            pwm_running_ = false;
        }
        p15_drive_low();
        return;
    }

    if (high >= TM41_PWM_PERIOD_TICKS) {
        if (pwm_running_) {
            TM41_Channel_Stop((tm4_channel_t)(TM4_CHANNEL_0 | TM4_CHANNEL_1));
            pwm_running_ = false;
        }
        p15_drive_high();
        return;
    }

    if (!pwm_running_) {
        TM41_PWM_1Period_1Duty(TM41_PWM_PERIOD_TICKS, high - 1);
        /* TM41_PWM_1Period_1Duty re-runs its TO11_PORT_SETTING macro so
         * the alt-function routing is already correct here. */
        pwm_running_ = true;
    } else {
        TM41->TDR11 = high - 1;
        /* If we were just driving the pin manually (after a 0/full call),
         * re-route P15 back to TO11. */
        p15_route_pwm();
    }
}

void disablePWM(uint8_t /*pin*/)
{
    if (pwm_running_) {
        TM41_Channel_Stop((tm4_channel_t)(TM4_CHANNEL_0 | TM4_CHANNEL_1));
        pwm_running_ = false;
    }
    p15_drive_low();
}

} // namespace outputPWM
