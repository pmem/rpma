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

-   **rpma\_read**() - XXX

-   **rpma\_write**() - XXX

-   **rpma\_write\_atomic**() - XXX

-   **rpma\_flush**() - XXX

DIRECT WRITE TO PMEM
====================

**Direct Write to PMem** is a feature of a platform and its
configuration which allows an RDMA-capable network interface to write
data to platform\'s PMem in a persistent way. It may be impossible
because of e.g. caching mechanisms existing on the data\'s way. When
**Direct Write to PMem** is impossible, operating in the way assuming it
is possible may corrupt data on PMem, so this is why **Direct Write to
PMem** is not enabled by default.

On the current Intel platforms, the only thing you have to do in order
to enable **Direct Write to PMem** is turning off Intel Direct Data I/O
(DDIO). Sometimes, you can turn off DDIO either globally for the whole
platform or for a specific PCIe Root Port. For details, please see the
manual of your platform.

When you have a platform which allows **Direct Write to PMem,** you have
to declare this is the case in your peer\'s configuration. The peer\'s
configuration has to be transferred to all the peers which want to
execute **rpma\_flush**() with RPMA\_FLUSH\_TYPE\_PERSISTENT against the
platform\'s PMem and applied to the connection object which safeguards
access to PMem.

-   **rpma\_peer\_cfg\_set\_direct\_write\_to\_pmem**() - declare
    **Direct Write to PMem** support

-   **rpma\_peer\_cfg\_get\_descriptor**() - get the descriptor of the
    peer configuration

-   **rpma\_peer\_cfg\_from\_descriptor**() - create a peer
    configuration from the descriptor

-   **rpma\_conn\_apply\_remote\_peer\_cfg**() - apply remote peer cfg
    to the connection

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

-   **rpma\_conn\_req\_new**() - create a new outgoing connection
    request object

-   **rpma\_conn\_req\_connect**() - initiate processing the connection
    request

-   **rpma\_conn\_next\_event**() - wait for the RPMA\_CONN\_ESTABLISHED
    event

After establishing the connection both peers can perform Remote Memory
Access and/or Messaging over the connection.

The client, in order to close a connection, has to perform the following
steps:

-   **rpma\_conn\_disconnect**() - initiate disconnection

-   **rpma\_conn\_next\_event**() - wait for the RPMA\_CONN\_CLOSED
    event

-   **rpma\_conn\_delete**() - delete the closed connection

SERVER OPERATION
================

A server is the passive side of the process of establishing a
connection. Note that after establishing the connection both peers have
the same capabilities.

The server, in order to establish a connection, has to perform the
following steps:

-   **rpma\_ep\_listen**() - create a listening endpoint

-   **rpma\_ep\_next\_conn\_req**() - obtain an incoming connection
    request

-   **rpma\_conn\_req\_connect**() - initiate connecting the connection
    request

-   **rpma\_conn\_next\_event**() - wait for the RPMA\_CONN\_ESTABLISHED
    event

After establishing the connection both peers can perform Remote Memory
Access and/or Messaging over the connection.

The server, in order to close a connection, has to perform the following
steps:

-   **rpma\_conn\_next\_event**() - wait for the RPMA\_CONN\_CLOSED
    event

-   **rpma\_conn\_disconnect**() - disconnect the connection

-   **rpma\_conn\_delete**() - delete the closed connection

When no more incoming connections are expected, the server can stop
waiting for them:

-   **rpma\_ep\_shutdown**() - stop listening and delete the endpoint

MEMORY MANAGEMENT
=================

Every piece of memory (either volatile or persistent) must be registered
and its usage must be specified in order to be used in Remote Memory
Access or Messaging. This can be done using the following memory
management librpma functions:

-   **rpma\_mr\_reg**() which registers a memory region and creates a
    local memory registration object and

-   **rpma\_mr\_dereg**() which deregisters the memory region and
    deletes the local memory registration object.

A description of the registered memory region sometimes has to be
transferred via network to the other side of the connection. In order to
do that a network-transferable description of the provided memory region
(called \'descriptor\') has to be created using
**rpma\_mr\_get\_descriptor**(). On the other side of the connection the
received descriptor should be decoded using
**rpma\_mr\_remote\_from\_descriptor**(). It creates a remote memory
region\'s structure that allows for Remote Memory Access.

MESSAGING
=========

The librpma messaging API allows transferring messages (buffers of
arbitrary data) between the peers. Transferring messages requires
preparing buffers (memory regions) on the remote side to receive the
sent data. The received data are written to those dedicated buffers and
the sender does not have to have a respective remote memory region
object to send a message. The memory buffers used for messaging have to
be registered using **rpma\_mr\_reg**() prior to **rpma\_send**() or
**rpma\_recv**() function call.

The librpma library implements the following messaging API:

-   **rpma\_send**() - initiates the send operation which transfers a
    message from the local memory to other side of the connection,

-   **rpma\_recv**() - initiates the receive operation which prepares a
    buffer for a message sent from other side of the connection,

-   **rpma\_conn\_req\_recv**() works as **rpma\_recv**(), but it may be
    used before the connection is established.

All of these operations are considered as finished when the respective
completion is generated.

COMPLETIONS
===========

-   **rpma\_conn\_prepare\_completions**() - XXX

-   **rpma\_conn\_next\_completion**() - XXX

PEER
====

Elaborate XXX

-   **rpma\_utils\_get\_ibv\_context**() - XXX

-   **rpma\_peer\_new**() - XXX

-   **rpma\_peer\_delete**() - XXX

SYNCHRONOUS AND ASYNCHRONOUS MODES
==================================

By default, all endpoints and connections operate in the synchronous
mode where:

-   **rpma\_ep\_next\_conn\_req**(),

-   **rpma\_conn\_prepare\_completions**() and

-   **rpma\_conn\_get\_next\_event**()

are blocking calls. You can make those API calls non-blocking by
modifying the respective file descriptors:

-   **rpma\_ep\_get\_fd**() - provides a file descriptor for
    **rpma\_ep\_next\_conn\_req**()

-   **rpma\_conn\_get\_completion\_fd**() - provides a file descriptor
    for **rpma\_conn\_prepare\_completions**()

-   **rpma\_conn\_get\_event\_fd**() - provides a file descriptor for
    **rpma\_conn\_get\_next\_event**()

When you have a file descriptor, you can make it non-blocking using
**fcntl**(2) as follows:

            int ret = fcntl(fd, F_GETFL);
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);

Such change makes the respective API call non-blocking automatically.

The provided file descriptors can also be used for scalable I/O handling
like **epoll**(7).

Please see the example showing how to make use of RPMA file descriptors:
https://github.com/pmem/rpma/tree/master/examples/06-multiple-connections

QUEUES, PERFORMANCE AND RESOURCE USE
====================================

**Remote Memory Access** operations, **Messaging** operations and their
**Completions** consume space in queues allocated in an RDMA-capable
network interface (RNIC) hardware for each of the connections. You must
be aware of the existence of these queues:

-   completion queue **(CQ)** where completions of operations are
    placed, either when a completion was required by a user
    (RPMA\_F\_COMPLETION\_ALWAYS) or a completion with an error
    occurred. All **Remote Memory Access** operations and **Messaging**
    operations can consume **CQ** space.

-   send queue **(SQ)** where all **Remote Memory Access** operations
    and **rpma\_send**() operations are placed before they are executed
    by RNIC.

-   receive queue **(RQ)** where **rpma\_recv**() entries are placed
    before they are consumed by the **rpma\_send**() coming from another
    side of the connection.

You must assume **SQ** and **RQ** entries occupy the place in their
respective queue till:

-   a respective operation\'s completion is generated or

-   a completion of an operation, which was scheduled later, is
    generated.

You must also be aware that RNIC has limited resources so it is
impossible to store a very long set of queues for many possibly existing
connections. If all of the queues will not fit into RNIC\'s resources it
will start using the platform\'s memory for this purpose. In this case,
the performance will be degraded because of inevitable cache misses.

Because the length of queues has so profound impact on the performance
of RPMA application you can configure the length of each of the queues
separately for each of the connections:

-   **rpma\_conn\_cfg\_set\_cq\_size**() - set length of **CQ**

-   **rpma\_conn\_cfg\_set\_sq\_size**() - set length of **SQ**

-   **rpma\_conn\_cfg\_set\_rq\_size**() - set length of **RQ**

When the connection configuration object is ready it has to be used for
either **rpma\_conn\_req\_new**() or **rpma\_ep\_next\_conn\_req**() for
the settings to take effect.

THREAD SAFETY
=============

Elaborate XXX

ON-DEMAND PAGING SUPPORT

Elaborate XXX

-   **rpma\_utils\_ibv\_context\_is\_odp\_capable**() - XXX

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
