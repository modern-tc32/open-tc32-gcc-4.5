/* Darwin host-specific hook definitions.
   Copyright (C) 2003, 2004, 2005, 2007 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include <sys/mman.h>
#include "hosthooks.h"
#include "hosthooks-def.h"
#include "toplev.h"
#include "config/host-darwin.h"

/* Return the address of the PCH address space, if the PCH will fit in it.  */

void *
darwin_gt_pch_get_address (size_t sz, int fd ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Check ADDR and SZ for validity, and deallocate (using munmap) that part of
   pch_address_space beyond SZ.  */

int
darwin_gt_pch_use_address (void *addr, size_t sz, int fd, size_t off)
{
  void *mapped_addr;

  mapped_addr = mmap (addr, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED,
                      fd, off);
  if (mapped_addr == addr)
    return 1;

  if (mapped_addr != MAP_FAILED)
    munmap (mapped_addr, sz);

  return 0;
}

const struct host_hooks host_hooks = HOST_HOOKS_INITIALIZER;
