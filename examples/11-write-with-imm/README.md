Example of rpma write with immediate data
===

The rpma write with immediate data implements two parts of the process:
- The client connects to the server and writes a message with immediate
data to the server.
- The server receives an immediate data from the client.
The immediate data has the same value as the message written by the
client so the immediate data is compared with the expected message.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port $imm
```
