Example of using messaging
===

The messages ping pong example implements two parts of the messaging process:
- The server starts a listening endpoint and waits for an incoming connection.
When a new connection request appears it is accepted. The client sends to
the server its current counter value. When the server receives the message
from the client, its content is incremented and send back to the client. When
the server receives the I_M_DONE message it waits for disconnection.
- The client connects to the server and sends to it its current counter value.
When the client gets the new value from the server it repeats the process
for the configured number of rounds. When it is quitting, it send the I_M_DONE
message and disconnects.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port $seed $rounds # [<sleep>]
```
