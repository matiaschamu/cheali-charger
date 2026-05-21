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

/* TM41 PWM period in chip ticks. 48 MHz / 1600 = 30 kHz, close enough to
 * the Nuvoton port's 32 kHz to keep SMPS_PID gains roughly compatible. */
static constexpr uint16_t TM41_PWM_PERIOD_TICKS = 1600;

/* External value scale exposed to SMPS_PID is 0..OUTPUT_PWM_PRECISION_PERIOD. */
static constexpr uint32_t VALUE_FULL_SCALE = OUTPUT_PWM_PRECISION_PERIOD;

static bool pwm_running_ = false;

static uint16_t value_to_ticks(uint32_t value)
{
    if (value >= VALUE_FULL_SCALE) return TM41_PWM_PERIOD_TICKS;
    return (uint16_t)((value * TM41_PWM_PERIOD_TICKS) / VALUE_FULL_SCALE);
}

static void p15_drive_low()
{
    PORT->PMC1   &= ~(1u << 5);   /* digital     */
    PORT->P1     &= ~(1u << 5);   /* value 0     */
    PORT->PM1    &= ~(1u << 5);   /* output      */
    PORT->P15CFG  = 0x00;         /* alt fn GPIO */
}

void initialize(void)
{
    pwm_running_ = false;
    p15_drive_low();
}

void setPWM(uint8_t /*pin*/, uint32_t value)
{
    if (!pwm_running_) {
        TM41_PWM_1Period_1Duty(TM41_PWM_PERIOD_TICKS, 0);
        pwm_running_ = true;
    }
    TM41->TDR11 = value_to_ticks(value);
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
