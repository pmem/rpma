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

**rpma\_peer\_cfg\_get\_descriptor** - get the descriptor of the peer
configuration

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_peer_cfg;
          int rpma_peer_cfg_get_descriptor(const struct rpma_peer_cfg *pcfg,
                          void *desc);

DESCRIPTION
===========

**rpma\_peer\_cfg\_get\_descriptor**() gets the descriptor of the peer
configuration.

RETURN VALUE
============

The **rpma\_peer\_cfg\_get\_descriptor**() function returns 0 on success
or a negative error code on failure.

ERRORS
======

**rpma\_peer\_cfg\_get\_descriptor**() can fail with the following
error:

-   RPMA\_E\_INVAL - pcfg or desc are NULL

SEE ALSO
========

**rpma\_peer\_cfg\_from\_descriptor**(3), **rpma\_peer\_cfg\_new**(3),
**librpma**(7) and https://pmem.io/rpma/
