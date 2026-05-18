/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/

/*
 * Include order matters here.
 *
 * gpio.h defines a PIN_ModeDef enum with values:
 *   INPUT=0, PULLUP_INPUT=1, TTL_INPUT=2, ANALOG_INPUT=3, OUTPUT=4 ...
 *
 * IO.h (included after) redefines INPUT, OUTPUT, ANALOG_INPUT as macros
 * with different numeric values (1, 0, 200).  To avoid the enum members
 * being substituted by those macros we include vendor headers first and
 * snapshot the numeric values we need before the macros arrive.
 *
 * CMS32L051.h must come before gpio.h because it defines the PORT peripheral
 * pointer used by PORT_SetBit / PORT_ClrBit / PORT_GetBit macros.
 */
#include "CMS32L051.h"   /* PORT_Type, PORT peripheral macro (has its own extern "C") */
extern "C" {
#include "gpio.h"        /* PORT_Init, PORT_SetBit, PORT_ClrBit, PORT_GetBit, PIN_ModeDef */
}

/* Snapshot PIN_ModeDef values using casts — immune to later macro redefinition. */
static const PIN_ModeDef CMS32_MODE_INPUT    = (PIN_ModeDef)0;   /* INPUT       */
static const PIN_ModeDef CMS32_MODE_PULLUP   = (PIN_ModeDef)1;   /* PULLUP_INPUT */
static const PIN_ModeDef CMS32_MODE_ANALOG   = (PIN_ModeDef)3;   /* ANALOG_INPUT */
static const PIN_ModeDef CMS32_MODE_OUTPUT   = (PIN_ModeDef)4;   /* OUTPUT      */

/* Now safe to include cheali-charger headers — their macros shadow the enum
 * names but we no longer use those names. */
#include "IO.h"

namespace IO
{

/*
 * Decode pin number (CMS32_PIN encoding) into PORT and BIT.
 * port = pinNumber >> 3  (i.e. pinNumber / 8)
 * bit  = pinNumber & 7
 */
static inline uint8_t pinToPort(uint8_t pinNumber) { return pinNumber >> 3; }
static inline uint8_t pinToBit (uint8_t pinNumber) { return pinNumber & 7u; }

void pinMode(uint8_t pinNumber, uint8_t mode)
{
    /* Virtual pins (≥ 128) have no real GPIO. */
    if (pinNumber >= 128) return;

    PORT_TypeDef port = (PORT_TypeDef)pinToPort(pinNumber);
    PIN_TypeDef  bit  = (PIN_TypeDef) pinToBit (pinNumber);

    PIN_ModeDef gpioMode;
    switch (mode) {
        case 0:   gpioMode = CMS32_MODE_OUTPUT; break;  /* OUTPUT                 */
        case 1:   gpioMode = CMS32_MODE_PULLUP; break;  /* INPUT (with pull-up)   */
        case 200: /* fall-through */
        case 201: gpioMode = CMS32_MODE_ANALOG; break;  /* ANALOG_INPUT[_DISCHARGE] */
        default:  gpioMode = CMS32_MODE_INPUT;  break;
    }
    PORT_Init(port, bit, gpioMode);
}

void digitalWrite(uint8_t pinNumber, uint32_t value)
{
    if (pinNumber >= 128) return;
    PORT_TypeDef port = (PORT_TypeDef)pinToPort(pinNumber);
    PIN_TypeDef  bit  = (PIN_TypeDef) pinToBit (pinNumber);
    if (value) {
        PORT_SetBit(port, bit);
    } else {
        PORT_ClrBit(port, bit);
    }
}

uint8_t digitalRead(uint8_t pinNumber)
{
    if (pinNumber >= 128) return 0;
    PORT_TypeDef port = (PORT_TypeDef)pinToPort(pinNumber);
    PIN_TypeDef  bit  = (PIN_TypeDef) pinToBit (pinNumber);
    return PORT_GetBit(port, bit) ? 1u : 0u;
}

/*
 * ANI channel lookup for CMS32L051.
 * Source: ADC_PORT_SETTING in userdefine.h.
 *
 * Encoded pin → ANI channel:
 *   P00=0  → ANI11   P01=1  → ANI10
 *   P10=8  → ANI9    P11=9  → ANI8    P12=10 → ANI13  P13=11 → ANI16
 *   P14=12 → ANI17   P15=13 → ANI18   P16=14 → ANI19  P17=15 → ANI20
 *   P20=16 → ANI0    P21=17 → ANI1    P22=18 → ANI2   P23=19 → ANI3
 *   P24=20 → ANI4    P25=21 → ANI5    P26=22 → ANI6   P27=23 → ANI7
 *   P30=24 → ANI21   P31=25 → ANI22
 *   P60=48 → ANI25   P61=49 → ANI26   P62=50 → ANI27  P63=51 → ANI28
 *   P70=56 → ANI29   P71=57 → ANI30   P72=58 → ANI31  P73=59 → ANI32
 *   P74=60 → ANI33   P75=61 → ANI34
 *   P120=96 → ANI14
 *   P130=104 → ANI35  P136=110 → ANI36
 *   P146=118 → ANI15  P147=119 → ANI12
 */
uint8_t getADCChannel(uint8_t pinNumber)
{
    switch (pinNumber) {
        case  0: return 11;  /* P00 */
        case  1: return 10;  /* P01 */
        case  8: return  9;  /* P10 */
        case  9: return  8;  /* P11 */
        case 10: return 13;  /* P12 */
        case 11: return 16;  /* P13 */
        case 12: return 17;  /* P14 */
        case 13: return 18;  /* P15 */
        case 14: return 19;  /* P16 */
        case 15: return 20;  /* P17 */
        case 16: return  0;  /* P20 */
        case 17: return  1;  /* P21 */
        case 18: return  2;  /* P22 */
        case 19: return  3;  /* P23 */
        case 20: return  4;  /* P24 */
        case 21: return  5;  /* P25 */
        case 22: return  6;  /* P26 */
        case 23: return  7;  /* P27 */
        case 24: return 21;  /* P30 */
        case 25: return 22;  /* P31 */
        case 48: return 25;  /* P60 */
        case 49: return 26;  /* P61 */
        case 50: return 27;  /* P62 */
        case 51: return 28;  /* P63 */
        case 56: return 29;  /* P70 */
        case 57: return 30;  /* P71 */
        case 58: return 31;  /* P72 */
        case 59: return 32;  /* P73 */
        case 60: return 33;  /* P74 */
        case 61: return 34;  /* P75 */
        case 96: return 14;  /* P120 */
        case 104: return 35; /* P130 */
        case 110: return 36; /* P136 */
        case 118: return 15; /* P146 */
        case 119: return 12; /* P147 */
        default:  return 0xFF;
    }
}

} // namespace IO
