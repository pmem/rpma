---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_set_timeout.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_cfg_set_timeout** - set connection establishment timeout

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg, int timeout_ms);

# DESCRIPTION

**rpma_conn_cfg_set_timeout**() sets the connection establishment
timeout. If this function is not called, the timeout has the default
value (1000) set by **rpma_conn_cfg_new**(3).

# RETURN VALUE

The **rpma_conn_cfg_set_timeout**() function returns 0 on success or a
negative error code on failure.

# ERRORS

**rpma_conn_cfg_set_timeout**() can fail with the following error:

-   RPMA_E\_INVAL - cfg is NULL or timeout_ms \< 0

# SEE ALSO

**rpma_conn_cfg_new**(3), **rpma_conn_cfg_get_timeout**(3),
**librpma**(7) and https://pmem.io/rpma/
