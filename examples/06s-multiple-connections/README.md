Example of handling multiple connections with shared completion channel
===

The multiple connections example implements two parts of the connection process:
- The server starts a listening endpoint and waits for incoming connections
using epoll (a type of scalable I/O technique). When a new connection request
appears it is accepted, if a free client slot is available, or rejected
otherwise. The new connection's file descriptors are added to the same epoll
(both an event file descriptor and a completion file descriptor) so the server
can use a single epoll to wait for incoming connections, connection-related
events and completions. When a specific client's connection is established
(for what the server waits asynchronously using epoll) the server performs
a read from the client's remote memory region to a local memory region prepared
for the client's slot. It should generate an IBV_WC_RDMA_READ completion
(which is also notified via epoll) after which the server displays the read data
(the client's name) and disconnects the client.
- The client picks randomly its name and copies it into a memory region
that is exposed to the server. When the connection is established it waits
for the server to disconnect.

**Note**: For the sake of this example, the memory region being write from is
transferred via connection's private data. In general, it can be transferred via
an out-of-band or the in-band channel.

**Note**: In this example, we use the shared completion event channel for CQ for completions of send.
We use the rpma_conn_wait() function to wait for completions' events.

## Usage

```bash
[user@server]$ ./server $server_address $port
```

```bash
[user@client]$ ./client $server_address $port $seed
```
