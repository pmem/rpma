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

**rpma\_conn\_cfg\_set\_timeout** - set connection establishment timeout

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg,
                          int timeout_ms);

DESCRIPTION
===========

**rpma\_conn\_cfg\_set\_timeout**() sets the connection establishment
timeout.

RETURN VALUE
============

The **rpma\_conn\_cfg\_set\_timeout**() function returns 0 on success or
a negative error code on failure.

ERRORS
======

**rpma\_conn\_cfg\_set\_timeout**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg is NULL or timeout\_ms \< 0

SEE ALSO
========

**rpma\_conn\_cfg\_new**(3), **rpma\_conn\_cfg\_get\_timeout**(3),
**librpma**(7) and https://pmem.io/rpma/
