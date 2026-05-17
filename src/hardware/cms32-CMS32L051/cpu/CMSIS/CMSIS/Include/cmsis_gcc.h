/* Minimal cmsis_gcc.h subset for arm-none-eabi GCC on Cortex-M0+.
 * Provides the macros / intrinsics used by core_cm0plus.h.
 * For production work, replace with the official ARM-software CMSIS_5 header.
 */
#ifndef __CMSIS_GCC_H
#define __CMSIS_GCC_H

#include <stdint.h>

/* ---------------- Compiler attributes ---------------- */
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
  #define __PACKED_UNION                         union  __attribute__((packed, aligned(1)))
#endif
#ifndef   __UNALIGNED_UINT16_WRITE
  __PACKED_STRUCT T_UINT16_WRITE { uint16_t v; };
  #define __UNALIGNED_UINT16_WRITE(a,v)          (void)((((struct T_UINT16_WRITE *)(void *)(a))->v) = (v))
#endif
#ifndef   __UNALIGNED_UINT16_READ
  __PACKED_STRUCT T_UINT16_READ { uint16_t v; };
  #define __UNALIGNED_UINT16_READ(a)             (((const struct T_UINT16_READ *)(const void *)(a))->v)
#endif
#ifndef   __UNALIGNED_UINT32_WRITE
  __PACKED_STRUCT T_UINT32_WRITE { uint32_t v; };
  #define __UNALIGNED_UINT32_WRITE(a,v)          (void)((((struct T_UINT32_WRITE *)(void *)(a))->v) = (v))
#endif
#ifndef   __UNALIGNED_UINT32_READ
  __PACKED_STRUCT T_UINT32_READ { uint32_t v; };
  #define __UNALIGNED_UINT32_READ(a)             (((const struct T_UINT32_READ *)(const void *)(a))->v)
#endif
#ifndef   __ALIGNED
  #define __ALIGNED(x)                           __attribute__((aligned(x)))
#endif
#ifndef   __RESTRICT
  #define __RESTRICT                             __restrict
#endif
#ifndef   __COMPILER_BARRIER
  #define __COMPILER_BARRIER()                   __ASM volatile("":::"memory")
#endif

/* ---------------- Core instructions ---------------- */
__STATIC_FORCEINLINE void __NOP(void)  { __ASM volatile ("nop"); }
__STATIC_FORCEINLINE void __WFI(void)  { __ASM volatile ("wfi":::"memory"); }
__STATIC_FORCEINLINE void __WFE(void)  { __ASM volatile ("wfe":::"memory"); }
__STATIC_FORCEINLINE void __SEV(void)  { __ASM volatile ("sev"); }
__STATIC_FORCEINLINE void __ISB(void)  { __ASM volatile ("isb 0xF":::"memory"); }
__STATIC_FORCEINLINE void __DSB(void)  { __ASM volatile ("dsb 0xF":::"memory"); }
__STATIC_FORCEINLINE void __DMB(void)  { __ASM volatile ("dmb 0xF":::"memory"); }

__STATIC_FORCEINLINE uint32_t __REV(uint32_t value)
{
    return __builtin_bswap32(value);
}
__STATIC_FORCEINLINE uint32_t __REV16(uint32_t value)
{
    uint32_t result;
    __ASM volatile ("rev16 %0, %1" : "=r" (result) : "r" (value));
    return result;
}
__STATIC_FORCEINLINE int32_t __REVSH(int32_t value)
{
    return (int16_t)__builtin_bswap16((uint16_t)value);
}
__STATIC_FORCEINLINE uint32_t __ROR(uint32_t op1, uint32_t op2)
{
    op2 %= 32U;
    if (op2 == 0U) return op1;
    return (op1 >> op2) | (op1 << (32U - op2));
}
#define __BKPT(value) __ASM volatile ("bkpt "#value)
__STATIC_FORCEINLINE uint32_t __RBIT(uint32_t value)
{
    uint32_t result = 0U;
    int8_t s = (4U /*sizeof(v)*/ * 8U) - 1U;
    for (value >>= 1U; value != 0U; value >>= 1U) {
        result <<= 1U;
        result |= value & 1U;
        s--;
    }
    result <<= s;
    return result;
}
__STATIC_FORCEINLINE uint8_t  __CLZ(uint32_t value) { return value ? (uint8_t)__builtin_clz(value) : 32U; }

/* ---------------- Special-register / IRQ control ---------------- */
__STATIC_FORCEINLINE void __enable_irq(void)  { __ASM volatile ("cpsie i" : : : "memory"); }
__STATIC_FORCEINLINE void __disable_irq(void) { __ASM volatile ("cpsid i" : : : "memory"); }

__STATIC_FORCEINLINE uint32_t __get_CONTROL(void) { uint32_t r; __ASM volatile ("MRS %0, control" : "=r" (r)); return r; }
__STATIC_FORCEINLINE void     __set_CONTROL(uint32_t c) { __ASM volatile ("MSR control, %0" : : "r" (c) : "memory"); }
__STATIC_FORCEINLINE uint32_t __get_IPSR(void)    { uint32_t r; __ASM volatile ("MRS %0, ipsr" : "=r" (r)); return r; }
__STATIC_FORCEINLINE uint32_t __get_APSR(void)    { uint32_t r; __ASM volatile ("MRS %0, apsr" : "=r" (r)); return r; }
__STATIC_FORCEINLINE uint32_t __get_xPSR(void)    { uint32_t r; __ASM volatile ("MRS %0, xpsr" : "=r" (r)); return r; }
__STATIC_FORCEINLINE uint32_t __get_PSP(void)     { uint32_t r; __ASM volatile ("MRS %0, psp"  : "=r" (r)); return r; }
__STATIC_FORCEINLINE void     __set_PSP(uint32_t v){ __ASM volatile ("MSR psp, %0" : : "r" (v) : ); }
__STATIC_FORCEINLINE uint32_t __get_MSP(void)     { uint32_t r; __ASM volatile ("MRS %0, msp"  : "=r" (r)); return r; }
__STATIC_FORCEINLINE void     __set_MSP(uint32_t v){ __ASM volatile ("MSR msp, %0" : : "r" (v) : ); }
__STATIC_FORCEINLINE uint32_t __get_PRIMASK(void) { uint32_t r; __ASM volatile ("MRS %0, primask" : "=r" (r)); return r; }
__STATIC_FORCEINLINE void     __set_PRIMASK(uint32_t v){ __ASM volatile ("MSR primask, %0" : : "r" (v) : "memory"); }

/* No-op fault control on M0+ */
#ifndef __CLREX
#define __CLREX()  ((void)0)
#endif

#endif /* __CMSIS_GCC_H */
