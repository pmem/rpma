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

**rpma\_mr\_reg** - create a local memory registration object

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_peer;
          struct rpma_mr_local;

          int rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size,
                  int usage, struct rpma_mr_local **mr_ptr);

DESCRIPTION
===========

**rpma\_mr\_reg**() registers a memory region and creates a local memory
registration object. The usage parameter specifies the operations that
can be performed on the given memory region which should be expressed as
bitwise-inclusive OR of the following:

-   RPMA\_MR\_USAGE\_READ\_SRC - memory used as a source of the read
    operation

-   RPMA\_MR\_USAGE\_READ\_DST - memory used as a destination of the
    read operation

-   RPMA\_MR\_USAGE\_WRITE\_SRC - memory used as a source of the write
    operation

-   RPMA\_MR\_USAGE\_WRITE\_DST - memory used as a destination of the
    write operation

-   RPMA\_MR\_USAGE\_FLUSH\_TYPE\_VISIBILITY - memory with available
    flush operation

-   RPMA\_MR\_USAGE\_FLUSH\_TYPE\_PERSISTENT - memory with available
    persistent flush operation

-   RPMA\_MR\_USAGE\_SEND - memory used for send operation

-   RPMA\_MR\_USAGE\_RECV - memory used for receive operation

RETURN VALUE
============

The **rpma\_mr\_reg**() function returns 0 on success or a negative
error code on failure. **rpma\_mr\_reg**() does not set \*mr\_ptr value
on failure.

ERRORS
======

**rpma\_mr\_reg**() can fail with the following errors:

-   RPMA\_E\_INVAL - peer or ptr or mr\_ptr is NULL

-   RPMA\_E\_INVAL - size equals 0

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_PROVIDER - memory registration failed

SEE ALSO
========

**rpma\_conn\_req\_recv**(3), **rpma\_mr\_dereg**(3),
**rpma\_mr\_get\_descriptor**(3),
**rpma\_mr\_get\_descriptor\_size**(3), **rpma\_peer\_new**(3),
**rpma\_read**(3), **rpma\_recv**(3), **rpma\_send**(3),
**rpma\_write**(3), **rpma\_write\_atomic**(3), **librpma**(7) and
https://pmem.io/rpma/
