
#include "IO.h"

namespace IO {

uint32_t dummyPin = 0;

void pinMode_(uint8_t port, uint8_t pin, uint8_t mode) {
    if(mode == (uint8_t)ANALOG_INPUT) {
        PORT_Init((PORT_TypeDef)port, (PIN_TypeDef)pin, (PIN_ModeDef)ANALOG_INPUT);
        // TODO: additional ADC setup if needed
    } else {
        PORT_Init((PORT_TypeDef)port, (PIN_TypeDef)pin, (PIN_ModeDef)mode);
    }
}

}
