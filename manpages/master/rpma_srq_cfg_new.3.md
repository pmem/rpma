---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_cfg_new.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.14.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_srq\_cfg\_new** - create a new shared RQ configuration object

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_srq_cfg;
          int rpma_srq_cfg_new(struct rpma_srq_cfg **cfg_ptr);

DESCRIPTION
===========

**rpma\_srq\_cfg\_new**() creates a new shared RQ configuration object
and fills it with the default values:

            .rcq_size = 100
            .rq_size = 100

Note that **rpma\_srq\_new**(3) with the default rcq\_size creates its
own receive CQ.

RETURN VALUE
============

The **rpma\_srq\_cfg\_new**() function returns 0 on success or a
negative error code on failure. **rpma\_srq\_cfg\_new**() does not set
\*cfg\_ptr value on failure.

ERRORS
======

**rpma\_srq\_cfg\_new**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg\_ptr is NULL

-   RPMA\_E\_NOMEM - out of memory

SEE ALSO
========

**rpma\_srq\_cfg\_delete**(3), **rpma\_srq\_cfg\_get\_rcq\_size**(3),
**rpma\_srq\_cfg\_get\_rq\_size**(3),
**rpma\_srq\_cfg\_set\_rcq\_size**(3),
**rpma\_srq\_cfg\_set\_rq\_size**(3), **rpma\_srq\_new**(3),
**librpma**(7) and https://pmem.io/rpma/
