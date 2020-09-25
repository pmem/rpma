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

**rpma\_ep\_shutdown** - stop listening and delete the endpoint

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_ep;
          int rpma_ep_shutdown(struct rpma_ep **ep);

DESCRIPTION
===========

**rpma\_ep\_shutdown**() stops listening for incoming connections and
delete the endpoint.

RETURN VALUE
============

The **rpma\_ep\_shutdown**() function returns 0 on success or a negative
error code on failure. **rpma\_ep\_shutdown**() does not set \*ep value
to NULL on failure.

ERRORS
======

**rpma\_ep\_shutdown**() can fail with the following errors:

-   RPMA\_E\_INVAL - ep is NULL

-   RPMA\_E\_PROVIDER - **rdma\_destroy\_id**(3) failed
