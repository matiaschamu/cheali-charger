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

/* TM41 PWM period: 48 MHz / 1600 = 30 kHz, restored to the charger's normal
 * switching frequency for power-stage validation. Resolution = 1/1600 =
 * 0.0625 % per tick. */
static constexpr uint16_t TM41_PWM_PERIOD_TICKS = 1600;

/* External value scale exposed to SMPS_PID is 0..OUTPUT_PWM_PRECISION_PERIOD. */
static constexpr uint32_t VALUE_FULL_SCALE = OUTPUT_PWM_PRECISION_PERIOD;
static constexpr uint16_t TM41_PWM_CHANNEL_MASK =
        (uint16_t)(TM4_CHANNEL_0 | TM4_CHANNEL_1);

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
    PORT->P15CFG  = 0x00;         /* GPIO             */
    PORT->PMC1   &= ~(1u << 5);   /* digital          */
    PORT->PM1    &= ~(1u << 5);   /* output           */
    PORT->POM1   &= ~(1u << 5);   /* normal push-pull */
    if (high) PORT->P1 |=  (1u << 5);
    else      PORT->P1 &= ~(1u << 5);
}

static void p15_drive_low()  { p15_drive(0); }
static void p15_drive_high() { p15_drive(1); }

/* [MANUAL] Output functions can be assigned through PxxCFG; 0x02 selects
 * TO11. The board schematic connects the PWM driver to P15. */
static void p15_route_pwm()
{
    PORT->P15CFG  = 0x02;             /* allocate TO11 to P15 */
    PORT->P1     &= ~(1u << 5);       /* output low default   */
    PORT->PM1    &= ~(1u << 5);       /* output mode          */
    PORT->POM1   &= ~(1u << 5);       /* normal push-pull     */
    PORT->PMC1   &= ~(1u << 5);       /* digital              */
}

/* TS1/TT1 are command registers.  The manual specifies a direct write of the
 * channel bits; read-modify-write is not valid for these trigger registers. */
static void tm41_stop_pwm()
{
    TM41->TT1 = TM41_PWM_CHANNEL_MASK;
    pwm_running_ = false;
}

/* Configure both channels while stopped, then start master and slave with one
 * direct TS1 write.  TS1 must only be used when starting: writing it while the
 * timer is active reinitializes the counter and breaks PWM phase continuity. */
static void tm41_start_pwm(uint16_t high_ticks)
{
    CGC->PER0 |= CGC_PER0_TM41EN_Msk;
    TM41->TT1 = TM41_PWM_CHANNEL_MASK;

    TM41->TPS1 = _0000_TM4_CKM3_fCLK_8 |
                 _0000_TM4_CKM2_fCLK_1 |
                 _0000_TM4_CKM1_fCLK_0 |
                 _0000_TM4_CKM0_fCLK_0;

    TM41->TMR10 = _8000_TM4_CLOCK_SELECT_CKM1 |
                  _0000_TM4_TRIGGER_SOFTWARE |
                  _0001_TM4_MODE_PWM_MASTER;
    TM41->TDR10 = TM41_PWM_PERIOD_TICKS - 1;
    TM41->TO1  &= ~_0001_TM4_CH0_OUTPUT_VALUE_1;
    TM41->TOE1 &= ~_0001_TM4_CH0_OUTPUT_ENABLE;

    TM41->TMR11 = _8000_TM4_CLOCK_SELECT_CKM1 |
                  _0400_TM4_TRIGGER_MASTER_INT |
                  _0009_TM4_MODE_PWM_SLAVE;
    TM41->TDR11 = high_ticks;
    TM41->TOM1 |=  _0002_TM4_CH1_SLAVE_OUTPUT;
    TM41->TOL1 &= ~_0002_TM4_CH1_OUTPUT_LEVEL_L;
    TM41->TO1  &= ~_0002_TM4_CH1_OUTPUT_VALUE_1;
    TM41->TOE1 |=  _0002_TM4_CH1_OUTPUT_ENABLE;

    p15_route_pwm();
    TM41->TS1 = TM41_PWM_CHANNEL_MASK;
    pwm_running_ = true;
}

/* [MANUAL] TDRmn may be rewritten at any time.  In PWM slave mode TDR11 is
 * the data value used by channel 1 on the following master trigger, while the
 * running count is held separately in TCR11.  Updating only TDR11 therefore
 * changes duty without stopping or retriggering TM41. */
static void tm41_update_duty(uint16_t high_ticks)
{
    TM41->TDR11 = high_ticks;
}

void initialize(void)
{
    pwm_running_ = false;
    p15_drive_low();
}

/*
 * Set output PWM duty.
 *
 * [MANUAL] Duty is TDR11/(TDR10+1), with TDR11=0 documented as 0 %.
 * Keep the explicit GPIO handling at the endpoints so 0 % and 100 % cannot
 * produce transition spikes. Intermediate values use TDR11=high_ticks.
 */
void setPWM(uint8_t /*pin*/, uint32_t value)
{
    uint16_t high = value_to_ticks(value);

    if (high == 0) {
        if (pwm_running_) tm41_stop_pwm();
        p15_drive_low();
        return;
    }

    if (high >= TM41_PWM_PERIOD_TICKS) {
        if (pwm_running_) tm41_stop_pwm();
        p15_drive_high();
        return;
    }

    if (pwm_running_) tm41_update_duty(high);
    else              tm41_start_pwm(high);
}

void disablePWM(uint8_t /*pin*/)
{
    if (pwm_running_) tm41_stop_pwm();
    p15_drive_low();
}

} // namespace outputPWM
