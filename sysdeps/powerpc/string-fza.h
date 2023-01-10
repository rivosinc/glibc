/* Zero byte detection; basics.  PowerPC version.
   Copyright (C) 2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _POWERPC_STRING_FZA_H
#define _POWERPC_STRING_FZA_H 1

/* PowerISA 2.05 (POWER6) provides cmpb instruction.  */
#ifdef _ARCH_PWR6
# include <limits.h>
# include <endian.h>
# include <stdint.h>
# include <string-maskoff.h>
# include <string-optype.h>

/* The functions return a byte mask.  */
typedef op_t find_t;

/* Return the mask WORD shifted based on S_INT address value, to ignore
   values not presented in the aligned word read.  */
static __always_inline find_t
shift_find (find_t word, uintptr_t s)
{
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    return word >> (CHAR_BIT * (s % sizeof (op_t)));
  else
    return word << (CHAR_BIT * (s % sizeof (op_t)));
}

/* Mask off the bits defined the the S alignment value.  */
static __always_inline find_t
shift_find_last (find_t word, uintptr_t s)
{
   return word & create_mask (s);
}

/* This function returns 0xff for each byte that is
   equal between X1 and X2.  */

static __always_inline find_t
find_eq_all (op_t x1, op_t x2)
{
  op_t ret;
  asm ("cmpb %0,%1,%2" : "=r"(ret) : "r"(x1), "r"(x2));
  return ret;
}

/* This function returns 0xff for each byte that is zero in X.  */

static __always_inline find_t
find_zero_all (op_t x)
{
  return find_eq_all (x, 0);
}

/* Identify zero bytes in X1 or equality between X1 and X2.  */

static __always_inline find_t
find_zero_eq_all (op_t x1, op_t x2)
{
  return find_zero_all (x1) | find_eq_all (x1, x2);
}

/* Identify zero bytes in X1 or inequality between X1 and X2.  */

static __always_inline find_t
find_zero_ne_all (op_t x1, op_t x2)
{
  return find_zero_all (x1) | ~find_eq_all (x1, x2);
}

/* Define the "inexact" versions in terms of the exact versions.  */
# define find_zero_low		find_zero_all
# define find_eq_low		find_eq_all
# define find_zero_eq_low	find_zero_eq_all
# define find_zero_ne_low	find_zero_ne_all
#else
# include <sysdeps/generic/string-fza.h>
#endif /* _ARCH_PWR6  */

#endif /* _POWERPC_STRING_FZA_H  */
