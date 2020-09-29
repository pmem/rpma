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

**librpma** - remote persistent memory access library

SYNOPSIS
========

          #include <librpma.h>
          cc ``` -lrpma

DESCRIPTION
===========

librpma is a C library to simplify accessing persistent memory devices
on remote hosts over Remote Direct Memory Access (RDMA).

Elaborate XXX

REMOTE MEMORY ACCESS
====================

-   rpma\_read - XXX

-   rpma\_write - XXX

-   rpma\_write\_atomic - XXX

-   rpma\_flush - XXX

DIRECT WRITE TO PMEM
====================

Elaborate XXX

-   rpma\_peer\_cfg\_set\_direct\_write\_to\_pmem - XXX

-   rpma\_peer\_cfg\_get\_descriptor - XXX

-   rpma\_peer\_cfg\_from\_descriptor - XXX

-   rpma\_conn\_apply\_remote\_peer\_cfg - XXX

CLIENT OPERATION
================

Elaborate XXX

-   rpma\_conn\_req\_new - XXX

-   rpma\_conn\_req\_connect - XXX

-   rpma\_conn\_next\_event - XXX

-   rpma\_conn\_disconnect - XXX

-   rpma\_conn\_next\_event - XXX

-   rpma\_conn\_delete - XXX

SERVER OPERATION
================

Elaborate XXX

-   rpma\_ep\_listen - XXX

-   rpma\_ep\_next\_conn\_req - XXX

-   rpma\_conn\_req\_connect - XXX

-   rpma\_conn\_next\_event - XXX

-   rpma\_conn\_disconnect - XXX

-   rpma\_conn\_delete - XXX

-   rpma\_ep\_shutdown - XXX

MEMORY MANAGEMENT
=================

-   rpma\_mr\_reg - XXX

-   rpma\_mr\_dereg - XXX

-   rpma\_mr\_get\_descriptor - XXX

-   rpma\_mr\_remote\_from\_descriptor - XXX

MESSAGING
=========

-   rpma\_send - XXX

-   rpma\_recv - XXX

-   rpma\_conn\_req\_recv - XXX

COMPLETIONS
===========

-   rpma\_conn\_prepare\_completions - XXX

-   rpma\_conn\_next\_completion - XXX

PEER
====

Elaborate XXX

-   rpma\_utils\_get\_ibv\_context - XXX

-   rpma\_peer\_new - XXX

-   rpma\_peer\_delete - XXX

SYNCHRONOUS AND ASYNCHRONOUS MODES
==================================

Elaborate XXX

-   rpma\_ep\_get\_fd - XXX

-   rpma\_conn\_get\_event\_fd - XXX

-   rpma\_conn\_get\_completion\_fd - XXX

SCALABILITY AND RESOURCE USE
============================

Elaborate XXX

-   rpma\_conn\_cfg\_set\_cq\_size - XXX

-   rpma\_conn\_cfg\_set\_sq\_size - XXX

-   rpma\_conn\_cfg\_set\_rq\_size - XXX

Mention getters XXX

THREAD SAFETY
=============

Elaborate XXX

ON-DEMAND PAGING SUPPORT

Elaborate XXX

-   rpma\_utils\_ibv\_context\_is\_odp\_capable - XXX

DEBUGGING AND ERROR HANDLING
============================

Elaborate XXX

-   rpma\_log\_set\_threshold - XXX

-   rpma\_log\_get\_threshold - XXX

-   rpma\_log\_set\_function - XXX

EXAMPLE
=======

See https://github.com/pmem/rpma/tree/master/examples for examples of
using the librpma API.

ACKNOWLEDGEMENTS
================

librpma is built on the top of libibverbs and librdmacm APIs.

SEE ALSO
========

https://pmem.io/rpma/
