---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_ep_get_fd.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.9.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_ep\_get\_fd** - get a file descriptor of the endpoint

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_ep;
          int rpma_ep_get_fd(const struct rpma_ep *ep, int *fd);

DESCRIPTION
===========

**rpma\_ep\_get\_fd**() gets the file descriptor of the endpoint.

RETURN VALUE
============

The **rpma\_ep\_get\_fd**() function returns 0 on success or a negative
error code on failure. **rpma\_ep\_get\_fd**() does not set \*fd value
on failure.

ERRORS
======

**rpma\_ep\_get\_fd**() can fail with the following error:

-   RPMA\_E\_INVAL - ep or fd is NULL

SEE ALSO
========

**rpma\_ep\_listen**(3), **librpma**(7) and https://pmem.io/rpma/
