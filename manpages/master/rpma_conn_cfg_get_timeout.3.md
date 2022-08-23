---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_get_timeout.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.0.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_conn\_cfg\_get\_timeout** - get connection establishment timeout

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_get_timeout(const struct rpma_conn_cfg *cfg,
                          int *timeout_ms);

DESCRIPTION
===========

**rpma\_conn\_cfg\_get\_timeout**() gets the connection establishment
timeout.

RETURN VALUE
============

The **rpma\_conn\_cfg\_get\_timeout**() function returns 0 on success or
a negative error code on failure. **rpma\_conn\_cfg\_get\_timeout**()
does not set \*timeout\_ms value on failure.

ERRORS
======

**rpma\_conn\_cfg\_get\_timeout**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg or timeout\_ms is NULL

SEE ALSO
========

**rpma\_conn\_cfg\_new**(3), **rpma\_conn\_cfg\_set\_timeout**(3),
**librpma**(7) and https://pmem.io/rpma/
