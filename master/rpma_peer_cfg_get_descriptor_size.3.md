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

**rpma\_peer\_cfg\_get\_descriptor\_size \-- get size of the peer cfg
descriptor**

SYNOPSIS
========

          #include <librpma.h>

          const struct rpma_peer_cfg;
          int rpma_peer_cfg_get_descriptor_size(const struct rpma_peer_cfg *pcfg,
                          size_t *desc_size);

DESCRIPTION
===========

**rpma\_peer\_cfg\_get\_descriptor\_size() gets size of the peer
configuration** descriptor.

RETURN VALUE
============

The **rpma\_peer\_cfg\_get\_descriptor\_size() function returns 0 on
success or** a negative error code on failure.

ERRORS
======

**rpma\_peer\_cfg\_get\_descriptor\_size() can fail with the following
error:**

-   RPMA\_E\_INVAL - pcfg or desc\_size is NULL
