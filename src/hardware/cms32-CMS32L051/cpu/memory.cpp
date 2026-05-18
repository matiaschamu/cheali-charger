/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013 Paweł Stawicki. All right reserved.

    Licensed under the GPLv3.
*/
#include "memory.h"
#include "eeprom.h"
#include <cstring>

extern "C" {
#include "CMS32L051.h"
}

/*
 * EEPROM-emulation for CMS32L051.
 *
 * The linker reserves the last 1 KB of flash (0xFC00..0xFFFF) as
 * `.eeprom_shadow`. On boot eeprom::initFromFlash() copies that region into
 * the RAM-resident `eeprom::data` struct; every write_impl() re-syncs the
 * entire struct (erase + byte-program).
 *
 * Flash programming uses the FMC at 0x40020000 — same sequence the OpenOCD
 * TCL script uses to flash firmware. Interrupts are disabled during each
 * erase/program operation; the CMS32L051 FMC stalls the bus while the
 * operation is in progress so code can still run from flash.
 */

extern uint8_t __eeprom_shadow_start[];

namespace {

constexpr uint32_t EEPROM_ADDR  = 0xFC00;
constexpr uint32_t EEPROM_SIZE  = 0x0400;

inline void fmc_wait_ovf() {
    while ((FMC->FLSTS & FMC_FLSTS_OVF_Msk) == 0) { /* spin */ }
    FMC->FLSTS = FMC_FLSTS_OVF_Msk;
}

void flash_erase_sector(uint32_t addr) {
    __disable_irq();
    FMC->FLERMD  = 0x10;     /* sector erase mode */
    FMC->FLPROT  = 0xF1;     /* unlock */
    FMC->FLOPMD1 = 0x55;
    FMC->FLOPMD2 = 0xAA;
    *(volatile uint32_t *)addr = 0xFFFFFFFF;   /* trigger erase */
    fmc_wait_ovf();
    FMC->FLERMD = 0x00;
    FMC->FLPROT = 0xF0;      /* lock */
    __enable_irq();
}

void flash_program_byte(uint32_t addr, uint8_t value) {
    __disable_irq();
    FMC->FLPROT  = 0xF1;
    FMC->FLOPMD1 = 0xAA;
    FMC->FLOPMD2 = 0x55;
    *(volatile uint8_t *)addr = value;
    fmc_wait_ovf();
    FMC->FLPROT = 0xF0;
    __enable_irq();
}

void sync_to_flash() {
    flash_erase_sector(EEPROM_ADDR);
    const uint8_t * src = reinterpret_cast<const uint8_t *>(&eeprom::data);
    const uint32_t n = sizeof(eeprom::data);
    for (uint32_t i = 0; i < n; i++) {
        if (src[i] != 0xFF) {              /* erased flash is 0xFF — skip */
            flash_program_byte(EEPROM_ADDR + i, src[i]);
        }
    }
}

} // namespace

namespace eeprom {

void initFromFlash() {
    std::memcpy(&data, reinterpret_cast<const void *>(EEPROM_ADDR), sizeof(data));
}

void write_impl(uint8_t * addressE, const uint8_t * src, int size) {
    if (size <= 0) return;
    /* 1. Update the RAM copy at the requested address. */
    std::memcpy(addressE, src, size);
    /* 2. Mirror the full eeprom::data struct back into flash. */
    sync_to_flash();
}

} // namespace eeprom
