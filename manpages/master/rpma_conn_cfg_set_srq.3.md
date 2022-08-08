---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_set_srq.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.14.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_conn\_cfg\_set\_srq** - set a shared RQ object for the
connection

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn_cfg;
          struct rpma_srq;
          int rpma_conn_cfg_set_srq(struct rpma_conn_cfg *cfg, struct rpma_srq *srq);

DESCRIPTION
===========

**rpma\_conn\_cfg\_set\_srq**() sets a shared RQ object for the
connection. If this function is not called, the srq has the default
value (NULL) set by **rpma\_conn\_cfg\_new**(3).

RETURN VALUE
============

The **rpma\_conn\_cfg\_set\_srq**() function returns 0 on success or a
negative error code on failure.

ERRORS
======

**rpma\_conn\_cfg\_set\_srq**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg is NULL

SEE ALSO
========

**rpma\_conn\_cfg\_new**(3), **rpma\_conn\_cfg\_get\_srq**(3),
**librpma**(7) and https://pmem.io/rpma/
