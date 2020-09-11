Example of performing an RPMA write and flushing it to persistence via GPSPM
===

The write example implements two parts of the write process:
- The server, if provided (and capable of), prepares a local persistent memory
and exposes the memory description along with other parameters required
to perform a RDMA write. After the connection is established the server waits
for a flush request to execute `pmem_persist(3)` according to provided flush
parameters. When it is done the server sends back a flush response and waits
for the client to disconnect.
- The client, if provided (and capable of), prepares a local persistent memory
(including its contents) and registers it as a writing source.
After the connection is established the client receives the server's memory
regions registered as a writing destination. The client performs a write from
the local memory region to a remote memory region and sends to the server a flush
request and waits for a flush response which indicates the flush operation's
completion.

**Note**: This example does not require RPMA_FLUSH_TYPE_PERSISTENT support
to provide remote PMem persistency. For the sake of simplicity, the client
do not need to know server's peer configuration. For an example of peer's
configuration exchange please see example 05.

**Note**: The flush request and resonse are send and receive via the RPMA'a
messaging API (`rpma_send()` and `rpma_recv()`).

**Note**: If either server or client does not have a PMem path (or it is not
capable to use PMem at all) it uses DRAM instead.

**Note**: For the sake of this example, the memory region being written to is
transferred via the connection's private data. In general, it can be transferred
via an out-of-band or the in-band channel.

## Usage

```bash
[user@server]$ ./server $server_address $port [<pmem-path>]
```

```bash
[user@client]$ ./client $server_address $port [<pmem-path>]
```
