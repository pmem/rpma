Example of rpma write with immediate data
===

The rpma write with immediate data implements two parts of the process:
- The client connects to the server and writes a message with immediate
data to the server.
- The server receives a message with immediate data from client.
The immediate data is compared with the expected immediate data sent
by the client as the private data during establishing the connection.
The argument 'operation' only supports write or write_atomic.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port $operation $word $imm
```
