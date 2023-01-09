---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_cfg_get_compl_channel.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_cfg_get_compl_channel** - get if the completion event
channel can be shared by CQ and RCQ

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_get_compl_channel(const struct rpma_conn_cfg *cfg, bool *shared);

# DESCRIPTION

**rpma_conn_cfg_get_compl_channel**() gets if the completion event
channel can be shared by CQ and RCQ.

# RETURN VALUE

The **rpma_conn_cfg_get_compl_channel**() function returns 0 on success
or a negative error code on failure.
**rpma_conn_cfg_get_compl_channel**() does not set \*shared value on
failure.

# ERRORS

**rpma_conn_cfg_get_compl_channel**() can fail with the following error:

-   RPMA_E\_INVAL - cfg or shared is NULL

# SEE ALSO

**rpma_conn_cfg_new**(3), **rpma_conn_wait**(3),
**rpma_conn_cfg_set_compl_channel**(3), **librpma**(7) and
https://pmem.io/rpma/
