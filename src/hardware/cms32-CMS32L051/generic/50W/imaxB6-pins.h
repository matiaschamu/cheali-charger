
#ifndef PINS_H_
#define PINS_H_

// Pin encoding: (PORT << 3) | PIN
// PORT enum: PORT0=0, PORT1=1, PORT2=2, PORT3=3, PORT4=4, PORT5=5, PORT6=6, PORT7=7, PORT12=12, PORT13=13, PORT14=14
// Example: PORT3 PIN1 -> (3 << 3) | 1 = 25

// LCD pins (PLEASE VERIFY WITH YOUR HARDWARE)
#define LCD_RS_PIN                      ((3 << 3) | 1)  // Guess
#define LCD_ENABLE_PIN                  ((4 << 3) | 1)  // Guess
#define LCD_D0_PIN                      ((0 << 3) | 4)  // Guess
#define LCD_D1_PIN                      ((0 << 3) | 5)  // Guess
#define LCD_D2_PIN                      ((0 << 3) | 6)  // Guess
#define LCD_D3_PIN                      ((0 << 3) | 7)  // Guess

// Buttons (PLEASE VERIFY)
#define BUTTON_STOP_PIN                 ((3 << 3) | 2)
#define BUTTON_DEC_PIN                  ((3 << 3) | 3)
#define BUTTON_INC_PIN                  ((3 << 3) | 4)
#define BUTTON_START_PIN                ((3 << 3) | 5)

// Other pins...
#define BUZZER_PIN                      ((4 << 3) | 3)

#define BALANCER1_LOAD_PIN              ((1 << 3) | 0) // Placeholder
#define BALANCER2_LOAD_PIN              ((1 << 3) | 1) // Placeholder
#define BALANCER3_LOAD_PIN              ((1 << 3) | 2) // Placeholder
#define BALANCER4_LOAD_PIN              ((1 << 3) | 3) // Placeholder
#define BALANCER5_LOAD_PIN              ((1 << 3) | 4) // Placeholder
#define BALANCER6_LOAD_PIN              ((1 << 3) | 5) // Placeholder

#define OUTPUT_DISABLE_PIN              ((2 << 3) | 0) // Placeholder
#define DISCHARGE_VALUE_PIN             ((2 << 3) | 1) // Placeholder
#define DISCHARGE_DISABLE_PIN           ((2 << 3) | 2) // Placeholder

#define SMPS_VALUE_BUCK_PIN             ((2 << 3) | 3) // Placeholder
#define SMPS_VALUE_BOOST_PIN            ((2 << 3) | 4) // Placeholder
#define SMPS_DISABLE_PIN                ((2 << 3) | 5) // Placeholder

#define T_EXTERNAL_PIN                  ((1 << 3) | 6) // Placeholder

#define MUX0_Z_D_PIN                    ((1 << 3) | 7) // Placeholder

//virtual pin
#define T_INTERNAL_PIN                  (3+128)

//Multiplexer addresses:
#define MADDR_V_BALANSER_BATT_MINUS     0
#define MADDR_V_BALANSER1               1
#define MADDR_V_BALANSER2               2
#define MADDR_V_BALANSER3               3
#define MADDR_V_BALANSER4               4
#define MADDR_V_BALANSER5               5
#define MADDR_V_BALANSER6               6

#define V_IN_PIN                        ((4 << 3) | 0) // Placeholder
#define OUTPUT_VOLTAGE_MINUS_PIN        ((4 << 3) | 1) // Placeholder
#define OUTPUT_VOLTAGE_PLUS_PIN         ((4 << 3) | 2) // Placeholder
#define SMPS_CURRENT_PIN                ((4 << 3) | 3) // Placeholder
#define DISCHARGE_CURRENT_PIN           ((4 << 3) | 4) // Placeholder

#define MUX_ADR0_PIN                    ((4 << 3) | 5) // Placeholder
#define MUX_ADR1_PIN                    ((4 << 3) | 6) // Placeholder
#define MUX_ADR2_PIN                    ((4 << 3) | 7) // Placeholder

#endif /* PINS_H_ */
