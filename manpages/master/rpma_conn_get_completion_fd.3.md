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

**rpma\_conn\_get\_completion\_fd** - get the completion file descriptor
(deprecated)

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          int rpma_conn_get_completion_fd(const struct rpma_conn *conn, int *fd);

DESCRIPTION
===========

**rpma\_conn\_get\_completion\_fd**() gets the completion file
descriptor of the connection. It is the same file descriptor as the one
returned by the **rpma\_cq\_get\_fd**(3) for the connection\'s main CQ
available via **rpma\_conn\_get\_cq**(3).

RETURN VALUE
============

The **rpma\_conn\_get\_completion\_fd**() function returns 0 on success
or a negative error code on failure.
**rpma\_conn\_get\_completion\_fd**() does not set \*fd value on
failure.

ERRORS
======

**rpma\_conn\_get\_completion\_fd**() can fail with the following error:

-   RPMA\_E\_INVAL - conn or fd is NULL

DEPRECATED
==========

Please use **rpma\_conn\_get\_cq**(3) and **rpma\_cq\_get\_fd**(3)
instead.

SEE ALSO
========

**rpma\_conn\_completion\_get**(3), **rpma\_conn\_completion\_wait**(3),
**rpma\_conn\_req\_connect**(3), **librpma**(7) and
https://pmem.io/rpma/
