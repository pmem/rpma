---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_get_qp_num.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.11.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_conn\_get\_qp\_num** - get the connection\'s qp\_num

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          int rpma_conn_get_qp_num(const struct rpma_conn *conn,
                          uint32_t *qp_num);

DESCRIPTION
===========

**rpma\_conn\_get\_qp\_num**() obtains the unique identifier of the
connection.

RETURN VALUE
============

The **rpma\_conn\_get\_qp\_num**() function returns 0 on success or a
negative error code on failure. **rpma\_conn\_get\_qp\_num**() does not
set \*qp\_num value on failure.

ERRORS
======

**rpma\_conn\_get\_qp\_num**() can fail with the following error:

-   RPMA\_E\_INVAL - conn or qp\_num is NULL

SEE ALSO
========

**rpma\_conn\_req\_new**(3), **rpma\_ep\_next\_conn\_req**(3),
**rpma\_conn\_req\_connect**(3), **librpma**(7) and
https://pmem.io/rpma/
