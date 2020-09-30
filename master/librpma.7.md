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

\*\*Direct Write to PMem\*\* is a feature of a platform and its
configuration which allows an RDMA-capable network interface to write
data to platform\'s PMem in a persistent way. It may be impossible
because of e.g. caching mechanisms existing on the data\'s way. When
\*\*Direct Write to PMem\*\* is impossible, operating in the way
assuming it is possible may corrupt data on PMem, so this is why
\*\*Direct Write to PMem\*\* is not enabled by default.

On the current Intel platforms, the only thing you have to do in order
to enable \*\*Direct Write to PMem\*\* is turning off Intel Direct Data
I/O (DDIO). Sometimes, you can turn off DDIO either globally for the
whole platform or for a specific PCIe Root Port. For details, please see
the manual of your platform.

When you have a platform which allows \*\*Direct Write to PMem\*\*, you
have to declare this is the case in your peer\'s configuration. The
peer\'s configuration has to be transferred to all the peers which want
to execute **rpma\_flush**() with RPMA\_FLUSH\_TYPE\_PERSISTENT against
the platform\'s PMem and applied to the connection object which
safeguards access to PMem.

-   rpma\_peer\_cfg\_set\_direct\_write\_to\_pmem - declare \*\*Direct
    Write to PMem\*\* support

-   rpma\_peer\_cfg\_get\_descriptor - get the descriptor of the peer
    configuration

-   rpma\_peer\_cfg\_from\_descriptor - create a peer configuration from
    the descriptor

-   rpma\_conn\_apply\_remote\_peer\_cfg - apply remote peer cfg to the
    connection

For details on how to use these APIs please see
https://github.com/pmem/rpma/tree/master/examples/05-flush-to-persistent.

CLIENT OPERATION
================

A client is the active side of the process of establishing a connection.
A role of the peer during the process of establishing connection does
not determine direction of the data flow (neither via Remote Memory
Access nor via Messaging). After establishing the connection both peers
have the same capabilities.

The client, in order to establish a connection, has to perform the
following steps:

-   rpma\_conn\_req\_new - create a new outgoing connection request
    object

-   rpma\_conn\_req\_connect - initiate processing the connection
    request

-   rpma\_conn\_next\_event - wait for the RPMA\_CONN\_ESTABLISHED event

After establishing the connection both peers can perform Remote Memory
Access and/or Messaging over the connection.

The client, in order to close a connection, has to perform the following
steps:

-   rpma\_conn\_disconnect - initiate disconnection

-   rpma\_conn\_next\_event - wait for the RPMA\_CONN\_CLOSED event

-   rpma\_conn\_delete - delete the closed connection

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

If a librpma function may fail, it returns a negative error code.
Checking if the returned value is non-negative is the only
programmatically available way to verify if the API call succeeded. The
exact meaning of all error codes is described in the manual of each
function.

The librpma library implements the logging API which may give additional
information in case of an error and during normal operation as well,
according to the current logging threshold levels.

The function that will handle all generated log messages can be set
using **rpma\_log\_set\_function**(). The logging function can be either
the default logging function (built into the library) or a user-defined,
thread-safe, function. The default logging function can write messages
to **syslog**(3) and **stderr**(3). The logging threshold level can be
set or got using **rpma\_log\_set\_threshold**() or
**rpma\_log\_get\_threshold**() respectively.

There is an example of the usage of the logging functions:
https://github.com/pmem/rpma/tree/master/examples/log

EXAMPLES
========

See https://github.com/pmem/rpma/tree/master/examples for examples of
using the librpma API.

ACKNOWLEDGEMENTS
================

librpma is built on the top of libibverbs and librdmacm APIs.

SEE ALSO
========

https://pmem.io/rpma/
