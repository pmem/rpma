---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_mr\_reg** - create a local memory registration object

SYNOPSIS
========

          #include <librpma.h>

          int rpma_mr_reg(const struct rpma_peer *peer, void *ptr, size_t size,
                  int usage, struct rpma_mr_local **mr_ptr);

DESCRIPTION
===========

**rpma\_mr\_reg**() registers a memory region and creates a local memory
registration object.

ERRORS
======

**rpma\_mr\_reg**() can fail with the following errors:

-   RPMA\_E\_INVAL - *peer* or *ptr* or *mr\_ptr* is NULL

-   RPMA\_E\_INVAL - *size* equals 0

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_PROVIDER - memory registration failed
