Example of rpma send with immediate data
===

The rpma send with immediate data implements two parts of the process:
- The client connects to the server and sends a message with imm_data
to the server.
- The server receives a message with imm_data from client.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port $word imm_data=$value
```
