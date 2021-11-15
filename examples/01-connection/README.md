Example of establishing an RPMA connection
===

The connection example implements two parts of the connection establishing
process:
- a server which will be waiting for incoming connections using single RPMA
endpoint
- a client which will initiate establishing the connection to the server

**Note**: This example, after establishing the connection, just disconnects. For
examples of how to make real use of the connection, please see other examples.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port
```
