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

**rpma\_peer\_cfg\_set\_direct\_write\_to\_pmem** - declare direct write
to PMEM support

SYNOPSIS
========

          #include <librpma.h>

          int rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg,
                          bool supported);

DESCRIPTION
===========

ERRORS
======

**rpma\_peer\_cfg\_set\_direct\_write\_to\_pmem**() can fail with the
following error:

-   RPMA\_E\_INVAL - *pcfg* is NULL
