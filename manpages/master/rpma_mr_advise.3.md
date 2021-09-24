---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.9.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_mr\_advise** - give advice about an address range in a memory
registration

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_local *mr;
          int rpma_mr_advise(struct rpma_mr_local *mr, size_t offset, size_t len,
                  int advice, uint32_t flags);

DESCRIPTION
===========

**rpma\_mr\_advise**() gives advice about an address range in a memory
registration. The usage parameter specifies the operations that can be
performed on the given memory address range. For available advice and
flags values please see **ibv\_advise\_mr**(3).

RETURN VALUE
============

The **rpma\_mr\_advise**() function returns 0 on success or a negative
error code on failure.

ERRORS
======

**rpma\_mr\_advise**() can fail with the following errors:

-   RPMA\_E\_INVAL - in one of the following:

    -   the requested range is out of the memory registration bounds

    -   the memory registration usage does not allow the specific advice

    -   the flags are invalid

-   RPMA\_E\_NOSUPP - the operation is not supported by the system

-   RPMA\_E\_PROVIDER - **ibv\_mr\_advise**(3) failed for other errors

SEE ALSO
========

**rpma\_mr\_reg**(3), **ibv\_mr\_advise**(3), **librpma**(7) and
https://pmem.io/rpma/
