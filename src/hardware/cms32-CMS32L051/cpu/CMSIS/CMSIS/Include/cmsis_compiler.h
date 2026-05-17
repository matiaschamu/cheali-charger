/* Minimal cmsis_compiler.h stub for arm-none-eabi GCC. */
#ifndef __CMSIS_COMPILER_H
#define __CMSIS_COMPILER_H

#include <stdint.h>

#if defined ( __GNUC__ )
  #include "cmsis_gcc.h"
#else
  #error "Unsupported compiler — please add a cmsis_compiler.h section for it."
#endif

#endif
