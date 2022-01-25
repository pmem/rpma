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

**rpma\_cq\_get\_fd** - get the completion queue\'s file descriptor

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_cq;
          int rpma_cq_get_fd(const struct rpma_cq *cq, int *fd);

DESCRIPTION
===========

**rpma\_cq\_get\_fd**() gets the file descriptor of the completion queue
(CQ in short). When a next completion in the CQ is ready to be consumed
by **rpma\_cq\_get\_completion**(3), the notification is delivered via
the file descriptor. The default mode of the file descriptor is blocking
but it can be changed to non-blocking mode using **fcntl**(2). The CQ is
either the connection\'s main CQ or the receive CQ, please see
**rpma\_conn\_get\_cq**(3) and **rpma\_conn\_get\_rcq**(3) for details.

Note after spotting the notification using the provided file descriptor
you do not have to call **rpma\_cq\_wait**(3) before consuming the
completion but it may cause that the next call to **rpma\_cq\_wait**(3)
will notify you of already consumed completion.

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

**fcntl**(2), **rpma\_conn\_get\_cq**(3), **rpma\_conn\_get\_rcq**(3),
**rpma\_cq\_wait**(3), **rpma\_cq\_get\_completion**(3), **librpma**(7)
and https://pmem.io/rpma/
