/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2014  Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#ifndef PINS_H_
#define PINS_H_

/*
 * CMS32L051 pin mapping for the imaxB6-80W board.
 *
 * Pin encoding: CMS32_PIN(port, bit) = port*8+bit  (defined in IO.h)
 *
 * Schematic notation used:
 *   Pxy  → regular port x, pin y   (ports 0-7)
 *   Qxy  → extended port 1x, pin y (ports 12-14)
 *            Q20 = P120  Q24 = P124
 *            Q30 = P130
 *            Q40 = P140  Q46 = P146  Q47 = P147
 *
 * LCD (HD44780 4-bit mode — only D4-D7 connected):
 *   RS  = Q30 = P130   CMS32_PIN(13,0) = 104
 *   E   = P22          CMS32_PIN(2,2)  =  18
 *   D4  = P27          CMS32_PIN(2,7)  =  23   ← LCD_D0_PIN in firmware
 *   D5  = Q47 = P147   CMS32_PIN(14,7) = 119   ← LCD_D1_PIN
 *   D6  = Q46 = P146   CMS32_PIN(14,6) = 118   ← LCD_D2_PIN
 *   D7  = P10          CMS32_PIN(1,0)  =   8   ← LCD_D3_PIN
 *
 * Buzzer:  P16   CMS32_PIN(1,6)  = 14
 *
 * Keyboard (active-low, pull-up enabled):
 *   MODE  = Q40 = P140   CMS32_PIN(14,0) = 112
 *   MINUS = Q20 = P120   CMS32_PIN(12,0) =  96
 *   PLUS  = P41          CMS32_PIN(4,1)  =  33
 *   ENTER = Q24 = P124   CMS32_PIN(12,4) = 100
 */

/* -----------------------------------------------------------------------
 * LCD (HD44780 in 4-bit mode)
 * The firmware drives D4-D7 via LCD_D0..D3 pins (D0=D4, D3=D7).
 * ----------------------------------------------------------------------- */
#define LCD_D0_PIN                      CMS32_PIN(2,  7)  /* P27  – HD44780 D4 */
#define LCD_D1_PIN                      CMS32_PIN(14, 7)  /* P147 – HD44780 D5 */
#define LCD_D2_PIN                      CMS32_PIN(14, 6)  /* P146 – HD44780 D6 */
#define LCD_D3_PIN                      CMS32_PIN(1,  0)  /* P10  – HD44780 D7 */
#define LCD_ENABLE_PIN                  CMS32_PIN(2,  2)  /* P22  – E          */
#define LCD_RS_PIN                      CMS32_PIN(13, 0)  /* P130 – RS         */

/* -----------------------------------------------------------------------
 * Buzzer
 * ----------------------------------------------------------------------- */
#define BUZZER_PIN                      CMS32_PIN(1,  6)  /* P16 */

/* -----------------------------------------------------------------------
 * Buttons (active-low; configured with internal pull-up in IO::pinMode)
 * ----------------------------------------------------------------------- */
#define BUTTON_STOP_PIN                 CMS32_PIN(14, 0)  /* P140 = Q40 – MODE  */
#define BUTTON_DEC_PIN                  CMS32_PIN(12, 0)  /* P120 = Q20 – MINUS */
#define BUTTON_INC_PIN                  CMS32_PIN(4,  1)  /* P41        – PLUS  */
#define BUTTON_START_PIN                CMS32_PIN(12, 4)  /* P124 = Q24 – ENTER */

/* -----------------------------------------------------------------------
 * Analog inputs  (TODO: verify schematic — placeholder ports chosen to
 * avoid collision with the confirmed LCD/button/buzzer pins above)
 * ----------------------------------------------------------------------- */
#define OUTPUT_VOLTAGE_MINUS_PIN        CMS32_PIN(7, 2)   /* TODO: verify (P72) */
#define OUTPUT_VOLTAGE_PLUS_PIN         CMS32_PIN(7, 3)   /* TODO: verify (P73) */
#define DISCHARGE_CURRENT_PIN           CMS32_PIN(7, 4)   /* TODO: verify (P74) */
#define V_IN_PIN                        CMS32_PIN(7, 5)   /* TODO: verify (P75) */
#define SMPS_CURRENT_PIN                CMS32_PIN(6, 3)   /* TODO: verify (P63) */

/* -----------------------------------------------------------------------
 * UART (debug serial)  (TODO: verify schematic)
 * ----------------------------------------------------------------------- */
#define UART_TX_PIN                     CMS32_PIN(5, 1)   /* TODO: P51 = TXD0 default */
#define T_EXTERNAL_PIN                  CMS32_PIN(2, 5)   /* TODO: verify */

/* -----------------------------------------------------------------------
 * Balancer load switches  (TODO: verify schematic)
 * ----------------------------------------------------------------------- */
#define BALANCER1_LOAD_PIN              CMS32_PIN(6, 0)   /* TODO: verify */
#define BALANCER2_LOAD_PIN              CMS32_PIN(6, 1)   /* TODO: verify */
#define BALANCER3_LOAD_PIN              CMS32_PIN(6, 2)   /* TODO: verify */
#define BALANCER4_LOAD_PIN              CMS32_PIN(6, 3)   /* TODO: verify */
#define BALANCER5_LOAD_PIN              CMS32_PIN(7, 0)   /* TODO: verify */
#define BALANCER6_LOAD_PIN              CMS32_PIN(7, 1)   /* TODO: verify */

/* -----------------------------------------------------------------------
 * Analog multiplexer address lines  (TODO: verify schematic)
 * ----------------------------------------------------------------------- */
#define MUX_ADR0_PIN                    CMS32_PIN(5, 0)   /* TODO: verify */
#define MUX_ADR1_PIN                    CMS32_PIN(5, 1)   /* TODO: verify */
#define MUX_ADR2_PIN                    CMS32_PIN(5, 2)   /* TODO: verify */
#define MUX0_Z_D_PIN                    CMS32_PIN(5, 3)   /* TODO: verify */

/* -----------------------------------------------------------------------
 * SMPS / charge / discharge control  (TODO: verify schematic)
 * ----------------------------------------------------------------------- */
#define OUTPUT_DISABLE_PIN              CMS32_PIN(1, 1)   /* TODO: verify */
#define DISCHARGE_DISABLE_PIN           CMS32_PIN(1, 2)   /* TODO: verify */
#define SMPS_DISABLE_PIN                CMS32_PIN(1, 3)   /* TODO: verify */
#define DISCHARGE_VALUE_PIN             CMS32_PIN(1, 4)   /* TODO: verify */
#define SMPS_VALUE_BOOST_PIN            CMS32_PIN(1, 4)   /* TODO: same as DISCHARGE_VALUE_PIN */
#define SMPS_VALUE_BUCK_PIN             CMS32_PIN(1, 5)   /* TODO: verify */

/* -----------------------------------------------------------------------
 * Hardware serial alternate pins  (TODO: verify schematic)
 * ----------------------------------------------------------------------- */
#define RX_HW_SERIAL_PIN5               CMS32_PIN(5, 0)   /* TODO: verify */
#define TX_HW_SERIAL_PIN7               CMS32_PIN(5, 1)   /* TODO: verify */
#define TX_HW_SERIAL_PIN38              CMS32_PIN(5, 1)   /* TODO: verify */

/* -----------------------------------------------------------------------
 * Virtual / internal pins
 * ----------------------------------------------------------------------- */
#define T_INTERNAL_PIN                  (3 + 128)         /* internal temp sensor (virtual) */

/* -----------------------------------------------------------------------
 * Analog multiplexer input addresses
 * ----------------------------------------------------------------------- */
#define MADDR_V_BALANSER_BATT_MINUS     0
#define MADDR_V_BALANSER1               1
#define MADDR_V_BALANSER2               2
#define MADDR_V_BALANSER3               3
#define MADDR_V_BALANSER4               4
#define MADDR_V_BALANSER5               5
#define MADDR_V_BALANSER6               6

#endif /* PINS_H_ */
