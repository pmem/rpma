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

**rpma\_mr\_remote\_delete** - delete a remote memory region\'s
structure

SYNOPSIS
========

          #include <librpma.h>

          int rpma_mr_remote_delete(struct rpma_mr_remote **mr_ptr);

DESCRIPTION
===========

ERRORS
======

**rpma\_mr\_remote\_delete**() can fail with the following error:

-   RPMA\_E\_INVAL - *mr\_ptr* is NULL
