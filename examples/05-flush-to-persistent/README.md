Example of performing an RPMA write and flushing it to persistence
===

The write example implements two parts of the write process:
- The server, if provided (and capable of), prepares a local persistent memory
and exposes the memory description along with other parameters required
to perform a RDMA write and RDMA flush. After the connection is established
the server waits for the client to disconnect.
- The client, if provided (and capable of), prepares a local persistent memory
(including its contents) and registers it as a writing source.
After the connection is established the client receives the server's memory
regions registered as a writing destination. The client performs a write from
the local memory region to a remote memory region followed by RPMA flush.

**Note**: If either server or client does not have a pmem path (or it is not
capable to use pmem at all) it uses DRAM instead.

**Note**: For the sake of this example, the memory region being write from is
transferred via connection's private data. In general, it can be transferred via
an out-of-band or the in-band channel.

## Usage

```bash
[user@server]$ ./server $server_address $port [<pmem-path>]
```

```bash
[user@client]$ ./client $server_address $port [<pmem-path>]
```
