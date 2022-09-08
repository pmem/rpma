---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_cfg_get_rcq_size.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_srq\_cfg\_get\_rcq\_size** - get receive CQ size of the shared
RQ

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_srq_cfg;
          int rpma_srq_cfg_get_rcq_size(const struct rpma_srq_cfg *cfg, uint32_t *rcq_size);

DESCRIPTION
===========

**rpma\_srq\_cfg\_get\_rcq\_size**() gets the receive CQ size of the
shared RQ.

RETURN VALUE
============

The **rpma\_srq\_cfg\_get\_rcq\_size**() function returns 0 on success
or a negative error code on failure.
**rpma\_srq\_cfg\_get\_rcq\_size**() does not set \*rcq\_size value on
failure.

ERRORS
======

**rpma\_srq\_cfg\_get\_rcq\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg or rcq\_size is NULL

SEE ALSO
========

**rpma\_srq\_cfg\_new**(3), **rpma\_srq\_cfg\_set\_rcq\_size**(3),
**librpma**(7) and https://pmem.io/rpma/
