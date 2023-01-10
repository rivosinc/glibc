/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <string.h>
#include <string-fzb.h>
#include <string-fzc.h>
#include <string-fzi.h>
#include <string-maskoff.h>
#include <libc-pointer-arith.h>

#ifdef STRLEN
# define __strlen STRLEN
#endif

/* Return the length of the null-terminated string STR.  Scan for
   the null terminator quickly by testing four bytes at a time.  */
size_t
__strlen (const char *str)
{
  /* Align pointer to sizeof op_t.  */
  const uintptr_t s_int = (uintptr_t) str;
  const op_t *word_ptr = (const op_t*) PTR_ALIGN_DOWN (str, sizeof (op_t));

  /* Read and MASK the first word. */
  op_t word = *word_ptr | create_mask (s_int);

  while (! has_zero (word))
    word = *++word_ptr;

  return ((const char *) word_ptr) + index_first_zero (word) - str;
}
#ifndef STRLEN
weak_alias (__strlen, strlen)
libc_hidden_builtin_def (strlen)
#endif
