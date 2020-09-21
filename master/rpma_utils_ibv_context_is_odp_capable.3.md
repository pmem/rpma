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

**rpma\_utils\_ibv\_context\_is\_odp\_capable** - is On-Demand Paging
supported

SYNOPSIS
========

          #include <librpma.h>

          struct ibv_context;
          int rpma_utils_ibv_context_is_odp_capable(struct ibv_context *dev,
                  int *is_odp_capable);

DESCRIPTION
===========

**rpma\_utils\_ibv\_context\_is\_odp\_capable**() queries the RDMA
device context\'s capabilities and check if it supports On-Demand
Paging.

RETURN VALUE
============

The **rpma\_utils\_ibv\_context\_is\_odp\_capable**() function returns 0
on success or a negative error code on failure. The \*is\_odp\_capable
value on failure is undefined.

ERRORS
======

**rpma\_utils\_ibv\_context\_is\_odp\_capable**() can fail with the
following errors:

-   RPMA\_E\_INVAL - dev or is\_odp\_capable is NULL

-   RPMA\_E\_PROVIDER - **ibv\_query\_device\_ex**() failed, errno can
    be checked using **rpma\_err\_get\_provider\_error**()
