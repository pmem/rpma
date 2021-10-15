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

**rpma\_conn\_completion\_wait** - wait for a completion (deprecated)

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          int rpma_conn_completion_wait(struct rpma_conn *conn);

DESCRIPTION
===========

**rpma\_conn\_completion\_wait**() waits for an incoming completion. If
it succeeds the completion can be collected using
**rpma\_conn\_completion\_get**().

RETURN VALUE
============

The **rpma\_conn\_completion\_wait**() function returns 0 on success or
a negative error code on failure.

ERRORS
======

**rpma\_conn\_completion\_wait**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn is NULL

-   RPMA\_E\_PROVIDER - **ibv\_req\_notify\_cq**(3) failed with a
    provider error

-   RPMA\_E\_NO\_COMPLETION - no completions available

DEPRECATED
==========

Please use **rpma\_conn\_get\_cq**(3) and **rpma\_cq\_wait**(3) instead.

SEE ALSO
========

**rpma\_conn\_get\_completion\_fd**(3),
**rpma\_conn\_completion\_get**(3), **rpma\_conn\_req\_connect**(3),
**librpma**(7) and https://pmem.io/rpma/
