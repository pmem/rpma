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

**rpma\_conn\_disconnect** - initialize disconnection

SYNOPSIS
========

          #include <librpma.h>

          const struct rpma_conn;
          int rpma_conn_disconnect(const struct rpma_conn *conn);

DESCRIPTION
===========

**rpma\_conn\_disconnect**() initializes disconnecting of the RPMA
connection process.

RETURN VALUE
============

The **rpma\_conn\_disconnect**() function returns 0 on success or a
negative error code on failure.

ERRORS
======

**rpma\_conn\_disconnect**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn is NULL

-   RPMA\_E\_PROVIDER - **rdma\_disconnect**() failed
