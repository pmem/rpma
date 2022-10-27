Example of performing an RPMA read to a persistent memory region
===

The read example implements two parts of the read process:
- The server, if provided (and capable of), prepares a local persistent memory
and registers it as a reading destination. After the connection is established
the server receives the client's memory region registered as a reading source.
The servers performs a read from the remote memory region to a local memory
region.
- The client, if provided (and capable of), prepares a local persistent memory
(including its contents), registers it as a reading source, and exposes
the memory description along with other parameters required to perform
a RDMA read. After the connection is established, the client just waits for
the server to disconnect.

**Note**: If either server or client does not have a pmem path (or it is not
capable to use pmem at all) it uses DRAM instead.

**Note**: For the sake of this example, the memory region being read from is
transferred via connection's private data. In general, it can be transferred via
an out-of-band or the in-band channel.

## Usage

```bash
[user@server]$ ./server $server_address $port [<pmem-path>]
```

```bash
[user@client]$ ./client $server_address $port [<pmem-path> [<pmem-offset>]]
```

where `<pmem-path>` can be:
  - a Device DAX (`/dev/dax0.0` for example) or
  - a file on File System DAX (`/mnt/pmem/file` for example)
and `<pmem-offset>` is an offset inside the above mentioned PMem device
where the user data begins from.
