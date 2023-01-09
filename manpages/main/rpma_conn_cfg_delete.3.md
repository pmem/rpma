---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_delete.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_cfg_delete** - delete the connection configuration object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr);

# DESCRIPTION

**rpma_conn_cfg_delete**() deletes the connection configuration object.

# RETURN VALUE

The **rpma_conn_cfg_delete**() function returns 0 on success or a
negative error code on failure. **rpma_conn_cfg_delete**() sets
\*cfg_ptr value to NULL on success and on failure.

# ERRORS

**rpma_conn_cfg_delete**() can fail with the following error:

-   RPMA_E\_INVAL - cfg_ptr is NULL

# SEE ALSO

**rpma_conn_cfg_new**(3), **librpma**(7) and https://pmem.io/rpma/
