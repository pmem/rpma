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

**rpma\_conn\_get\_completion\_fd** - get a completion file descriptor

SYNOPSIS
========

          #include <librpma.h>

          int rpma_conn_get_completion_fd(struct rpma_conn *conn, int *fd);

DESCRIPTION
===========

Get a completion file descriptor of the connection.

ERRORS
======

**rpma\_conn\_get\_completion\_fd**() can fail with the following error:

-   RPMA\_E\_INVAL - *conn* or *fd* is NULL
