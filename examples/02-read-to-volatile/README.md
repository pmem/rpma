Example of performing an RPMA read to a volatile memory region
===

The read example implements two parts of the read process:
- a server which will register a volatile memory region as a read source
- a client which will register a volatile memory region as a read destination,
post a read request and wait for its completion

**Note**: For the sake of this example, the memory region being read from is
transferred via connection's private data. In general, it can be transferred via
an out-of-band or the in-band channel.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port
```
