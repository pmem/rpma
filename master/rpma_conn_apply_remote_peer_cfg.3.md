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

**rpma\_conn\_apply\_remote\_peer\_cfg** - apply remote peer cfg to the
connection

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          const struct rpma_peer_cfg;
          int rpma_conn_apply_remote_peer_cfg(struct rpma_conn *conn,
                          const struct rpma_peer_cfg *pcfg);

DESCRIPTION
===========

**rpma\_conn\_apply\_remote\_peer\_cfg**() applies the remote peer
configuration to the connection.

RETURN VALUE
============

The **rpma\_conn\_apply\_remote\_peer\_cfg**() function returns 0 on
success or a negative error code on failure.
**rpma\_conn\_apply\_remote\_peer\_cfg**() does not set \*pcfg value on
failure.

ERRORS
======

**rpma\_conn\_apply\_remote\_peer\_cfg**() can fail with the following
error:

-   RPMA\_E\_INVAL - conn or pcfg are NULL
