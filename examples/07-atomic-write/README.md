Example of performing an RPMA atomic write to a persistent memory region
===

The atomic write example shows a simple remote log manipulation where two
parts of the process are implemented:
- The server, if provided (and capable of), prepares a local persistent memory.
The memory is filled with an initial log state. The simplest log structure
looks as follow:
```
struct log {
	char signature[LOG_SIGNATURE_SIZE];
	/* last written data (note the field is aligned to RPMA_ATOMIC_WRITE_ALIGNMENT) */
	uint64_t used;
	char data[LOG_DATA_SIZE];
};
```
The client along with the exposed memory descriptor receives an offset of
the used field. Which is enough to read the current state of the log and
perform an append. After the connection is established the server waits for
the client to disconnect.
- The client, after the connection is established, receives the server's
memory descriptor and the used offset. The client reads the used value.
Having that, the client writes and flushes the new data at the end of the log.
Without waiting for both operations completions the client can perform
an atomic write of the new used value and flush it. The client has to wait
for the last flush completion before appending another entry to the log.
When the client is done it simply disconnects from the server.

**Note**: For the sake of simplicity, the client assumes unilaterally that
the server has the persistent flush support.

**Note**: If server does not have a pmem path (or it is not
capable to use pmem at all) it uses DRAM instead.

**Note**: For the sake of this example, the memory region being write from is
transferred via connection's private data. In general, it can be transferred via
an out-of-band or the in-band channel.

## Usage

```bash
[user@server]$ ./server $server_address $port [<pmem-path>]
```

```bash
[user@client]$ ./client $server_address $port $word1 [<word2>] [<...>]
```

where `<pmem-path>` can be:
  - a Device DAX (`/dev/dax0.0` for example) or
  - a file on File System DAX (`/mnt/pmem/file` for example).
