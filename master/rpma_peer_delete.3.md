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

**rpma\_peer\_delete** - delete a peer object

SYNOPSIS
========

          #include <librpma.h>

          int rpma_peer_delete(struct rpma_peer **peer);

DESCRIPTION
===========

RETURN VALUE
============

The **rpma\_peer\_delete**() function returns 0 on success or a negative
error code on failure. **rpma\_peer\_delete**() does not set \**peer* to
NULL on failure.

ERRORS
======

**rpma\_peer\_delete**() can fail with the following error:

-   RPMA\_E\_PROVIDER - deleting the verbs protection domain failed.
