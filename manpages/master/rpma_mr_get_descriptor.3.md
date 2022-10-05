---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_get_descriptor.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_mr\_get\_descriptor** - get a descriptor of a memory region

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_mr_local;
          int rpma_mr_get_descriptor(const struct rpma_mr_local *mr, void *desc);

DESCRIPTION
===========

**rpma\_mr\_get\_descriptor**() writes a network-transferable
description of the provided local memory region (called \'descriptor\').
Once the descriptor is transferred to the other side it should be
decoded by **rpma\_mr\_remote\_from\_descriptor**() to create a remote
memory region\'s structure which allows for Remote Memory Access. Please
see **librpma**(7) for details.

SECURITY WARNING
================

See **rpma\_mr\_remote\_from\_descriptor**(3).

RETURN VALUE
============

The **rpma\_mr\_get\_descriptor**() function returns 0 on success or a
negative error code on failure.

ERRORS
======

**rpma\_mr\_get\_descriptor**() can fail with the following error:

-   RPMA\_E\_INVAL - mr or desc is NULL

SEE ALSO
========

**rpma\_mr\_get\_descriptor\_size**(3), **rpma\_mr\_reg**(3),
**librpma**(7) and https://pmem.io/rpma/
