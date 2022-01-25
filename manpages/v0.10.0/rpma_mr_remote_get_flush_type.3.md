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

**rpma\_mr\_remote\_get\_flush\_type** - get a remote memory region\'s
flush types

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr,
                          int *flush_type);

DESCRIPTION
===========

**rpma\_mr\_remote\_get\_flush\_type**() gets flush types supported by
the remote memory region.

RETURN VALUE
============

The **rpma\_mr\_remote\_get\_flush\_type**() function returns 0 on
success or a negative error code on failure.
**rpma\_mr\_remote\_get\_flush\_type**() does not set \*flush\_type
value on failure.

ERRORS
======

**rpma\_mr\_remote\_get\_flush\_type**() can fail with the following
error:

-   RPMA\_E\_INVAL - mr or flush\_type is NULL

SEE ALSO
========

**rpma\_mr\_remote\_from\_descriptor**(3), **librpma**(7) and
https://pmem.io/rpma/
