---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_delete.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.14.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_srq\_delete** - delete the shared RQ object

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_srq;
          int rpma_conn_req_delete(struct rpma_srq **srq_ptr);

DESCRIPTION
===========

**rpma\_srq\_delete**() deletes the shared RQ object.

RETURN VALUE
============

The **rpma\_srq\_delete**() function returns 0 on success or a negative
error code on failure. **rpma\_srq\_delete**() sets \*srq\_ptr value to
NULL on success and on failure.

ERRORS
======

**rpma\_srq\_delete**() can fail with the following errors:

-   RPMA\_E\_INVAL - srq\_ptr is NULL

-   RPMA\_E\_PROVIDER - **ibv\_destroy\_cq**(3),
    **ibv\_destroy\_comp\_channel**(3), or **ibv\_destroy\_srq**(3)
    failed

SEE ALSO
========

**rpma\_srq\_new**(3), **rpma\_srq\_get\_rcq**(3), **librpma**(7) and
https://pmem.io/rpma/
