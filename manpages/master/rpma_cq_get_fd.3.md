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

**rpma\_cq\_get\_fd** - get the completion file descriptor

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_cq;
          int rpma_cq_get_fd(const struct rpma_cq *cq, int *fd);

DESCRIPTION
===========

**rpma\_cq\_get\_fd**() gets the completion file descriptor of the CQ,
either for the connection\'s main CQ or the receive CQ. For details
please see **rpma\_conn\_get\_cq**(3) and **rpma\_conn\_get\_rcq**(3).

RETURN VALUE
============

The **rpma\_cq\_get\_fd**() function returns 0 on success or a negative
error code on failure. **rpma\_cq\_get\_fd**() does not set \*fd value
on failure.

ERRORS
======

**rpma\_cq\_get\_fd**() can fail with the following error:

-   RPMA\_E\_INVAL - cq or fd is NULL

SEE ALSO
========

**rpma\_conn\_get\_cq**(3), **rpma\_conn\_get\_rcq**(3),
**rpma\_cq\_wait**(3), **rpma\_cq\_get\_completion**(3), **librpma**(7)
and https://pmem.io/rpma/
