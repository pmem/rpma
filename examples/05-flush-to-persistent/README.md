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

**Note**: For a server with PMem support it is allowed to configure assumed direct
PMem write support which indicates whether RPMA_FLUSH_TYPE_PERSISTENT is supported.
The client has to apply this configuration to be aware of what types of flush it
can perform on the server's memory.

**Note**: If either server or client does not have a pmem path (or it is not
capable to use pmem at all) it uses DRAM instead.

**Note**: For the sake of this example, the memory region being written to and
the server's peer configuration are transferred via the connection's private
data. In general, it can be transferred via an out-of-band or the in-band
channel.

## Usage

```bash
[user@server]$ ./server $server_address $port [<pmem-path>] [<direct-pmem-write>]
```

```bash
[user@client]$ ./client $server_address $port [<pmem-path> [<pmem-offset>]]
```

where `<pmem-path>` can be:
  - a Device DAX (`/dev/dax0.0` for example) or
  - a file on File System DAX (`/mnt/pmem/file` for example)
and `<pmem-offset>` is an offset inside the above mentioned PMem device
where the user data begins from.
