---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_utils_conn_event_2str.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_utils_conn_event_2str** - convert RPMA_CONN\_\* enum to a string

# SYNOPSIS

          #include <librpma.h>

          const char *rpma_utils_conn_event_2str(enum rpma_conn_event conn_event);

          enum rpma_conn_event {
                  RPMA_CONN_UNDEFINED = -1,
                  RPMA_CONN_ESTABLISHED,
                  RPMA_CONN_CLOSED,
                  RPMA_CONN_LOST,
                  RPMA_CONN_REJECTED,
                  RPMA_CONN_UNREACHABLE
          };

# DESCRIPTION

**rpma_utils_conn_event_2str**() converts RPMA_CONN\_\* enum to the
const string representation.

# RETURN VALUE

The **rpma_utils_conn_event_2str**() function returns the const string
representation of RPMA_CONN\_\* enums.

# ERRORS

**rpma_utils_conn_event_2str**() can not fail.

# SEE ALSO

**rpma_conn_next_event**(3), **librpma**(7) and https://pmem.io/rpma/
