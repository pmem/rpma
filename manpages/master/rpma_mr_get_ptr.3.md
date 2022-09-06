---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_get_ptr.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_mr\_get\_ptr** - get the pointer to the local memory region

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_local;
          int rpma_mr_get_ptr(const struct rpma_mr_local *mr, void **ptr);

DESCRIPTION
===========

**rpma\_mr\_get\_ptr**() gets a memory region pointer from the local
memory registration object.

RETURN VALUE
============

The **rpma\_mr\_get\_ptr**() function returns 0 on success or a negative
error code on failure. **rpma\_mr\_get\_ptr**() does not set \*ptr value
on failure.

ERRORS
======

**rpma\_mr\_get\_ptr**() can fail with the following error:

-   RPMA\_E\_INVAL - mr or ptr is NULL

SEE ALSO
========

**rpma\_mr\_reg**(3), **librpma**(7) and https://pmem.io/rpma/
