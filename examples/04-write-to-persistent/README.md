Example of performing an RPMA write to a persistent memory region
===

The write example implements two parts of the write process:
- The server, if provided (and capable of), prepares a local persistent memory
and registers it as a writing destination. After the connection is established
the server receives the client's memory region registered as a writing source.
The servers performs a write from the remote memory region to a local memory
region.
- The client, if provided (and capable of), prepares a local persistent memory
(including its contents) and registers it as a writing source and exposes
the memory description along with other parameters required to perform
a RDMA write. After the connection is established the client just waits for
the server to disconnect.

**Note**: If either server or client does not have a pmem path (or it is not
capable to use pmem at all) it uses DRAM instead.

**Note**: For the sake of this example, the memory region being write from is
transferred via connection's private data. In general, it can be transferred via
an out-of-band or the in-band channel.

## Usage

```bash
[user@server]$ ./server $server_address $service [<pmem-path>]
```

```bash
[user@client]$ ./client $server_address $service [<pmem-path>]
```
