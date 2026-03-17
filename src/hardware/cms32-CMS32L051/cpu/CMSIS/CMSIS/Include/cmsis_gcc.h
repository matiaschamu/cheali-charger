/**************************************************************************//**
 * @file     cmsis_gcc.h
 * @brief    CMSIS compiler GCC header file
 * @version  V5.0.4
 * @date     09. April 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 */

#ifndef __CMSIS_GCC_H
#define __CMSIS_GCC_H

#include <stdint.h>

/* ignore some GCC warnings */
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wattributes"


/* Fallback for __has_builtin */
#ifndef __has_builtin
  #define __has_builtin(x) (0)
#endif

/* CMSIS compiler specific defines */
#ifndef   __ASM
  #define __ASM                                  __asm
#endif
#ifndef   __INLINE
  #define __INLINE                               inline
#endif
#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE                        static inline
#endif
#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static inline
#endif
#ifndef   __NO_RETURN
  #define __NO_RETURN                            __attribute__((__noreturn__))
#endif
#ifndef   __USED
  #define __USED                                 __attribute__((used))
#endif
#ifndef   __WEAK
  #define __WEAK                                 __attribute__((weak))
#endif
#ifndef   __PACKED
  #define __PACKED                               __attribute__((packed, aligned(1)))
#endif
#ifndef   __PACKED_STRUCT
  #define __PACKED_STRUCT                        struct __attribute__((packed, aligned(1)))
#endif
#ifndef   __PACKED_UNION
  #define __PACKED_UNION                         union __attribute__((packed, aligned(1)))
#endif
#ifndef   __UNALIGNED_UINT32        /* deprecated */
  struct __attribute__((packed)) T_UINT32 { uint32_t v; };
  #define __UNALIGNED_UINT32(x)                  (((struct T_UINT32 *)(x))->v)
#endif
#ifndef   __ALIGNED
  #define __ALIGNED(x)                           __attribute__((aligned(x)))
#endif
#ifndef   __RESTRICT
  #define __RESTRICT                             __restrict
#endif

/* ###########################  Core Function Access  ########################### */

__STATIC_FORCEINLINE void __enable_irq(void)
{
  __ASM volatile ("cpsie i" : : : "memory");
}

__STATIC_FORCEINLINE void __disable_irq(void)
{
  __ASM volatile ("cpsid i" : : : "memory");
}

__STATIC_FORCEINLINE uint32_t __get_CONTROL(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, control" : "=r" (result) );
  return(result);
}

__STATIC_FORCEINLINE void __set_CONTROL(uint32_t control)
{
  __ASM volatile ("MSR control, %0" : : "r" (control) : "memory");
}

__STATIC_FORCEINLINE uint32_t __get_IPSR(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, ipsr" : "=r" (result) );
  return(result);
}

__STATIC_FORCEINLINE uint32_t __get_APSR(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, apsr" : "=r" (result) );
  return(result);
}

__STATIC_FORCEINLINE uint32_t __get_xPSR(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, xpsr" : "=r" (result) );
  return(result);
}

__STATIC_FORCEINLINE uint32_t __get_PSP(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, psp"  : "=r" (result) );
  return(result);
}

__STATIC_FORCEINLINE void __set_PSP(uint32_t topOfProcStack)
{
  __ASM volatile ("MSR psp, %0" : : "r" (topOfProcStack) : );
}

__STATIC_FORCEINLINE uint32_t __get_MSP(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, msp" : "=r" (result) );
  return(result);
}

__STATIC_FORCEINLINE void __set_MSP(uint32_t topOfMainStack)
{
  __ASM volatile ("MSR msp, %0" : : "r" (topOfMainStack) : );
}

__STATIC_FORCEINLINE uint32_t __get_PRIMASK(void)
{
  uint32_t result;
  __ASM volatile ("MRS %0, primask" : "=r" (result) :: "memory");
  return(result);
}

__STATIC_FORCEINLINE void __set_PRIMASK(uint32_t priMask)
{
  __ASM volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}

/* ##########################  Core Instruction Access  ######################### */

#define __NOP()                             __ASM volatile ("nop")
#define __WFI()                             __ASM volatile ("wfi")
#define __WFE()                             __ASM volatile ("wfe")
#define __SEV()                             __ASM volatile ("sev")

__STATIC_FORCEINLINE void __ISB(void)
{
  __ASM volatile ("isb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __DSB(void)
{
  __ASM volatile ("dsb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __DMB(void)
{
  __ASM volatile ("dmb 0xF":::"memory");
}

__STATIC_FORCEINLINE uint32_t __REV(uint32_t value)
{
  return __builtin_bswap32(value);
}

__STATIC_FORCEINLINE uint32_t __REV16(uint32_t value)
{
  uint32_t result;
  __ASM volatile ("rev16 %0, %1" : "=r" (result) : "r" (value) );
  return result;
}

__STATIC_FORCEINLINE int32_t __REVSH(int32_t value)
{
  return (int16_t)__builtin_bswap16(value);
}

__STATIC_FORCEINLINE uint32_t __ROR(uint32_t op1, uint32_t op2)
{
  op2 %= 32U;
  if (op2 == 0U) { return op1; }
  return (op1 >> op2) | (op1 << (32U - op2));
}

__STATIC_FORCEINLINE uint32_t __BKPT(uint8_t value)
{
  __ASM volatile ("bkpt %0" : : "i" (value) );
  return 0; // dummy
}

#endif /* __CMSIS_GCC_H */
