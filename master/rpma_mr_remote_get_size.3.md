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

**rpma\_mr\_remote\_get\_size** - get a remote memory region size

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_get_size(const struct rpma_mr_remote *mr,
                          size_t *size);

DESCRIPTION
===========

**rpma\_mr\_remote\_get\_size**() gets the size of the remote memory
region.

RETURN VALUE
============

The **rpma\_mr\_remote\_get\_size**() function returns 0 on success or a
negative error code on failure. **rpma\_mr\_remote\_get\_size**() does
not set \*size value on failure.

ERRORS
======

**rpma\_mr\_remote\_get\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - mr or size is NULL
