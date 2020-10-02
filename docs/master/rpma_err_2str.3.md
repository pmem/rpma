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

**rpma\_err\_2str** - convert RPMA error code to a string

SYNOPSIS
========

          #include <librpma.h>

          const char *rpma_err_2str(int ret);

DESCRIPTION
===========

**rpma\_err\_2str**() returns the const string representation of RPMA
error codes.

ERRORS
======

**rpma\_err\_2str**() can not fail.

SEE ALSO
========

**librpma**(7) and https://pmem.io/rpma/
