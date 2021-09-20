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

**rpma\_conn\_get\_cq** - get the connection\'s main CQ

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_cq;
          int rpma_conn_get_cq(const struct rpma_conn *conn,
                          struct rpma_cq **cq_ptr);

DESCRIPTION
===========

**rpma\_conn\_get\_cq**() gets the main CQ from the connection. When the
receive CQ is not present the main CQ allows handling all completions
within the connection. When the receive CQ is present the main CQ allows
handling all completions except **rpma\_recv**(3) completions within the
connection. Please see **rpma\_conn\_get\_rcq**(3) for details about the
receive CQ.

RETURN VALUE
============

The **rpma\_conn\_get\_cq**() function returns 0 on success or a
negative error code on failure. **rpma\_conn\_get\_cq**() does not set
\*cq\_ptr value on failure.

ERRORS
======

**rpma\_conn\_get\_cq**() can fail with the following error:

-   RPMA\_E\_INVAL - conn or cq\_ptr is NULL

SEE ALSO
========

**rpma\_conn\_req\_connect**(3), **rpma\_conn\_get\_rcq**(3),
**rpma\_cq\_wait**(3), **rpma\_cq\_get\_completion**(3),
**rpma\_cq\_get\_fd**(3), **rpma\_recv**(3), **librpma**(7) and
https://pmem.io/rpma/
