
#include "memory.h"

extern "C" {
#include "CMS32L051.h"
#include "flash.h"
}
#include "atomic.h"

#define PAGE_SIZE         512

namespace eeprom {

void write_impl_less(uint8_t * addressE, const uint8_t * data, int size)
{
    uint8_t buf[PAGE_SIZE];
    int i;
    uint8_t * p_adr_start = (uint8_t*) (((uint32_t)addressE) & (~(PAGE_SIZE - 1)));
    uint8_t * p_adr_pos =  buf + (((uint32_t)addressE) & (PAGE_SIZE - 1));

    // Read current page
    for(i=0; i<PAGE_SIZE; i++)
        buf[i] = p_adr_start[i];

    // check if update is needed
    bool ok = true;
    for(i=0; i<size; i++) {
        if (p_adr_pos[i] != data[i]) {
            ok = false;
            break;
        }
    }

    if (ok)
        return;

    // Modify buffer
    for(i=0; i<size; i++)
        p_adr_pos[i] = data[i];

    // Erase sector
    EraseSector((uint32_t)p_adr_start);

    // Program page
    ProgramPage((uint32_t)p_adr_start, PAGE_SIZE, buf);
}

void write_impl(uint8_t * addressE, const uint8_t * data, int size)
{
    // don't interrupt the eeprom write operation
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        while(size > 0) {
            uint32_t adr_offset = (uint32_t) addressE & (PAGE_SIZE - 1);
            int less_size;
            if(adr_offset + size > PAGE_SIZE) {
                less_size = PAGE_SIZE - adr_offset;
            } else {
                less_size = size;
            }
            write_impl_less(addressE, data, less_size);
            addressE += less_size;
            data += less_size;
            size -= less_size;
        }
    } // enable interrupts
}

} // namespace eeprom
