
set(CMSIS_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CMSIS_DIRS
    ${CMSIS_ROOT}/CMSIS/Include
    ${CMSIS_ROOT}/Device/Cmsemicon/CMS32L051/Include
    ${CMSIS_ROOT}/Device/Cmsemicon/CMS32L051/Source
    ${CMSIS_ROOT}/Device/Cmsemicon/CMS32L051/Source/GCC
    ${CMSIS_ROOT}/StdDriver/inc
    ${CMSIS_ROOT}/StdDriver/src
)

include_directories(${CMSIS_DIRS})

# Vendor StdDriver filenames (Cmsemicon CMS32L051):
#   sci.c   <- UART/serial driver (was uart.c on Nuvoton)
#   tim4.c  <- timer driver        (was timer.c on Nuvoton)
#   flash.c <- flash controller    (was FMC.c on Nuvoton)
set(CMSIS_SOURCE
    clk.c
    adc.c
    tim4.c
    gpio.c
    sci.c
    flash.c
    rst.c
    system_CMS32L051.c
    startup_CMS32L051.S
)

set(CMSIS_LINKER_SCRIPT gcc_arm.ld)

CHEALI_FIND(CPU_SOURCE_FILES "${CMSIS_SOURCE}" "${CMSIS_DIRS}")
CHEALI_FIND(CPU_LINKER_SCRIPT "${CMSIS_LINKER_SCRIPT}" "${CMSIS_DIRS}")
