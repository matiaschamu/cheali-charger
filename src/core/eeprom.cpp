/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013  Paweł Stawicki. All right reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "AnalogInputs.h"
#include "ProgramData.h"
#include "Hardware.h"
#include "Settings.h"
#include "memory.h"
#include "Version.h"
#include "eeprom.h"
#include "Screen.h"

#define CHARS_TO_UINT16(x,y) (((y)<< 8) + (x))

#define EEPROM_READ_TRIALS 5

namespace eeprom {
    Data data EEMEM;

#ifdef ENABLE_CUMULATIVE_BALANCE_PORT
    /* Convert the previous mixed balance calibration layout exactly once:
     * Vb1/Vb2 were already cumulative, while Vb3..VbN stored one cell each.
     * The final flash image contains the new version and its matching CRC;
     * boot validation rejects an interrupted or partial synchronization. */
    static void migrateCumulativeBalancePortCalibration() {
        uint16_t version = eeprom::read(&data.calibrationVersion);
        if(version != CUMULATIVE_BALANCE_PORT_LEGACY_CALIBRATION_VERSION)
            return;
        if(restoreCalibrationCRC(false))
            return;

        AnalogInputs::CalibrationPoint legacy[MAX_BALANCE_CELLS][ANALOG_INPUTS_MAX_CALIBRATION_POINTS];
        for(uint8_t cell=0; cell < MAX_BALANCE_CELLS; cell++) {
            AnalogInputs::Name pin = AnalogInputs::Name(AnalogInputs::Vb1_pin+cell);
            for(uint8_t point=0; point < ANALOG_INPUTS_MAX_CALIBRATION_POINTS; point++) {
                eeprom::read<AnalogInputs::CalibrationPoint>(legacy[cell][point],
                        &data.calibration[pin].p[point]);
            }
        }

        /* CMS EEPROM emulation otherwise reprograms the complete 1 KiB shadow
         * after every small write. Build the converted image in RAM and sync
         * it once; version and CRC will reject an interrupted flash image. */
        eeprom::beginWriteBatch();
        for(uint8_t point=0; point < ANALOG_INPUTS_MAX_CALIBRATION_POINTS; point++) {
            uint32_t cumulative = legacy[1][point].y;
            for(uint8_t cell=2; cell < MAX_BALANCE_CELLS; cell++) {
                cumulative += legacy[cell][point].y;
                AnalogInputs::CalibrationPoint migrated = legacy[cell][point];
                migrated.y = cumulative > UINT16_MAX ? UINT16_MAX : cumulative;
                AnalogInputs::Name pin = AnalogInputs::Name(AnalogInputs::Vb1_pin+cell);
                eeprom::write<AnalogInputs::CalibrationPoint>(
                        &data.calibration[pin].p[point], migrated);
            }
        }
        restoreCalibrationCRC();
        eeprom::write(&data.calibrationVersion,
                uint16_t(CHEALI_CHARGER_EEPROM_CALIBRATION_VERSION));
        eeprom::endWriteBatch();
    }
#endif

    bool testOrRestore(uint16_t * adr, uint16_t version, bool restore) {
        uint8_t trials = EEPROM_READ_TRIALS;
        if(restore) {
            eeprom::write(adr, version);
        }
        while(--trials) {
            if(eeprom::read(adr) == version)
                return false;
            Time::delay(100);
        }
        return true;
    }

    uint8_t testOrRestore(uint8_t restore) {
        uint8_t test = 0;

        if(testOrRestore((uint16_t*) &data.magicString[0], CHARS_TO_UINT16('c','h'), restore & EEPROM_RESTORE_MAGIC_NUMBER)) test |= EEPROM_RESTORE_MAGIC_NUMBER;
        if(testOrRestore((uint16_t*) &data.magicString[2], CHARS_TO_UINT16('l','i'), restore & EEPROM_RESTORE_MAGIC_NUMBER)) test |= EEPROM_RESTORE_MAGIC_NUMBER;
        if(testOrRestore((uint16_t*) &data.architecture, CHEALI_CHARGER_ARCHITECTURE, restore & EEPROM_RESTORE_MAGIC_NUMBER)) test |= EEPROM_RESTORE_MAGIC_NUMBER;
        if(testOrRestore((uint16_t*) &data.architectureInfo, CHEALI_CHARGER_ARCHITECTURE_INFO, restore & EEPROM_RESTORE_MAGIC_NUMBER)) test |= EEPROM_RESTORE_MAGIC_NUMBER;

#ifdef ENABLE_CUMULATIVE_BALANCE_PORT
        if(restore == 0 && !(test & EEPROM_RESTORE_MAGIC_NUMBER))
            migrateCumulativeBalancePortCalibration();
#endif

        if(testOrRestore(&data.calibrationVersion, CHEALI_CHARGER_EEPROM_CALIBRATION_VERSION, restore & EEPROM_RESTORE_CALIBRATION))    test |= EEPROM_RESTORE_CALIBRATION;
        if(testOrRestore(&data.programDataVersion, CHEALI_CHARGER_EEPROM_PROGRAMDATA_VERSION, restore & EEPROM_RESTORE_PROGRAM_DATA))   test |= EEPROM_RESTORE_PROGRAM_DATA;
        if(testOrRestore(&data.settingVersion, CHEALI_CHARGER_EEPROM_SETTINGS_VERSION, restore & EEPROM_RESTORE_SETTINGS))              test |= EEPROM_RESTORE_SETTINGS;

        if(restore & EEPROM_RESTORE_CALIBRATION) AnalogInputs::restoreDefault();
        if(restoreCalibrationCRC(false)) test |= EEPROM_RESTORE_CALIBRATION;

        if(restore & EEPROM_RESTORE_PROGRAM_DATA) ProgramData::restoreDefault();
        if(restoreProgramDataCRC(false)) test |= EEPROM_RESTORE_PROGRAM_DATA;

        if(restore & EEPROM_RESTORE_SETTINGS)   Settings::restoreDefault();
        if(restoreSettingsCRC(false)) test |= EEPROM_RESTORE_SETTINGS;

        return test;
    }

    void restoreDefault(uint8_t what) {
        Screen::runAskResetEeprom(what);
        if(what & EEPROM_RESTORE_MAGIC_NUMBER)  what |= EEPROM_RESTORE_CALIBRATION;
        if(what & EEPROM_RESTORE_CALIBRATION)   what |= EEPROM_RESTORE_PROGRAM_DATA;
        if(what & EEPROM_RESTORE_PROGRAM_DATA)  what |= EEPROM_RESTORE_SETTINGS;

        Screen::displayResettingEeprom();
        uint8_t after = testOrRestore(what);
        Screen::runResetEepromDone(what, after);
    }

#ifdef ENABLE_EEPROM_RESTORE_DEFAULT
    bool check() {
        uint8_t c = testOrRestore(0);
        if(c == 0) return true;
        restoreDefault(c);
        return false;
    }

    void restoreDefault() {
        uint8_t what = testOrRestore(0) | EEPROM_RESTORE_MAGIC_NUMBER;
        restoreDefault(what);
    }
#endif


#ifdef ENABLE_EEPROM_CRC

    inline uint16_t crc16_update(uint16_t crc, uint8_t a) {
        uint8_t i;
        crc ^= a;
        for (i = 0; i < 8; ++i) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
        return crc;
    }

    uint16_t getCRC(uint8_t * adr, uint16_t size) {
        uint16_t crc = 0xffff;
        for(uint16_t i = 0; i < size; i++) {
            crc = crc16_update(crc, eeprom::read(&adr[i]));
        }
        return crc;
    }

    bool testOrRestoreCRC(uint8_t * adr, uint16_t size, bool restore) {
        uint16_t CRC = getCRC(adr, size);
        return testOrRestore((uint16_t*)(adr+size),CRC, restore);
    }

    bool restoreCalibrationCRC(bool restore) {
        return testOrRestoreCRC((uint8_t*)&data.calibration, sizeof(data.calibration), restore);
    }

    bool restoreProgramDataCRC(bool restore) {
        return testOrRestoreCRC((uint8_t*)&data.battery, sizeof(data.battery), restore);
    }

    bool restoreSettingsCRC(bool restore) {
        return testOrRestoreCRC((uint8_t*)&data.settings, sizeof(data.settings), restore);
    }
#endif

    /* Weak default — ports that need to load eeprom::data from flash at boot
     * (e.g. CMS32L051) override this. */
    __attribute__((weak)) void initFromFlash() { /* no-op */ }

}
