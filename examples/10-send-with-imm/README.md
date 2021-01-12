Example of rpma send with immediate data
===

The rpma send with immediate data implements two parts of the process:
- The client connects to the server and sends a message with immediate
data to the server.
- The server receives a message with immediate data from client.  The
immediate data is compared with an expected immediate data sent by the
client as the private data during establishing the connection.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port $word $imm
```
