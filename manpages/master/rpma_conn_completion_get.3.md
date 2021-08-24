---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.9.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_conn\_completion\_get** - receive a completion of an operation

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_completion;

          int rpma_conn_completion_get(struct rpma_conn *conn,
                          struct rpma_completion *cmpl);

DESCRIPTION
===========

**rpma\_conn\_completion\_get**() receives the next available completion
of an already posted operation from the connection\'s main CQ one can
access directly using **rpma\_conn\_get\_cq**(3). Please see
**rpma\_cq\_get\_completion**(3) for details.

RETURN VALUE
============

The **rpma\_conn\_completion\_get**() function returns 0 on success or a
negative error code on failure.

ERRORS
======

**rpma\_conn\_completion\_get**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn or cmpl is NULL

-   Other errors - please see **rpma\_cq\_get\_completion**(3)

SEE ALSO
========

**rpma\_conn\_get\_completion\_fd**(3),
**rpma\_conn\_completion\_wait**(3), **rpma\_conn\_req\_connect**(3),
**rpma\_conn\_get\_cq**(3), **rpma\_conn\_get\_rcq**(3),
**rpma\_flush**(3), **rpma\_read**(3), **rpma\_recv**(3),
**rpma\_send**(3), **rpma\_send\_with\_imm**(3), **rpma\_write**(3),
**rpma\_write\_with\_imm**(3), **rpma\_write\_atomic**(3),
**librpma**(7) and https://pmem.io/rpma/
