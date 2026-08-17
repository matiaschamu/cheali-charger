/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

#include "AdcTest.h"
#include "AnalogInputs.h"
#include "Hardware.h"
#include "IO.h"
#include "Keyboard.h"
#include "LcdPrint.h"
#include "outputPWM.h"

namespace AdcTest {

struct Channel {
    AnalogInputs::Name name;
    const char *label;
};

/* VoutMux is not a physical CMS32L051 ADC channel on this board. */
static const Channel channels_[] = {
    {AnalogInputs::Vout_plus_pin,  "Vout+"},
    {AnalogInputs::Vout_minus_pin, "Vout-"},
    {AnalogInputs::Ismps,          "Ismps"},
    {AnalogInputs::Idischarge,     "Idisch"},
    {AnalogInputs::Tintern,        "Tint"},
    {AnalogInputs::Vin,            "Vin"},
    {AnalogInputs::Textern,        "Text"},
    {AnalogInputs::Vb0_pin,        "Vb0"},
    {AnalogInputs::Vb1_pin,        "Vb1"},
    {AnalogInputs::Vb2_pin,        "Vb2"},
    {AnalogInputs::Vb3_pin,        "Vb3"},
    {AnalogInputs::Vb4_pin,        "Vb4"},
    {AnalogInputs::Vb5_pin,        "Vb5"},
    {AnalogInputs::Vb6_pin,        "Vb6"},
};

static uint8_t index_;
static bool show_instant_;

static void force_safe_state()
{
    outputPWM::disablePWM(SMPS_VALUE_BUCK_PIN);
    hardware::setBalancer(0);
    hardware::setBatteryOutput(false);
}

static void draw()
{
    const Channel &channel = channels_[index_];

    lcdSetCursor0_0();
    lcdPrint("ADC ", 4);
    lcdPrintUnsigned(index_ + 1, 2);
    lcdPrint(" ", 1);
    uint8_t label_length = lcdPrint(channel.label, 9);
    lcdPrintSpaces(9 - label_length);

    lcdSetCursor0_1();
    lcdPrint(show_instant_ ? "raw: " : "avg: ", 5);
    uint16_t value = show_instant_
            ? AnalogInputs::getADCValue(channel.name)
            : AnalogInputs::getAvrADCValue(channel.name);
    lcdPrintUnsigned(value, 5);
    lcdPrintSpaces(6);
}

void run()
{
    index_ = 0;
    show_instant_ = false;

    force_safe_state();
    AnalogInputs::powerOn(false);

    /* The START press used to enter the menu may still be held. */
    while(Keyboard::getPressedWithDelay() != BUTTON_NONE) {}

    while(true) {
        draw();
        uint8_t key = Keyboard::getPressedWithDelay();

        if(key == BUTTON_STOP) break;
        if(key == BUTTON_INC) {
            index_++;
            if(index_ >= sizeof(channels_) / sizeof(channels_[0])) index_ = 0;
        } else if(key == BUTTON_DEC) {
            if(index_ == 0) index_ = sizeof(channels_) / sizeof(channels_[0]);
            index_--;
        } else if(key == BUTTON_START) {
            show_instant_ = !show_instant_;
            while(Keyboard::getPressedWithDelay() != BUTTON_NONE) {}
        }
    }

    AnalogInputs::powerOff();
    force_safe_state();
}

} // namespace AdcTest
