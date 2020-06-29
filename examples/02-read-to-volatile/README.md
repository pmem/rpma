Example of performing an RPMA read to volatile memory region
===

The read example implements two parts of the read process:
- a server which will register a volatile memory region as read source
- a client which will register a volatile memory region as read destiation,
post an read request and wait for its completion

**Note**: For the sake of this example, the memory region being read from is
transferred via connection's private data. In general, it can be transferred via
any type of the out-of-band or in-band channel.

## Usage

```bash
[user@server]$ ./example-read-server $addr_server $service
```

```bash
[user@client]$ ./example-read-client $addr_server $service
```