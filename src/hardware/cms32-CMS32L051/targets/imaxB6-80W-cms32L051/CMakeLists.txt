
set(hardware imaxB6-80W-cms32L051)

# Calibration v11 stores every balance ADC as a cumulative connector-tap
# voltage. Other targets keep the global v10 layout and behavior.
add_definitions(-DCHEALI_CHARGER_EEPROM_CALIBRATION_VERSION=11)
set(cheali-charger-eeprom-version-string
    "e11.${cheali-charger-eeprom-programdata-version}.${cheali-charger-eeprom-settings-version}")

set(SOURCE_FILES
    defaultCalibration.cpp
    HardwareConfig.h
)

CHEALI_CPU(cms32-CMS32L051)
CHEALI_GENERIC_CHARGER(50W)

add_definitions(-D__HXT=16000000UL) # External Crystal Clock Frequency (adjust to actual board)

CHEALI_GENERATE_ARM_EXEC()
