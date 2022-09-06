---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_new.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_srq\_new** - create a new shared RQ object

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_peer;
          struct rpma_srq_cfg;
          struct rpma_srq;
          int rpma_srq_new(struct rpma_peer *peer, const struct rpma_srq_cfg *cfg,
                          struct rpma_srq **srq_ptr);

DESCRIPTION
===========

**rpma\_srq\_new**() creates a new shared RQ object including a new
shared RQ and a new shared receive CQ. It does not create the shared
receive CQ if the size of the receive CQ in cfg equals 0.

RETURN VALUE
============

The **rpma\_srq\_new**() function returns 0 on success or a negative
error code on failure. **rpma\_srq\_new**() does not set \*srq\_ptr
value on failure. If cfg is NULL, then the default values are used

-   see **rpma\_srq\_cfg\_new**(3) for more details.

ERRORS
======

**rpma\_srq\_new**() can fail with the following errors:

-   RPMA\_E\_INVAL - peer or srq\_ptr is NULL

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_PROVIDER - **ibv\_create\_srq**(3),
    **ibv\_create\_comp\_channel**(3), **ibv\_create\_cq**(3) or
    **ibv\_req\_notify\_cq**(3) failed

SEE ALSO
========

**rpma\_srq\_delete**(3), **rpma\_srq\_get\_rcq**(3), **librpma**(7) and
https://pmem.io/rpma/
