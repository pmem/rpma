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

**rpma\_conn\_get\_private\_data** - obtain a pointer to the
connection\'s

SYNOPSIS
========

          #include <librpma.h>

          int rpma_conn_get_private_data(const struct rpma_conn *conn,
                          struct rpma_conn_private_data *pdata);

DESCRIPTION
===========

private data

Obtain a pointer to the private data given by the other side of the
connection.

ERRORS
======

**rpma\_conn\_get\_private\_data**() can fail with the following error:

-   RPMA\_E\_INVAL - *conn* or *pdata* is NULL
