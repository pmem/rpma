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

**rpma\_mr\_remote\_delete** - delete the remote memory region\'s
structure

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_delete(struct rpma_mr_remote **mr_ptr);

DESCRIPTION
===========

**rpma\_mr\_remote\_delete**() deletes the remote memory region\'s
structure.

RETURN VALUE
============

The **rpma\_mr\_remote\_delete**() function returns 0 on success or a
negative error code on failure. **rpma\_mr\_remote\_delete**() does not
set \*mr\_ptr value to NULL on failure.

ERRORS
======

**rpma\_mr\_remote\_delete**() can fail with the following error:

-   RPMA\_E\_INVAL - mr\_ptr is NULL
