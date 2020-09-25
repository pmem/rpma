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

**rpma\_mr\_get\_descriptor\_size** - get size of a memory region
descriptor

SYNOPSIS
========

          #include <librpma.h>

          int rpma_mr_get_descriptor_size(const struct rpma_mr_local *mr,
                          size_t *desc_size);

DESCRIPTION
===========

ERRORS
======

**rpma\_mr\_get\_descriptor\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - *mr* or *desc\_size* is NULL
