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

**rpma\_peer\_cfg\_delete** - delete the peer configuration object

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_peer_cfg;
          int rpma_peer_cfg_delete(struct rpma_peer_cfg **pcfg_ptr);

DESCRIPTION
===========

**rpma\_peer\_cfg\_delete**() deletes the peer configuration object.

RETURN VALUE
============

The **rpma\_peer\_cfg\_delete**() function returns 0 on success or a
negative error code on failure. **rpm\_peer\_cfg\_delete**() does not
set \*pcfg\_ptr value to NULL on failure.

ERRORS
======

**rpma\_peer\_cfg\_delete**() can fail with the following error:

-   RPMA\_E\_INVAL - pcfg\_ptr is NULL
