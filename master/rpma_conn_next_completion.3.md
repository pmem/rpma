---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_conn\_next\_completion** - receive a completion of an operation

SYNOPSIS
========

          #include <librpma.h>

          int rpma_conn_next_completion(struct rpma_conn *conn,
                  struct rpma_completion *cmpl);

DESCRIPTION
===========

Receive the next available completion of an already posted operation.
All operations are generating completion on error. All operations posted
with the \*\*RPMA\_F\_COMPLETION\_ALWAYS\*\* flag will also generate a
completion on success.

ERRORS
======

**rpma\_conn\_next\_completion**() can fail with the following errors:

-   RPMA\_E\_INVAL - *conn* or *cmpl* is NULL

-   RPMA\_E\_NO\_COMPLETION - no completions available

-   RPMA\_E\_PROVIDER - **ibv\_poll\_cq**(3) failed with a provider
    error

-   RPMA\_E\_UNKNOWN - **ibv\_poll\_cq**(3) failed but no provider error
    is available

-   RPMA\_E\_NOSUPP - not supported opcode
