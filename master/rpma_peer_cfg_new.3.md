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

**rpma\_peer\_cfg\_new** - create a new peer configuration object

SYNOPSIS
========

          #include <librpma.h>

          int rpma_peer_cfg_new(struct rpma_peer_cfg **pcfg_ptr);

DESCRIPTION
===========

ERRORS
======

**rpma\_peer\_cfg\_new**() can fail with the following errors:

-   RPMA\_E\_INVAL - *pcfg\_ptr* is NULL

-   RPMA\_E\_NOMEM - out of memory
