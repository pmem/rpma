A ping-pong example with multiple connections
===

The multiple connections example implements two parts of the connection process:
- The server starts a listening endpoint and waits for incoming connections
using epoll (a type of scalable I/O technique). When a new connection request
appears it is accepted and the server initiate a receive operation, if a free
client slot is available, or rejected otherwise. The new connection's file
descriptors are added to the same epoll (both an event file descriptor and a
completion file descriptor) so the server can use a single epoll to wait for
incoming connections, connection-related events and completions. When a specific
client's connection is established (for what the server waits asynchronously
using epoll) the server wait a receive completion (which is also notified via
epoll) event which indicates it has receives client's "hello" message. It then
sends a string "What's your name?" to client. After that, the client will send
its name to server. The client's name is saved in a local memory region
prepared for the client's slot. The server displays the client's name and
disconnects the client.
- The client picks randomly its name and copies it into a memory region
that is exposed to the server. When the connection is established it waits
for the server to ask its name and replys a random name.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port
```
