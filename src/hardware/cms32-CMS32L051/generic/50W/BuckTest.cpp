/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

#include "BuckTest.h"
#include "imaxB6.h"            /* hardware::setTopology / setChargerMode */
#include "imaxB6-pins.h"
#include "outputPWM.h"
#include "Hardware.h"          /* hardware::setBatteryOutput            */
#include "IO.h"
#include "Keyboard.h"
#include "LcdPrint.h"
#include "LiquidCrystal.h"

extern "C" {
#include "CMS32L051.h"
}

namespace BuckTest {

enum Mode : uint8_t { MODE_BUCK = 0, MODE_BOOST = 1 };

static Mode    mode_     = MODE_BUCK;
static uint8_t duty_pct_ = 0;     /* 0..100                   */
static bool    enabled_  = false; /* true when PWM is running */

/* ------------------------------------------------------------------
 * PWM control
 *
 * outputPWM expects a value scaled 0..OUTPUT_PWM_PRECISION_PERIOD.
 * Convert from a 0..100 % display value at every change so the screen
 * reads naturally regardless of how the underlying tick range moves.
 * ------------------------------------------------------------------ */

static uint32_t duty_pct_to_value(uint8_t pct)
{
    if (pct >= 100) return OUTPUT_PWM_PRECISION_PERIOD;
    return ((uint32_t)pct * OUTPUT_PWM_PRECISION_PERIOD) / 100u;
}

static void pwm_apply()
{
    outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, duty_pct_to_value(duty_pct_));
}

static void pwm_stop()
{
    outputPWM::disablePWM(SMPS_VALUE_BUCK_PIN);
}

/* Force everything back to a safe idle state. Output disconnected from
 * the battery, PWM stopped, duty zeroed. Topology pin keeps its current
 * value so we don't glitch the selector while flipping modes. */
static void enter_idle()
{
    enabled_  = false;
    duty_pct_ = 0;
    pwm_stop();
    hardware::setBatteryOutput(false);
}

/* Switch buck ↔ boost: always disables output and resets duty. */
static void toggle_mode()
{
    enter_idle();
    mode_ = (mode_ == MODE_BUCK) ? MODE_BOOST : MODE_BUCK;
    hardware::setTopology(mode_ == MODE_BOOST);
}

/* ------------------------------------------------------------------
 * Screen
 * ------------------------------------------------------------------ */

static void draw()
{
    lcdSetCursor0_0();
    lcdPrint(mode_ == MODE_BUCK ? "buck P " : "boost P", 7);
    lcdPrintUnsigned(TM41->TDR10, 4);
    lcdPrintSpaces(5);

    lcdSetCursor0_1();
    lcdPrint(enabled_ ? "ON d " : "off d", 5);
    lcdPrintUnsigned(duty_pct_, 3);
    lcdPrint(" T", 2);
    lcdPrintUnsigned(TM41->TDR11, 4);
    lcdPrintSpaces(2);
}

/* ------------------------------------------------------------------
 * Main loop
 *
 *  STOP          : exit
 *  START         : toggle output ON/OFF
 *  INC/DEC (ON)  : change duty (with key-speed auto-repeat)
 *  INC/DEC (OFF) : switch buck ↔ boost mode (fresh press only)
 * ------------------------------------------------------------------ */

void run()
{
    mode_ = MODE_BUCK;
    hardware::setTopology(false);          /* P21 LOW = buck */
    hardware::setChargerMode(true);        /* P20 HIGH = charger path */
    enter_idle();
    draw();

    uint8_t prev_key = BUTTON_NONE;

    while (true) {
        uint8_t key = Keyboard::getPressedWithDelay();
        bool fresh  = (key != BUTTON_NONE) && (key != prev_key);
        prev_key    = key;

        if (key == BUTTON_NONE) {
            continue;
        }

        if (key == BUTTON_STOP && fresh) {
            break;
        }

        if (key == BUTTON_START && fresh) {
            enabled_ = !enabled_;
            if (enabled_) {
                /* setBatteryOutput(false) on the previous OFF call goes
                 * through setChargerOutput(false) → disableChargerBuck(),
                 * which clears P21 to LOW. Re-assert topology and charger
                 * mode here so they stick across ON/OFF toggles. */
                hardware::setTopology(mode_ == MODE_BOOST);
                hardware::setChargerMode(true);
                hardware::setBatteryOutput(true);
                pwm_apply();
            } else {
                pwm_stop();
                hardware::setBatteryOutput(false);
            }
        }

        if (key == BUTTON_INC || key == BUTTON_DEC) {
            if (!enabled_) {
                /* Output off: Inc/Dec switches converter mode. No auto-
                 * repeat — only react on transition. */
                if (fresh) {
                    toggle_mode();
                }
            } else {
                uint8_t step = Keyboard::getSpeedFactor();
                if (key == BUTTON_INC) {
                    uint16_t n = (uint16_t)duty_pct_ + step;
                    duty_pct_  = (n > 100) ? 100 : (uint8_t)n;
                } else {
                    duty_pct_  = (step > duty_pct_) ? 0 : (duty_pct_ - step);
                }
                pwm_apply();
            }
        }

        draw();
    }

    /* Cleanup on exit — SMPS off, mode pin LOW (buck), charger path. */
    enter_idle();
    mode_ = MODE_BUCK;
    hardware::setTopology(false);
    hardware::setChargerMode(true);
}

} // namespace BuckTest
