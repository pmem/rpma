Example of using messaging
===

The messages ping pong example implements two parts of the messaging process:
- The server starts a listening endpoint and waits for incoming connections.
When a new connection request appears it is accepted, if a free client slot
is available, or rejected if otherwise. The server prepares a receiving buffer
for each of the clients. The client sends to the server they current counter
value. When the server will receive the message from the client its content is
incremented and send back to the client.
- The client connects to the server and sends to it they current counter value.
When the client will get the new value from the server they repeat the process
for the configured number of rounds.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port $seed $rounds
```
