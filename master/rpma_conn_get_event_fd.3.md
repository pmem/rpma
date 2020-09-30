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

**rpma\_conn\_get\_event\_fd** - get an event file descriptor of the
connection

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          int rpma_conn_get_event_fd(const struct rpma_conn *conn, int *fd);

DESCRIPTION
===========

**rpma\_conn\_get\_event\_fd**() gets an event file descriptor of the
connection.

RETURN VALUE
============

The **rpma\_conn\_get\_event\_fd**() function returns 0 on success or a
negative error code on failure. **rpma\_conn\_get\_event\_fd**() does
not set \*fd value on failure.

ERRORS
======

**rpma\_conn\_get\_event\_fd**() can fail with the following error:

-   RPMA\_E\_INVAL - conn or fd is NULL

SEE ALSO
========

**rpma\_conn\_next\_event**(3), **rpma\_conn\_req\_connect**(3) and
https://pmem.io/rpma/
