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

**rpma\_peer\_new** - create a peer object

SYNOPSIS
========

          #include <librpma.h>

          struct ibv_context;
          struct rpma_peer;
          int rpma_peer_new(struct ibv_context *ibv_ctx, struct rpma_peer **peer);

DESCRIPTION
===========

**rpma\_peer\_new**() creates a new peer object.

RETURN VALUE
============

The **rpma\_peer\_new**() function returns 0 on success or a negative
error code on failure. **rpma\_peer\_new**() does not set \*peer value
on failure.

ERRORS
======

**rpma\_peer\_new**() can fail with the following errors:

-   RPMA\_E\_INVAL - ibv\_ctx or peer is NULL

-   RPMA\_E\_NOMEM - creating a verbs protection domain failed with
    ENOMEM.

-   RPMA\_E\_PROVIDER - creating a verbs protection domain failed with
    error other than ENOMEM.

-   RPMA\_E\_UNKNOWN - creating a verbs protection domain failed without
    error value.

-   RPMA\_E\_NOMEM - out of memory
