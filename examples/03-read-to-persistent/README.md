Example of performing an RPMA read to a persistent memory region
===

The read example implements two parts of the read process:
- The server in this example, if provided (and capable of), prepares a local
persistent memory and registers it as a reading destination. After
the connection is established the server receives the client's memory region
registered as read source. The servers performs read from the remote memory
region to a local memory region.
- The client in this example, if provided (and capable of), prepares a local
persistent memory (including its contents) and registers it as a reading
source and exposes the memory description along with other parameters
required to perform an RDMA read. After the connection is established
the client just waits for the server to disconnect.

**Note**: If either server or client does not have a pmem path (or it is not
capable to use pmem at all) it uses DRAM instead.

**Note**: For the sake of this example, the memory region being read from is
transferred via connection's private data. In general, it can be transferred via
an out-of-band or the in-band channel.

## Usage

```bash
[user@server]$ ./server $server_address $service [<pmem-path>]
```

```bash
[user@client]$ ./client $server_address $service [<pmem-path>]
```
