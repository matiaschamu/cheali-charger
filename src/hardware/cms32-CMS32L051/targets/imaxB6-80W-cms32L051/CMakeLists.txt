
set(hardware imaxB6-80W-cms32L051)

set(SOURCE_FILES
    defaultCalibration.cpp
    HardwareConfig.h
)

CHEALI_CPU(cms32-CMS32L051)
CHEALI_GENERIC_CHARGER(50W)

add_definitions(-D__HXT=16000000UL) # External Crystal Clock Frequency (adjust to actual board)

CHEALI_GENERATE_ARM_EXEC()
