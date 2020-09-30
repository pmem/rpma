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

          struct rpma_peer_cfg;
          int rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg,
                          bool supported);

DESCRIPTION
===========

**rpma\_peer\_cfg\_set\_direct\_write\_to\_pmem**() declares the support
of the direct write to PMEM.

RETURN VALUE
============

The **rpma\_peer\_cfg\_set\_direct\_write\_to\_pmem**() function returns
0 on success or a negative error code on failure.

ERRORS
======

**rpma\_peer\_cfg\_set\_direct\_write\_to\_pmem**() can fail with the
following error:

-   RPMA\_E\_INVAL - pcfg is NULL

SEE ALSO
========

**rpma\_conn\_apply\_remote\_peer\_cfg**(3),
**rpma\_peer\_cfg\_get\_descriptor**(3),
**rpma\_peer\_cfg\_get\_direct\_write\_to\_pmem**(3),
**rpma\_peer\_cfg\_new**(3) and https://pmem.io/rpma/
