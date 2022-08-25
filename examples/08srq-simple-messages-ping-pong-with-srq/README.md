Example of using messaging with shared RQ and a single connection
===

The single-connection messages ping pong example with shared RQ implements two parts of the messaging process:
- The server starts a listening endpoint and waits for an incoming connection.
When a new connection request appears it is accepted. The client sends to
the server its current counter value. When the server receives the message
from the client, its content is incremented and send back to the client. When
the server receives the I_M_DONE message it disconnects.
- The client connects to the server and sends to it its current counter value.
When the client gets the new value from the server it repeats the process
for the configured number of rounds. When it is quitting, it send the I_M_DONE
message and waits for disconnection.

## Usage

```bash
[user@server]$ ./server $server_address $port [m|r]
```
**Note** The third parameter can be one of the following values:
- unspecified - get receive completions by the receive CQ of the shared RQ
- m - get receive completions by the main CQ of the connection
- r - get receive completions by the separate receive CQ of the connection

```bash
[user@client]$ ./client $server_address $port $seed $rounds [$sleep]
```
**Note** seed is the initial value of the counter
