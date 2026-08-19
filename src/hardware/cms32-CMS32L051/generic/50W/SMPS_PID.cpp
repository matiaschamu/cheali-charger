#include "Hardware.h"
#include "SMPS_PID.h"
#include "imaxB6.h"
#include "IO.h"
#include "AnalogInputs.h"
#include "AnalogInputsADC.h"
#include "outputPWM.h"
#include "atomic.h"
#include "Monitor.h"

namespace {
    volatile uint16_t i_PID_setpoint;
    //we have to use i_PID_CutOffVoltage, on some chargers (M0516) ADC can read up to 60V
    volatile uint16_t i_PID_CutOffVoltage;
    volatile long i_PID_MV;
    volatile bool i_PID_enable;

    /* Temporary bring-up telemetry. These counters distinguish an ADC path
     * that never calls the PID from one that calls it while disabled. */
    volatile uint16_t i_PID_update_calls;
    volatile uint16_t i_PID_active_updates;
    volatile uint16_t i_PID_last_feedback;
    volatile uint16_t i_PID_last_vout;
    volatile bool i_PID_cutoff_tripped;

    /* This board has one physical power PWM: TM41/TO11 on P15.  The `pin`
     * parameter remains in outputPWM only for API compatibility with the
     * multi-channel AVR/Nuvoton ports. */
    static constexpr uint8_t POWER_PWM_PIN = CMS32_PIN(1, 5);
    bool power_topology_boost = false;

    void stopPowerPWM()
    {
        outputPWM::disablePWM(POWER_PWM_PIN);
    }

    void setPowerPWM(uint16_t value)
    {
        outputPWM::setPWM(POWER_PWM_PIN, value);
    }

    /* P21 must only change while P15 is inactive.  Always write P21 so a
     * previous boost state cannot leak into the following buck update. */
    void selectPowerTopology(bool boost)
    {
        if (power_topology_boost != boost) {
            stopPowerPWM();
        }
        hardware::setTopology(boost);
        power_topology_boost = boost;
    }

    void forcePowerIdle()
    {
        stopPowerPWM();
        hardware::setTopology(false);   /* P21 LOW = buck/safe idle */
        power_topology_boost = false;
    }
}

#define A 4

uint16_t hardware::getPIDValue()
{
    uint16_t v;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        v = i_PID_MV>>PID_MV_PRECISION;
    }
    return v;
}


void SMPS_PID::update()
{
    i_PID_update_calls++;
    if(!i_PID_enable) return;

    i_PID_active_updates++;
    i_PID_last_vout = AnalogInputsADC::getFastADCValue(AnalogInputs::Vout_plus_pin);
    //if Vout is too high disable PID
    if(i_PID_last_vout >= i_PID_CutOffVoltage) {
        i_PID_cutoff_tripped = true;
        hardware::setChargerOutput(false);
        i_PID_enable = false;
        Monitor::i_externalError = MONITOR_EXTERNAL_ERROR_BATTERY_DISCONNECTED;
        return;
    }

    //TODO: rewrite PID
    //this is the PID - actually it is an I (Integral part) - should be rewritten
    uint16_t PV = AnalogInputsADC::getFastADCValue(AnalogInputs::Ismps);
    i_PID_last_feedback = PV;
    long error = i_PID_setpoint;
    error -= PV;
    i_PID_MV += error*A;

    if(i_PID_MV<0) i_PID_MV = 0;
    if((uint32_t)i_PID_MV > MAX_PID_MV_PRECISION) {
        i_PID_MV = MAX_PID_MV_PRECISION;
    }

    SMPS_PID::setPID_MV(i_PID_MV>>PID_MV_PRECISION);
}

void SMPS_PID::init(uint16_t Vin, uint16_t Vout)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        i_PID_setpoint = 0;
        i_PID_update_calls = 0;
        i_PID_active_updates = 0;
        i_PID_last_feedback = 0;
        i_PID_last_vout = 0;
        i_PID_cutoff_tripped = false;
        if(Vout>Vin) {
            i_PID_MV = OUTPUT_PWM_PRECISION_PERIOD;
        } else {
            i_PID_MV = 0;
        }
        i_PID_MV <<= PID_MV_PRECISION;
        i_PID_enable = true;
    }

}

void SMPS_PID::getDebugState(DebugState &state)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        state.updateCalls = i_PID_update_calls;
        state.activeUpdates = i_PID_active_updates;
        state.setpoint = i_PID_setpoint;
        state.feedback = i_PID_last_feedback;
        state.output = (uint16_t)(i_PID_MV >> PID_MV_PRECISION);
        state.vout = i_PID_last_vout;
        state.cutoff = i_PID_CutOffVoltage;
        state.enabled = i_PID_enable;
        state.cutoffTripped = i_PID_cutoff_tripped;
    }
}

void SMPS_PID::setPID_MV(uint16_t value) {
    if(value > MAX_PID_MV)
        value = MAX_PID_MV;

    if(value <= OUTPUT_PWM_PRECISION_PERIOD) {
        selectPowerTopology(false);     /* P21 LOW = buck */
        setPowerPWM(value);             /* P15 = buck duty */
    } else {
        selectPowerTopology(true);      /* P21 HIGH = boost */
        uint16_t v2 = value - OUTPUT_PWM_PRECISION_PERIOD;
        setPowerPWM(v2);                /* P15 = boost duty */
    }
}

void hardware::setVoutCutoff(AnalogInputs::ValueType v) {
    if(v > MAX_CHARGE_V) {
        v = MAX_CHARGE_V;
    }
    AnalogInputs::ValueType cutOff = AnalogInputs::reverseCalibrateValue(AnalogInputs::Vout_plus_pin, v);
    if(cutOff > ANALOG_INPUTS_MAX_ADC_Vout_plus_pin) {
        //extra limit if calibration is wrong
        cutOff = ANALOG_INPUTS_MAX_ADC_Vout_plus_pin;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        i_PID_CutOffVoltage = cutOff;
    }
}

void hardware::setChargerValue(uint16_t value)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        i_PID_setpoint = value;
    }

//  TODO: test without PID
//  SMPS_PID::setPID_MV(value);
//  outputPWM::setPWM(SMPS_VALUE_BUCK_PIN, value);
}

void hardware::setChargerOutput(bool enable)
{
    if(enable) setDischargerOutput(false);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        i_PID_enable = false;
        forcePowerIdle();
    }
    /* P20 HIGH keeps the discharge path blocked both while charging and in
     * idle.  P00 remains independently controlled by setBatteryOutput(). */
    setChargerMode(true);
    if(enable) {
        SMPS_PID::init(AnalogInputs::getRealValue(AnalogInputs::Vin), AnalogInputs::getRealValue(AnalogInputs::Vout_plus_pin));
    }
}


void hardware::setDischargerOutput(bool enable)
{
    if(enable) {
        setChargerOutput(false);
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            forcePowerIdle();
        }
        setChargerMode(false);           /* P20 LOW = discharge path */
    } else {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            forcePowerIdle();
        }
        setChargerMode(true);            /* P20 HIGH = discharge blocked */
    }
}

void hardware::setDischargerValue(uint16_t value)
{
    selectPowerTopology(false);          /* discharge uses the non-boost path */
    setPowerPWM(value);                  /* shared P15 PWM */
}

