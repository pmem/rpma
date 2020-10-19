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

**rpma\_conn\_cfg\_delete** - delete the connection configuration object

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr);

DESCRIPTION
===========

**rpma\_conn\_cfg\_delete**() deletes the connection configuration
object.

RETURN VALUE
============

The **rpma\_conn\_cfg\_delete**() function returns 0 on success or a
negative error code on failure. **rpma\_conn\_cfg\_delete**() sets
\*cfg\_ptr value to NULL on success and on failure.

ERRORS
======

**rpma\_conn\_cfg\_delete**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg\_ptr is NULL

SEE ALSO
========

**rpma\_conn\_cfg\_new**(3), **librpma**(7) and https://pmem.io/rpma/
