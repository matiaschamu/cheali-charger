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
#define OUTPUT_VOLTAGE_MINUS_PIN        CMS32_PIN(7,  4)  /* P74        – Vout−   ANI33 */
#define OUTPUT_VOLTAGE_PLUS_PIN         CMS32_PIN(7,  5)  /* P75        – Vout+   ANI34 */
#define DISCHARGE_CURRENT_PIN           CMS32_PIN(7,  3)  /* P73        – Idis    ANI32 */
#define V_IN_PIN                        CMS32_PIN(6,  2)  /* P62        – Vin     ANI27 */
#define SMPS_CURRENT_PIN                CMS32_PIN(13, 6)  /* P136 = Q36 – Ismps   ANI36 */

/* -----------------------------------------------------------------------
 * UART (debug serial)  (TODO: verify schematic)
 * ----------------------------------------------------------------------- */
#define UART_TX_PIN                     CMS32_PIN(5, 1)   /* TODO: P51 = TXD0 default */
#define T_EXTERNAL_PIN                  CMS32_PIN(6,  3)  /* P63        – NTC ext ANI28 */

/* -----------------------------------------------------------------------
 * Balancer load switches  (TODO: verify schematic)
 * ----------------------------------------------------------------------- */
#define BALANCER1_LOAD_PIN              CMS32_PIN(1,  3)  /* P13        – cell 1 discharge */
#define BALANCER2_LOAD_PIN              CMS32_PIN(1,  2)  /* P12        – cell 2 discharge */
#define BALANCER3_LOAD_PIN              CMS32_PIN(1,  1)  /* P11        – cell 3 discharge */
#define BALANCER4_LOAD_PIN              CMS32_PIN(12, 2)  /* P122 = Q22 – cell 4 discharge */
#define BALANCER5_LOAD_PIN              CMS32_PIN(12, 3)  /* P123 = Q23 – cell 5 discharge */
#define BALANCER6_LOAD_PIN              CMS32_PIN(12, 1)  /* P121 = Q21 – cell 6 discharge */

/* -----------------------------------------------------------------------
 * Balancer cell voltage ADC inputs (direct, no multiplexer)
 * Vb0 = VBATT− reference, Vb6 = VBATT+ (absolute voltages; firmware
 * computes per-cell voltage as Vb_N − Vb_(N-1)).
 * ----------------------------------------------------------------------- */
#define BALANSER0_PIN                   CMS32_PIN(3, 1)   /* P31  – Vb0 (VBATT−) ANI22 */
#define BALANSER1_PIN                   CMS32_PIN(1, 4)   /* P14  – Vb1           ANI17 */
#define BALANSER2_PIN                   CMS32_PIN(1, 7)   /* P17  – Vb2           ANI20 */
#define BALANSER3_PIN                   CMS32_PIN(3, 0)   /* P30  – Vb3           ANI21 */
#define BALANSER4_PIN                   CMS32_PIN(7, 0)   /* P70  – Vb4           ANI29 */
#define BALANSER5_PIN                   CMS32_PIN(7, 1)   /* P71  – Vb5           ANI30 */
#define BALANSER6_PIN                   CMS32_PIN(7, 2)   /* P72  – Vb6 (VBATT+)  ANI31 */

/* -----------------------------------------------------------------------
 * SMPS / charge / discharge control  (TODO: verify schematic)
 * ----------------------------------------------------------------------- */
#define OUTPUT_DISABLE_PIN              CMS32_PIN(0,  0)  /* P00 – disable output    */
#define DISCHARGE_DISABLE_PIN           CMS32_PIN(2,  0)  /* P20 – disable discharge */
#define SMPS_DISABLE_PIN                CMS32_PIN(2, 3)   /* TODO: verify schematic */
#define DISCHARGE_VALUE_PIN             CMS32_PIN(2, 4)   /* TODO: verify schematic */
#define SMPS_VALUE_BOOST_PIN            CMS32_PIN(2, 6)   /* TODO: verify schematic */
#define SMPS_VALUE_BUCK_PIN             CMS32_PIN(2, 1)   /* TODO: verify schematic */

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
 * Not used: this hardware connects cell voltages directly to ADC.
 * Kept as stubs so generic code that references them still compiles.
 * ----------------------------------------------------------------------- */
#define MADDR_V_BALANSER_BATT_MINUS     0
#define MADDR_V_BALANSER1               1
#define MADDR_V_BALANSER2               2
#define MADDR_V_BALANSER3               3
#define MADDR_V_BALANSER4               4
#define MADDR_V_BALANSER5               5
#define MADDR_V_BALANSER6               6

#endif /* PINS_H_ */
