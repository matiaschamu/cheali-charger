/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

/*
 * CMS32L051 ADC driver.
 *
 * The imaxB6-80W board has *no* analog multiplexer — every balancer cell
 * voltage and every charger sense pin goes straight to a dedicated ANIxx
 * input, so we just walk an input list one channel at a time in software.
 *
 * Conversion loop (driven entirely from the ADC interrupt):
 *   1. ADC IRQ fires after each conversion completes.
 *   2. The first sample of every burst is discarded (input mux settling).
 *   3. The next BURST_COUNT samples are summed into burst_sum_.
 *   4. The burst average feeds the fast CMS power-control path. In parallel,
 *      every raw 12-bit conversion is accumulated in a per-channel block of
 *      256 samples. When that block completes, i_adc_[] receives a genuine
 *      16-bit-resolution oversampled result.
 *   5. The burst sum also lands in i_avrSum_[]; current_input_ advances to the
 *      next slot and the next conversion is started.
 *   6. After a full round (back to slot 0), finalizeMeasurement() pushes the
 *      SMPS/Discharger set-point readbacks and calls
 *      intterruptFinalizeMeasurement() so the core measurement loop wakes up.
 *
 * Ismps is sampled four times per round (ADC_I_SMPS_PER_ROUND) — the sum is
 * divided by four when the last measurement of the average window lands.
 *
 * ADCR stores the 12-bit result right-aligned in bits 11:0. For the slow raw
 * path, 256 samples are summed and shifted right by four: four additional
 * resolution bits are available when the input/noise supplies enough code
 * variation. This improves resolution, not absolute ADC accuracy.
 */

#include "atomic.h"
#include "Hardware.h"
#include "SMPS_PID.h"
#include "AnalogInputsADC.h"
#include "Utils.h"
#include "memory.h"
#include "Settings.h"
#include "AnalogInputsPrivate.h"
#include "IO.h"
#include "SMPS.h"
#include "Discharger.h"
#include "irq_priority.h"

extern "C" {
#include "CMS32L051.h"
#include "adc.h"
}


#define ADC_I_SMPS_PER_ROUND        4
#define ADC_INTERNAL_TEMP_CHANNEL   0x80U   /* ADS value for internal temp sensor */
#define ADC_BURST_DISCARD_SAMPLES   1       /* discard first sample after channel switch */
#define ADC_RESULT_MASK             0x0FFFU
#define ADC_RESULT_SHIFT            (ANALOG_INPUTS_RESOLUTION - ANALOG_INPUTS_ADC_RESOLUTION_BITS)
#define ADC_OVERSAMPLE_COUNT        256U    /* 4^(16 - 12) samples for +4 bits */
#define ADC_OVERSAMPLE_SHIFT        4U


namespace AnalogInputsADC {

struct adc_correlation {
    uint8_t adc_pin_;
    AnalogInputs::Name ai_name_;
    bool trigger_PID_;
};

/*
 * Sample order. Layout mirrors the Nuvoton-M051 driver (Ismps appears 4x per
 * round so the SMPS PID has tight control feedback), but every entry maps
 * directly to a pin since there is no analog mux on this board.
 */
static const adc_correlation order_analogInputs_on[] = {
    {BALANSER0_PIN,           AnalogInputs::Vb0_pin,         false},
    {OUTPUT_VOLTAGE_MINUS_PIN,AnalogInputs::Vout_minus_pin,  false},
    {BALANSER1_PIN,           AnalogInputs::Vb1_pin,         false},
    {SMPS_CURRENT_PIN,        AnalogInputs::Ismps,           true },
    {BALANSER2_PIN,           AnalogInputs::Vb2_pin,         false},
    {OUTPUT_VOLTAGE_PLUS_PIN, AnalogInputs::Vout_plus_pin,   false},
    {BALANSER6_PIN,           AnalogInputs::Vb6_pin,         false},
    {SMPS_CURRENT_PIN,        AnalogInputs::Ismps,           true },
    {BALANSER5_PIN,           AnalogInputs::Vb5_pin,         false},
    {DISCHARGE_CURRENT_PIN,   AnalogInputs::Idischarge,      false},
    {BALANSER4_PIN,           AnalogInputs::Vb4_pin,         false},
    {SMPS_CURRENT_PIN,        AnalogInputs::Ismps,           true },
    {BALANSER3_PIN,           AnalogInputs::Vb3_pin,         false},
    {V_IN_PIN,                AnalogInputs::Vin,             false},
    {T_EXTERNAL_PIN,          AnalogInputs::Textern,         false},
    {T_INTERNAL_PIN,          AnalogInputs::Tintern,         false},
    {SMPS_CURRENT_PIN,        AnalogInputs::Ismps,           true },
};

static const uint8_t kNumInputs = sizeof(order_analogInputs_on) / sizeof(order_analogInputs_on[0]);


static volatile uint8_t current_input_   = 0;
static volatile uint8_t burst_count_     = 0;
static volatile uint32_t burst_sum_      = 0;
static volatile uint8_t addSumToInput_   = 0;  /* latched at start of every round */

/* The core's i_adc_[] now exposes these completed 256-sample blocks. A
 * separate burst average keeps the PID/protection response at its old rate. */
static volatile uint32_t oversample_sum_[AnalogInputs::PHYSICAL_INPUTS];
static volatile uint16_t oversample_count_[AnalogInputs::PHYSICAL_INPUTS];
static volatile uint16_t fast_adc_[AnalogInputs::PHYSICAL_INPUTS];


static inline uint8_t nextInput(uint8_t i) {
    i++;
    if(i >= kNumInputs) i = 0;
    return i;
}

static void selectChannel(uint8_t pin)
{
    if(pin >= 128) {
        /* Virtual pins: only T_INTERNAL_PIN (3+128) is wired up — feeds the
         * on-chip temperature sensor through the dedicated ADC channel. */
        ADC->ADS = ADC_INTERNAL_TEMP_CHANNEL;
    } else {
        ADC->ADS = IO::getADCChannel(pin);
    }
}

static void startConversion(uint8_t input_idx)
{
    burst_count_ = 0;
    burst_sum_   = 0;
    selectChannel(order_analogInputs_on[input_idx].adc_pin_);
    ADC->ADM0 |= ADCS;   /* kick off conversion */
}

uint16_t getFastADCValue(uint8_t name)
{
    if(name >= AnalogInputs::PHYSICAL_INPUTS) return 0;

    uint16_t value;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        value = fast_adc_[name];
    }
    return value;
}

static void finalizeMeasurement()
{
    AnalogInputs::i_adc_[AnalogInputs::IsmpsSet]      = SMPS::getValue();
    AnalogInputs::i_adc_[AnalogInputs::IdischargeSet] = Discharger::getValue();

    if(addSumToInput_) {
        AnalogInputs::i_avrSum_[AnalogInputs::IsmpsSet]      += (uint32_t)SMPS::getValue()      * ANALOG_INPUTS_ADC_BURST_COUNT;
        AnalogInputs::i_avrSum_[AnalogInputs::IdischargeSet] += (uint32_t)Discharger::getValue()* ANALOG_INPUTS_ADC_BURST_COUNT;
        if(AnalogInputs::i_avrCount_ == 1) {
            /* Ismps is sampled four times per round; collapse to a single average. */
            AnalogInputs::i_avrSum_[AnalogInputs::Ismps] /= ADC_I_SMPS_PER_ROUND;
        }
        AnalogInputs::intterruptFinalizeMeasurement();
    }
}


void initialize()
{
    for(uint8_t i = 0; i < AnalogInputs::PHYSICAL_INPUTS; i++) {
        oversample_sum_[i] = 0;
        oversample_count_[i] = 0;
        fast_adc_[i] = 0;
    }

    /* Disable IRQ + ADC before reconfiguring. */
    NVIC_DisableIRQ((IRQn_Type)ADC_IRQn);
    CGC->PER0 |= CGC_PER0_ADCEN_Msk;            /* enable ADC peripheral clock */
    ADC->ADM0  = 0x00U;

    /* Pin functions */
    IO::pinMode(BALANSER0_PIN,            ANALOG_INPUT);
    IO::pinMode(BALANSER1_PIN,            ANALOG_INPUT);
    IO::pinMode(BALANSER2_PIN,            ANALOG_INPUT);
    IO::pinMode(BALANSER3_PIN,            ANALOG_INPUT);
    IO::pinMode(BALANSER4_PIN,            ANALOG_INPUT);
    IO::pinMode(BALANSER5_PIN,            ANALOG_INPUT);
    IO::pinMode(BALANSER6_PIN,            ANALOG_INPUT);
    IO::pinMode(OUTPUT_VOLTAGE_MINUS_PIN, ANALOG_INPUT);
    IO::pinMode(OUTPUT_VOLTAGE_PLUS_PIN,  ANALOG_INPUT);
    IO::pinMode(DISCHARGE_CURRENT_PIN,    ANALOG_INPUT);
    IO::pinMode(SMPS_CURRENT_PIN,         ANALOG_INPUT);
    IO::pinMode(V_IN_PIN,                 ANALOG_INPUT);
    IO::pinMode(T_EXTERNAL_PIN,           ANALOG_INPUT);

    /* ADM0: fCLK/8 (48 MHz / 8 = 6 MHz ADC clock — inside the 1-8 MHz spec).
     * ADCE (analog comparator) is set last so ADM1/ADM2/ADTRG can be safely
     * reconfigured.  */
    ADC->ADM0 = _10_AD_CONVERSION_CLOCK_8;

    /* Select mode + one-shot conversion + high-speed.
     * `select mode` means ADS picks one channel; one-shot means ADCS clears
     * itself after each conversion so we can change ADS between samples. */
    ADC->ADM1 = _00_AD_OPERMODE_SELECT
              | _08_AD_CONVMODE_ONESELECT
              | _00_AD_HISPEED;

    /* VREF = VDD / VSS, area-mode-1 (we don't use the comparator triggers). */
    ADC->ADM2 = _00_AD_POSITIVE_VDD
              | _00_AD_NEGATIVE_VSS
              | _00_AD_AREA_MODE_1;

    /* Software-triggered conversions. */
    ADC->ADTRG = _00_AD_TRIGGER_SOFTWARE;

    /* Upper / lower comparator limits — not used, set to full range. */
    ADC->ADUL = _FF_AD_ADUL_VALUE;
    ADC->ADLL = _00_AD_ADLL_VALUE;

    /* Power-on the ADC (analog comparator). Must come after configuration. */
    ADC->ADM0 |= ADCE;

    /* NVIC + INTC (CMS32L051 needs both — vendor INTC mask must be cleared) */
    INTC_DisableIRQ(ADC_IRQn);
    INTC_ClearPendingIRQ(ADC_IRQn);
    NVIC_ClearPendingIRQ((IRQn_Type)ADC_IRQn);
    NVIC_SetPriority((IRQn_Type)ADC_IRQn, ADC_IRQ_PRIORITY);
    NVIC_EnableIRQ((IRQn_Type)ADC_IRQn);
    INTC_EnableIRQ(ADC_IRQn);

    /* Kick off the loop. */
    current_input_  = 0;
    addSumToInput_  = (AnalogInputs::i_avrCount_ > 0);
    startConversion(current_input_);
}


} // namespace AnalogInputsADC


/* ----------------------------------------------------------------------- */
/* ADC interrupt — ADC_IRQn == 21, so the startup vector slot is IRQ21.     */
/* ----------------------------------------------------------------------- */

extern "C" void IRQ21_Handler(void)
{
    using namespace AnalogInputsADC;

    /* [MANUAL] Acknowledge the conversion that brought us here before
     * starting another one. Clearing these flags at the end can erase the
     * next conversion's interrupt if it completes while the PID is running. */
    INTC_ClearPendingIRQ(ADC_IRQn);
    NVIC_ClearPendingIRQ((IRQn_Type)ADC_IRQn);

    /* [MANUAL] ADCR[11:0] contains the right-aligned conversion result. */
    uint16_t raw_sample = (uint16_t)(ADC->ADCR & ADC_RESULT_MASK);
    uint16_t sample = (uint16_t)(raw_sample << ADC_RESULT_SHIFT);
    AnalogInputs::Name name = order_analogInputs_on[current_input_].ai_name_;

    /* Drop the very first sample of every burst (channel switch settling). */
    if(burst_count_ >= ADC_BURST_DISCARD_SAMPLES) {
        burst_sum_ += sample;

        uint32_t sum = oversample_sum_[name] + raw_sample;
        uint16_t count = oversample_count_[name] + 1;
        oversample_sum_[name] = sum;
        oversample_count_[name] = count;

        /* Avoid an initial zero until the first complete block is ready. Once
         * a block completes, keep its result stable until the next one. */
        if(count == 1 && AnalogInputs::i_adc_[name] == 0) {
            AnalogInputs::i_adc_[name] = sample;
        }

        if(count >= ADC_OVERSAMPLE_COUNT) {
            /* sum / 16 converts 256 x 12-bit codes to the existing 16-bit
             * scale. Add half a divisor for rounding to the nearest code. */
            AnalogInputs::i_adc_[name] = (uint16_t)
                    ((sum + (1U << (ADC_OVERSAMPLE_SHIFT - 1)))
                     >> ADC_OVERSAMPLE_SHIFT);
            oversample_sum_[name] = 0;
            oversample_count_[name] = 0;
        }
    }
    burst_count_++;

    if(burst_count_ >= ANALOG_INPUTS_ADC_BURST_COUNT + ADC_BURST_DISCARD_SAMPLES) {
        /* Burst complete: publish a rounded fast average for the power loop
         * and retain the existing long averaging path for calibration. */
        fast_adc_[name] = (uint16_t)
                ((burst_sum_ + ANALOG_INPUTS_ADC_BURST_COUNT / 2U)
                 / ANALOG_INPUTS_ADC_BURST_COUNT);
        if(addSumToInput_) {
            AnalogInputs::i_avrSum_[name] += burst_sum_;
        }

        bool trigger_pid = order_analogInputs_on[current_input_].trigger_PID_;
        current_input_ = nextInput(current_input_);

        if(current_input_ == 0) {
            finalizeMeasurement();
            addSumToInput_ = (AnalogInputs::i_avrCount_ > 0);
        }

        startConversion(current_input_);

        if(trigger_pid) {
            SMPS_PID::update();
        }
    } else {
        /* Trigger the next sample of the same channel. */
        ADC->ADM0 |= ADCS;
    }

}


namespace adc {
    void debug() {}
}
