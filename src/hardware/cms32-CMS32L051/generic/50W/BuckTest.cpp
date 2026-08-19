/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

#include "BuckTest.h"
#include "AnalogInputs.h"
#include "Hardware.h"
#include "IO.h"
#include "Keyboard.h"
#include "LcdPrint.h"
#include "Menu.h"
#include "imaxB6.h"
#include "imaxB6-pins.h"
#include "memory.h"
#include "outputPWM.h"
#include "SMPS_PID.h"

extern "C" {
#include "CMS32L051.h"
}

namespace BuckTest {
namespace {

/* Laboratory limits. Boost follows the production PID limit (50 %).
 * Discharge remains deliberately lower until U9 and Idischarge are measured. */
static constexpr uint8_t BOOST_MAX_DUTY_PCT = 50;
static constexpr uint8_t DISCHARGE_TEST_MAX_DUTY_PCT = 20;
static constexpr uint8_t COMBINED_MAX_STEP = 100 + BOOST_MAX_DUTY_PCT;

enum PowerPath : uint8_t { PATH_CHARGE = 0, PATH_DISCHARGE = 1 };

static void runPwmManual();
static void runBuckBoost();
static void runPowerAdc();
static void runChargeDischarge();
static void runOutputCutoff();
static void runBalancers();
static void runPidDebug();

static const char string_pwm_manual[] PROGMEM = "PWM manual";
static const char string_buck_boost[] PROGMEM = "buck/boost";
static const char string_power_adc[] PROGMEM = "power ADC";
static const char string_charge_discharge[] PROGMEM = "charge/disch";
static const char string_output_cutoff[] PROGMEM = "output P00";
static const char string_balancers[] PROGMEM = "balancers";
static const char string_pid_debug[] PROGMEM = "PID debug";

static const Menu::StaticMenu test_menu[] PROGMEM = {
    {string_pwm_manual,       runPwmManual},
    {string_buck_boost,       runBuckBoost},
    {string_power_adc,        runPowerAdc},
    {string_charge_discharge, runChargeDischarge},
    {string_output_cutoff,    runOutputCutoff},
    {string_balancers,        runBalancers},
    {string_pid_debug,        runPidDebug},
    {NULL, NULL}
};

static uint32_t dutyPctToValue(uint8_t pct)
{
    if (pct >= 100) return OUTPUT_PWM_PRECISION_PERIOD;
    return ((uint32_t)pct * OUTPUT_PWM_PRECISION_PERIOD) / 100u;
}

static void waitButtonsReleased()
{
    while (Keyboard::getPressedWithDelay() != BUTTON_NONE) {}
}

/* Every test enters and exits through this state:
 * P15 LOW, P21 LOW, P20 HIGH, P00 HIGH and all balancers off. */
static void forceSafeState()
{
    outputPWM::disablePWM(SMPS_VALUE_BUCK_PIN); /* P15 LOW first */
    hardware::setBalancer(0);
    hardware::setBatteryOutput(false);          /* P00 HIGH       */
    hardware::setTopology(false);               /* P21 LOW        */
    hardware::setChargerMode(true);             /* P20 HIGH       */
}

static void armPower(bool boost, PowerPath path)
{
    outputPWM::disablePWM(SMPS_VALUE_BUCK_PIN);
    hardware::setTopology(boost);
    hardware::setChargerMode(path == PATH_CHARGE);
    hardware::setBatteryOutput(true);            /* P00 LOW */
}

static void drawTimerRow()
{
    lcdSetCursor0_1();
    lcdPrint("P", 1);
    lcdPrintUnsigned(TM41->TDR10, 4);
    lcdPrint(" T", 2);
    lcdPrintUnsigned(TM41->TDR11, 4);
    lcdPrintSpaces(5);
}

static uint8_t increasePct(uint8_t value, uint8_t maximum)
{
    uint16_t next = (uint16_t)value + Keyboard::getSpeedFactor();
    return next > maximum ? maximum : (uint8_t)next;
}

static uint8_t decreasePct(uint8_t value)
{
    uint8_t step = Keyboard::getSpeedFactor();
    return step > value ? 0 : (uint8_t)(value - step);
}

/* Fixed buck topology, used to inspect P15 and the Q9 driver. */
static void drawPwmManual(bool enabled, uint8_t duty)
{
    lcdSetCursor0_0();
    lcdPrint(enabled ? "PWM B ON  d" : "PWM B off d", 11);
    lcdPrintUnsigned(duty, 3);
    lcdPrintSpaces(2);
    drawTimerRow();
}

static void runPwmManual()
{
    bool enabled = false;
    uint8_t duty = 0;
    uint8_t previous_key = BUTTON_NONE;

    forceSafeState();
    waitButtonsReleased();

    while (true) {
        drawPwmManual(enabled, duty);
        uint8_t key = Keyboard::getPressedWithDelay();
        bool fresh = key != BUTTON_NONE && key != previous_key;
        previous_key = key;

        if (key == BUTTON_STOP && fresh) break;
        if (key == BUTTON_START && fresh) {
            enabled = !enabled;
            duty = 0;
            if (enabled) {
                armPower(false, PATH_CHARGE);
                outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, 0);
            } else {
                forceSafeState();
            }
        } else if (enabled && key == BUTTON_INC) {
            duty = increasePct(duty, 100);
            outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, dutyPctToValue(duty));
        } else if (enabled && key == BUTTON_DEC) {
            duty = decreasePct(duty);
            outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, dutyPctToValue(duty));
        }
    }

    forceSafeState();
}

/* Combined production command:
 *   step 0..100   -> buck  0..100 %
 *   step 101..150 -> boost 1..50 %
 * Crossing the boundary preserves P00 LOW, but forces P15 LOW before P21. */
static bool combinedIsBoost(uint8_t step)
{
    return step > 100;
}

static uint8_t combinedDuty(uint8_t step)
{
    return combinedIsBoost(step) ? (uint8_t)(step - 100) : step;
}

static uint8_t increaseCombined(uint8_t step)
{
    if (step == 100) return 101; /* B100 -> G001 exactly */
    if (step < 100) {
        uint16_t next = (uint16_t)step + Keyboard::getSpeedFactor();
        return next > 100 ? 100 : (uint8_t)next;
    }
    return increasePct(step, COMBINED_MAX_STEP);
}

static uint8_t decreaseCombined(uint8_t step)
{
    if (step == 101) return 100; /* G001 -> B100 exactly */
    if (step > 101) {
        uint8_t amount = Keyboard::getSpeedFactor();
        return amount >= step - 100 ? 101 : (uint8_t)(step - amount);
    }
    return decreasePct(step);
}

static void applyCombined(uint8_t step, bool &boost_selected)
{
    bool boost = combinedIsBoost(step);
    if (boost != boost_selected) {
        outputPWM::disablePWM(SMPS_VALUE_BUCK_PIN); /* P15 LOW */
        hardware::setTopology(boost);               /* switch P21 */
        boost_selected = boost;
    }
    outputPWM::setPWM(SMPS_VALUE_BUCK_PIN,
                      dutyPctToValue(combinedDuty(step)));
}

static void drawCombined(const char *label, bool enabled, uint8_t step,
                         bool timer_row)
{
    lcdSetCursor0_0();
    lcdPrint(label, 4);
    lcdPrint(combinedIsBoost(step) ? " G" : " B", 2);
    lcdPrintUnsigned(combinedDuty(step), 3);
    lcdPrint(enabled ? " ON" : "off", 3);
    lcdPrintSpaces(4);

    if (timer_row) drawTimerRow();
}

static void runBuckBoost()
{
    bool enabled = false;
    bool boost_selected = false;
    uint8_t step = 0;
    uint8_t previous_key = BUTTON_NONE;

    forceSafeState();
    waitButtonsReleased();

    while (true) {
        drawCombined("B/G ", enabled, step, true);
        uint8_t key = Keyboard::getPressedWithDelay();
        bool fresh = key != BUTTON_NONE && key != previous_key;
        previous_key = key;

        if (key == BUTTON_STOP && fresh) break;
        if (key == BUTTON_START && fresh) {
            enabled = !enabled;
            step = 0;
            boost_selected = false;
            if (enabled) {
                armPower(false, PATH_CHARGE);
                outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, 0);
            } else {
                forceSafeState();
            }
        } else if (enabled && key == BUTTON_INC) {
            step = increaseCombined(step);
            applyCombined(step, boost_selected);
        } else if (enabled && key == BUTTON_DEC) {
            step = decreaseCombined(step);
            applyCombined(step, boost_selected);
        }
    }

    forceSafeState();
}

struct PowerAdcChannel {
    AnalogInputs::Name name;
    const char *label;
};

static const PowerAdcChannel power_adc_channels[] = {
    {AnalogInputs::Vout_plus_pin,  "Vo+"},
    {AnalogInputs::Vout_minus_pin, "Vo-"},
    {AnalogInputs::Ismps,          "Ism"},
    {AnalogInputs::Idischarge,     "Ids"},
    {AnalogInputs::Vin,            "Vin"},
};

static void drawPowerAdc(bool enabled, uint8_t step, uint8_t channel)
{
    drawCombined("ADC ", enabled, step, false);
    lcdSetCursor0_1();
    lcdPrint(power_adc_channels[channel].label, 3);
    lcdPrint(" raw ", 5);
    lcdPrintUnsigned(AnalogInputs::getADCValue(power_adc_channels[channel].name), 5);
    lcdPrintSpaces(3);
}

/* Same B/G command with a live raw channel. START arms, then cycles ADC. */
static void runPowerAdc()
{
    bool enabled = false;
    bool boost_selected = false;
    uint8_t step = 0;
    uint8_t channel = 0;
    uint8_t previous_key = BUTTON_NONE;

    forceSafeState();
    AnalogInputs::powerOn(false);
    waitButtonsReleased();

    while (true) {
        drawPowerAdc(enabled, step, channel);
        uint8_t key = Keyboard::getPressedWithDelay();
        bool fresh = key != BUTTON_NONE && key != previous_key;
        previous_key = key;

        if (key == BUTTON_STOP && fresh) break;
        if (key == BUTTON_START && fresh) {
            if (!enabled) {
                enabled = true;
                armPower(false, PATH_CHARGE);
                outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, 0);
            } else {
                channel++;
                if (channel >= sizeof(power_adc_channels) /
                               sizeof(power_adc_channels[0])) channel = 0;
            }
        } else if (enabled && key == BUTTON_INC) {
            step = increaseCombined(step);
            applyCombined(step, boost_selected);
        } else if (enabled && key == BUTTON_DEC) {
            step = decreaseCombined(step);
            applyCombined(step, boost_selected);
        }
    }

    forceSafeState();
    AnalogInputs::powerOff();
}

static void drawChargeDischarge(PowerPath path, bool enabled, uint8_t duty)
{
    lcdSetCursor0_0();
    lcdPrint(path == PATH_CHARGE ? "CHG " : "DIS ", 4);
    lcdPrint(enabled ? "ON  d" : "off d", 5);
    lcdPrintUnsigned(duty, 3);
    lcdPrintSpaces(4);

    lcdSetCursor0_1();
    lcdPrint(path == PATH_CHARGE ? "Ism raw " : "Ids raw ", 8);
    AnalogInputs::Name name = path == PATH_CHARGE
            ? AnalogInputs::Ismps : AnalogInputs::Idischarge;
    lcdPrintUnsigned(AnalogInputs::getADCValue(name), 5);
    lcdPrintSpaces(3);
}

/* Select P20 path while OFF, START to arm at 0 %, then adjust P15. */
static void runChargeDischarge()
{
    PowerPath path = PATH_CHARGE;
    bool enabled = false;
    uint8_t duty = 0;
    uint8_t previous_key = BUTTON_NONE;

    forceSafeState();
    AnalogInputs::powerOn(false);
    waitButtonsReleased();

    while (true) {
        drawChargeDischarge(path, enabled, duty);
        uint8_t key = Keyboard::getPressedWithDelay();
        bool fresh = key != BUTTON_NONE && key != previous_key;
        previous_key = key;

        if (key == BUTTON_STOP && fresh) break;
        if (key == BUTTON_START && fresh) {
            enabled = !enabled;
            duty = 0;
            if (enabled) {
                armPower(false, path);
                outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, 0);
            } else {
                forceSafeState();
            }
        } else if (!enabled && fresh &&
                   (key == BUTTON_INC || key == BUTTON_DEC)) {
            path = path == PATH_CHARGE ? PATH_DISCHARGE : PATH_CHARGE;
        } else if (enabled && key == BUTTON_INC) {
            uint8_t maximum = path == PATH_CHARGE
                    ? 100 : DISCHARGE_TEST_MAX_DUTY_PCT;
            duty = increasePct(duty, maximum);
            outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, dutyPctToValue(duty));
        } else if (enabled && key == BUTTON_DEC) {
            duty = decreasePct(duty);
            outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, dutyPctToValue(duty));
        }
    }

    forceSafeState();
    AnalogInputs::powerOff();
}

static void drawOutputCutoff(bool connected)
{
    lcdSetCursor0_0();
    lcdPrint(connected ? "OUTPUT connected" : "OUTPUT cut      ", 16);
    lcdSetCursor0_1();
    lcdPrint("P00 pin ", 8);
    lcdPrintUnsigned(IO::digitalRead(OUTPUT_DISABLE_PIN), 1);
    lcdPrint(connected ? " LOW" : " HIGH", 5);
    lcdPrintSpaces(2);
}

/* P00 test with P15 permanently LOW. START toggles the normal output API. */
static void runOutputCutoff()
{
    bool connected = false;
    uint8_t previous_key = BUTTON_NONE;

    forceSafeState();
    waitButtonsReleased();

    while (true) {
        drawOutputCutoff(connected);
        uint8_t key = Keyboard::getPressedWithDelay();
        bool fresh = key != BUTTON_NONE && key != previous_key;
        previous_key = key;

        if (key == BUTTON_STOP && fresh) break;
        if (key == BUTTON_START && fresh) {
            connected = !connected;
            outputPWM::disablePWM(SMPS_VALUE_BUCK_PIN);
            hardware::setTopology(false);
            hardware::setChargerMode(true);
            hardware::setBatteryOutput(connected);
        }
    }

    forceSafeState();
}

static void drawPidDebug(bool enabled, uint8_t page)
{
    SMPS_PID::DebugState state;
    SMPS_PID::getDebugState(state);

    lcdSetCursor0_0();
    lcdPrint(enabled ? "PID diag ON " : "PID diag off", 12);
    lcdPrint("p", 1);
    lcdPrintUnsigned(page, 1);
    lcdPrintSpaces(2);

    lcdSetCursor0_1();
    if(page == 0) {
        lcdPrint("U", 1);
        lcdPrintUnsigned(state.updateCalls, 5);
        lcdPrint(" A", 2);
        lcdPrintUnsigned(state.activeUpdates, 5);
        lcdPrintSpaces(3);
    } else if(page == 1) {
        lcdPrint(state.enabled ? "E1" : "E0", 2);
        lcdPrint(state.cutoffTripped ? " X1 " : " X0 ", 4);
        lcdPrint("S", 1);
        lcdPrintUnsigned(state.setpoint, 5);
        lcdPrintSpaces(4);
    } else if(page == 2) {
        lcdPrint("M", 1);
        lcdPrintUnsigned(state.output, 5);
        lcdPrint(" I", 2);
        lcdPrintUnsigned(state.feedback, 5);
        lcdPrintSpaces(3);
    } else {
        lcdPrint("V", 1);
        lcdPrintUnsigned(state.vout, 5);
        lcdPrint(" C", 2);
        lcdPrintUnsigned(state.cutoff, 5);
        lcdPrintSpaces(3);
    }
}

/* Inspect the last PID state and, when START is pressed, enable only the
 * internal zero-setpoint callback with P00 cut and P15 initially low. No key
 * in this screen can connect the output or increase duty: INC/DEC only select
 * telemetry pages. */
static void runPidDebug()
{
    bool enabled = false;
    uint8_t page = 0;
    uint8_t previous_key = BUTTON_NONE;

    forceSafeState();
    AnalogInputs::powerOn(false);
    waitButtonsReleased();

    while(true) {
        drawPidDebug(enabled, page);
        uint8_t key = Keyboard::getPressedWithDelay();
        bool fresh = key != BUTTON_NONE && key != previous_key;
        previous_key = key;

        if(key == BUTTON_STOP && fresh) break;
        if(key == BUTTON_START && fresh) {
            enabled = !enabled;
            if(enabled) {
                hardware::setVoutCutoff(MAX_CHARGE_V);
                SMPS_PID::init(0, 0);
            } else {
                hardware::setChargerOutput(false);
                forceSafeState();
            }
        } else if(fresh && key == BUTTON_INC) {
            page = (uint8_t)((page + 1) & 3u);
        } else if(fresh && key == BUTTON_DEC) {
            page = page == 0 ? 3 : (uint8_t)(page - 1);
        }
    }

    hardware::setChargerOutput(false);
    AnalogInputs::powerOff();
    forceSafeState();
}

static const AnalogInputs::Name balance_adc_channels[] = {
    AnalogInputs::Vb1_pin,
    AnalogInputs::Vb2_pin,
    AnalogInputs::Vb3_pin,
    AnalogInputs::Vb4_pin,
    AnalogInputs::Vb5_pin,
    AnalogInputs::Vb6_pin,
};

static void applyBalancer(uint8_t index, bool enabled)
{
    hardware::setBalancer(enabled ? (uint8_t)(1u << index) : 0);
}

static void drawBalancer(uint8_t index, bool enabled)
{
    lcdSetCursor0_0();
    lcdPrint("BAL ", 4);
    lcdPrintUnsigned(index + 1, 1);
    lcdPrint(enabled ? " ON " : " off", 4);
    lcdPrintSpaces(7);

    lcdSetCursor0_1();
    lcdPrint("Vb", 2);
    lcdPrintUnsigned(index + 1, 1);
    lcdPrint(" raw ", 5);
    lcdPrintUnsigned(AnalogInputs::getADCValue(balance_adc_channels[index]), 5);
    lcdPrintSpaces(3);
}

/* Only one balance output can be active. START toggles the selected output. */
static void runBalancers()
{
    uint8_t index = 0;
    bool enabled = false;
    uint8_t previous_key = BUTTON_NONE;

    forceSafeState();
    AnalogInputs::powerOn(false);
    waitButtonsReleased();

    while (true) {
        drawBalancer(index, enabled);
        uint8_t key = Keyboard::getPressedWithDelay();
        bool fresh = key != BUTTON_NONE && key != previous_key;
        previous_key = key;

        if (key == BUTTON_STOP && fresh) break;
        if (key == BUTTON_START && fresh) {
            enabled = !enabled;
            applyBalancer(index, enabled);
        } else if (fresh && key == BUTTON_INC) {
            hardware::setBalancer(0);
            index = (uint8_t)((index + 1) % 6);
            applyBalancer(index, enabled);
        } else if (fresh && key == BUTTON_DEC) {
            hardware::setBalancer(0);
            index = index == 0 ? 5 : (uint8_t)(index - 1);
            applyBalancer(index, enabled);
        }
    }

    hardware::setBalancer(0);
    AnalogInputs::powerOff();
    forceSafeState();
}

} // namespace

void run()
{
    forceSafeState();
    Menu::runStatic(test_menu);
    forceSafeState();
}

} // namespace BuckTest
