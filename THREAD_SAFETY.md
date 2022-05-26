# THREAD SAFETY

This document presents the analysis of thread safety of the librpma library.

**The main assumptions** this analysis is based on are following:
1) many threads may use the same peer (`struct rpma_peer`) to create separate connections (`struct rpma_conn`) for each of threads, but
2) each of the endpoints (`struct rpma_ep`) can be used by only one thread at the same time and
3) each of the connections (`struct rpma_conn`) can be used by only one thread at the same time.

so **the most common scenarios** are following:
1) on the active side: each thread creates and uses a separate connection (`struct rpma_conn`),
2) on the passive side: the main thread establishes the connection but the rest of work (including connection shutdown) is done by a separate thread.

Most of the core librpma API calls are thread-safe but there are also very important exceptions (described below) mainly related to connection's configuration, establishment and tear-down.

Creating resources of RPMA library usually involves dynamic memory allocation and destroying resources usually involves a dynamic memory release. The same resource cannot be destroyed more than once at any thread and a resource cannot be used after it was destroyed. It is the user's responsibility to follow those rules and not doing so may result in a segmentation fault or an undefined behaviour.

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- rpma_peer_new
- rpma_peer_delete
- rpma_peer_cfg_new
- rpma_peer_cfg_delete
- rpma_peer_cfg_from_descriptor
- rpma_peer_cfg_get_descriptor_size
- rpma_ep_get_fd;
- rpma_ep_listen;
- rpma_ep_next_conn_req;
- rpma_ep_shutdown;
- rpma_conn_cfg_new
- rpma_conn_cfg_delete
- rpma_mr_reg
- rpma_mr_dereg
- rpma_mr_get_descriptor
- rpma_mr_remote_from_descriptor
- rpma_mr_get_descriptor_size
- rpma_mr_get_ptr
- rpma_mr_get_size
- rpma_mr_remote_get_size
- rpma_mr_remote_delete
- rpma_mr_remote_get_flush_type
- rpma_mr_advise
- rpma_conn_delete
- rpma_conn_disconnect
- rpma_conn_get_cq
- rpma_conn_get_compl_fd
- rpma_conn_get_event_fd
- rpma_conn_get_private_data
- rpma_conn_get_qp_num
- rpma_conn_get_rcq
- rpma_conn_next_event
- rpma_conn_wait
- rpma_atomic_write
- rpma_flush
- rpma_read
- rpma_recv
- rpma_send
- rpma_send_with_imm
- rpma_write
- rpma_write_with_imm
- rpma_cq_get_fd
- rpma_cq_wait
- rpma_cq_get_wc

## Conditionally thread-safe API calls

The following API calls of the librpma library:
- rpma_peer_cfg_set_direct_write_to_pmem
- rpma_peer_cfg_get_direct_write_to_pmem
- rpma_peer_cfg_get_descriptor
- rpma_conn_apply_remote_peer_cfg - calls rpma_peer_cfg_get_direct_write_to_pmem

are thread-safe only if each thread operates on a **separate peer configuration structure** (`struct rpma_peer_cfg`) used only by this one thread. They are not thread-safe if threads operate on one peer configuration structure common for more than one thread.

The following API calls of the librpma library:
- rpma_conn_cfg_get_compl_channel
- rpma_conn_cfg_get_cq_size
- rpma_conn_cfg_get_rcq_size
- rpma_conn_cfg_get_rq_size
- rpma_conn_cfg_get_sq_size
- rpma_conn_cfg_get_timeout
- rpma_conn_cfg_set_compl_channel
- rpma_conn_cfg_set_cq_size
- rpma_conn_cfg_set_rcq_size
- rpma_conn_cfg_set_rq_size
- rpma_conn_cfg_set_sq_size
- rpma_conn_cfg_set_timeout
- rpma_conn_req_new (calls rpma_conn_cfg_get_*() functions)

are thread-safe only if each thread operates on a **separate connection configuration structure** (`struct rpma_conn_cfg`) used only by this one thread. They are not thread-safe if threads operate on one connection configuration structure common for more than one thread.

The following API calls of the librpma library:
- rpma_conn_req_connect
- rpma_conn_req_delete
- rpma_conn_req_get_private_data
- rpma_conn_req_recv

are thread-safe only if each thread operates on a **separate connection request** (`struct rpma_conn_req`) used only by this one thread. They are not thread-safe if threads operate on one connection request common for more than one thread.

## NOT thread-safe API calls

The following API calls of the librpma library are NOT thread-safe:
- XXX

## Relationship of libibverbs and librdmacm

XXX

## Not thread-safe scenarios

XXX

## Analysis of Valgrind suppressions

### Suppressions for libibverbs

The suppressions for libibverbs are described in the [tests/memcheck-libibverbs.supp](tests/memcheck-libibverbs.supp) file.

### Suppressions for the drd tool

The suppressions for the drd tool are described in the [tests/drd.supp](tests/drd.supp) file.

### Suppressions for the helgrind tool

The suppressions for the helgrind tool are described in the [tests/helgrind.supp](tests/helgrind.supp) file.
