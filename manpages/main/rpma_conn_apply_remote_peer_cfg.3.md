---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_apply_remote_peer_cfg.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_apply_remote_peer_cfg** - apply remote peer cfg to the
connection

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_peer_cfg;
          int rpma_conn_apply_remote_peer_cfg(struct rpma_conn *conn,
                          const struct rpma_peer_cfg *pcfg);

# DESCRIPTION

**rpma_conn_apply_remote_peer_cfg**() applies the remote peer
configuration to the connection.

# RETURN VALUE

The **rpma_conn_apply_remote_peer_cfg**() function returns 0 on success
or a negative error code on failure.
**rpma_conn_apply_remote_peer_cfg**() does not set \*pcfg value on
failure.

# ERRORS

**rpma_conn_apply_remote_peer_cfg**() can fail with the following error:

-   RPMA_E\_INVAL - conn or pcfg are NULL

# SEE ALSO

**rpma_conn_req_connect**(3), **rpma_peer_cfg_new**(3), **librpma**(7)
and https://pmem.io/rpma/
