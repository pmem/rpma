---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_remote_get_flush_type.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.0.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_mr\_remote\_get\_flush\_type** - get a remote memory region\'s
flush types

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr,
                          int *flush_type);

DESCRIPTION
===========

**rpma\_mr\_remote\_get\_flush\_type**() gets flush types supported by
the remote memory region.

RETURN VALUE
============

The **rpma\_mr\_remote\_get\_flush\_type**() function returns 0 on
success or a negative error code on failure.
**rpma\_mr\_remote\_get\_flush\_type**() does not set \*flush\_type
value on failure.

ERRORS
======

**rpma\_mr\_remote\_get\_flush\_type**() can fail with the following
error:

-   RPMA\_E\_INVAL - mr or flush\_type is NULL

SEE ALSO
========

**rpma\_mr\_remote\_from\_descriptor**(3), **librpma**(7) and
https://pmem.io/rpma/
