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

**rpma\_conn\_cfg\_set\_sq\_size** - set SQ size for the connection

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg,
                          uint32_t sq_size);

DESCRIPTION
===========

**rpma\_conn\_cfg\_set\_sq\_size**() sets the SQ size for the
connection.

RETURN VALUE
============

The **rpma\_conn\_cfg\_set\_sq\_size**() function returns 0 on success
or a negative error code on failure.

ERRORS
======

**rpma\_conn\_cfg\_set\_sq\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg is NULL
