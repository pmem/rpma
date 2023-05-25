---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_peer_cfg_from_descriptor.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_peer_cfg_from_descriptor** - create a peer cfg from the
descriptor

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer_cfg;
          int rpma_peer_cfg_from_descriptor(const void *desc, size_t desc_size,
                          struct rpma_peer_cfg **pcfg_ptr);

# DESCRIPTION

**rpma_peer_cfg_from_descriptor**() creates a peer configuration object
from the descriptor.

# SECURITY WARNING

An attacker might modify the serialized remote node configuration while
it is transferred via an unsecured connection (e.g. rdma_cm private
data), which might cause different remote persistency method selections.
The most dangerous situation is switching from the GPSPM mode to the APM
one. Users should avoid using **rpma_conn_get_private_data**(3) and
**rpma_conn_req_get_private_data**(3) API calls and they should utilize
TLS/SSL connections to transfer all configuration data between peers
instead.

# RETURN VALUE

The **rpma_peer_cfg_from_descriptor**() function returns 0 on success or
a negative error code on failure. **rpma_peer_cfg_from_descriptor**()
does not set \*pcfg_ptr value on failure.

# ERRORS

**rpma_peer_cfg_from_descriptor**() can fail with the following errors:

-   RPMA_E\_INVAL - desc or pcfg_ptr are NULL

-   RPMA_E\_NOMEM - out of memory

# SEE ALSO

**rpma_conn_apply_remote_peer_cfg**(3),
**rpma_peer_cfg_get_descriptor**(3), **rpma_peer_cfg_new**(3),
**librpma**(7) and https://pmem.io/rpma/
