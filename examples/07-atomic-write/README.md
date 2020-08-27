Example of performing an RPMA atomic write to a persistent memory region
===

The atomic write example implements two parts of the write process:
- The server, if provided (and capable of), prepares a local persistent memory
and exposes the memory description along with other parameters required to perform
a RDMA read/write.
The server prepares the log structure:
```
struct log_header {
	char signature[LOG_SIGNATURE_SIZE];
	uint64_t used; /* offset data */
};

struct log {
	struct log_header header;
	char data[LOG_DATA_SIZE];
};
```
After the connection is established the server waits for the client to disconnect.
- The client, prepares a local memory (including its contents) and registers it as a writing source.
After the connection is established the client receives the server's memory regions registered
as a writing destination. The client reads the offset under which it can write data. It saves the data and then updates value of the field used with rpma_write_atomic.

**Note**: If either server does not have a pmem path (or it is not
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
