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

**rpma\_mr\_remote\_from\_descriptor** - create a memory region from a
descriptor

SYNOPSIS
========

          #include <librpma.h>

          int rpma_mr_remote_from_descriptor(
           const struct rpma_mr_descriptor *desc,
           struct rpma_mr_remote **mr_ptr);

DESCRIPTION
===========

Create a remote memory region\'s structure based on the provided
descriptor with a network-transferable description of the memory region
local to the remote peer.

ERRORS
======

**rpma\_mr\_remote\_from\_descriptor**() can fail with the following
errors:

-   RPMA\_E\_INVAL - *desc* or *mr\_ptr* is NULL

-   RPMA\_E\_NOSUPP - deserialized information does not represent a
    valid memory region

-   RPMA\_E\_NOMEM - out of memory
