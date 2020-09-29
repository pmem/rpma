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

**rpma\_mr\_get\_descriptor** - get a descriptor of a memory region

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_local;
          int rpma_mr_get_descriptor(const struct rpma_mr_local *mr, void *desc);

DESCRIPTION
===========

**rpma\_mr\_get\_descriptor**() writes a network-transferable
description of the provided local memory region. Once the descriptor is
transferred to the other side it can be consumed by
**rpma\_mr\_remote\_from\_descriptor**() to create a remote memory
region\'s structure which allows transferring data between the peers.

RETURN VALUE
============

The **rpma\_mr\_get\_descriptor**() function returns 0 on success or a
negative error code on failure.

ERRORS
======

**rpma\_mr\_get\_descriptor**() can fail with the following error:

-   RPMA\_E\_INVAL - mr or desc is NULL
