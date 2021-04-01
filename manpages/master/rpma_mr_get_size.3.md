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

**rpma\_mr\_get\_size** - get the size of the local memory region

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_local;
          int rpma_mr_get_size(const struct rpma_mr_local *mr, size_t *size);

DESCRIPTION
===========

**rpma\_mr\_get\_size**() gets a memory region size from the local
memory registration object.

RETURN VALUE
============

The **rpma\_mr\_get\_size**() function returns 0 on success or a
negative error code on failure. **rpma\_mr\_get\_size**() does not set
\*size value on failure.

ERRORS
======

**rpma\_mr\_get\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - mr or size is NULL

SEE ALSO
========

**rpma\_mr\_reg**(3), **librpma**(7) and https://pmem.io/rpma/
