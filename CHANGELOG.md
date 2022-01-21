# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Possibility to give an advice about an address range in a memory registration ([#1220][1220]).
- Separate receive Completion Queue (CQ) ([#1080][1080]).
- Support for iWARP protocol ([#1044][1044]).
- Write operation with immediate data ([#856][856]).
- Send operation with immediate data ([#713][713]).
- Handling the RDMA_CM_EVENT_REJECTED event ([#802][802]).
- The [ddio.sh script][ddio] to toggle and query the DDIO (Intel&reg Data Direct I/O Technology) state per PCIe root port on Intel&reg Cascade Lake platforms ([#597][597]).
- [Benchmarking framework][bench] for the librpma.
- Support for many [Linux distributions and versions][distros] each of them tested [once a day with CI][nightly]:
  - Ubuntu 18.04, 20.04, 21.04, 21.10 and Rolling,
  - Debian 9-11, Stable, Testing and Experimental,
  - Fedora 26-34,
  - CentOS 7 and 8,
  - Rocky Linux 8.4,
  - VzLinux 8,
  - OpenSUSE Leap and Tumbleweed,
  - Arch Linux.
- APIs:
  - rpma_conn_cfg_get_rcq_size - gets receive CQ size for the connection,
  - rpma_conn_cfg_set_rcq_size - sets receive CQ size for the connection,
  - rpma_conn_get_cq - gets the connection's main CQ,
  - rpma_conn_get_qp_num - gets the connection's qp_num,
  - rpma_conn_get_rcq - gets the connection's receive CQ,
  - rpma_conn_req_get_private_data - gets a pointer to the request's private data,
  - rpma_cq_get_completion - receives a completion of an operation,
  - rpma_cq_get_fd - gets the completion queue's file descriptor,
  - rpma_cq_wait - waits for a completion,
  - rpma_mr_advise - gives advice about an address range in a memory registration,
  - rpma_mr_get_ptr - gets the pointer to the local memory region,
  - rpma_mr_get_size - gets the size of the local memory region,
  - rpma_send_with_imm - initiates the send operation with immediate data,
  - rpma_write_with_imm - initiates the write operation with immediate data.
- enum rpma_conn_event: RPMA_CONN_REJECTED ([#802][802]).
- enum rpma_op: RPMA_OP_RECV_RDMA_WITH_IMM ([#856][856]).

### Changed
- Atomic write operation (rpma_write_atomic()) implemented with fence
  (now it waits for RDMA Read which simulates RDMA flush) ([#603][603]).
- API for completions handling (see Deprecated).
- rpma_read, rpma_write, rpma_send and rpma_recv can be called with 0B message.
- Updated and fixed documentation.

### Deprecated
- APIs:
  - rpma_conn_completion_get - replaced with rpma_conn_get_cq and rpma_cq_get_completion,
  - rpma_conn_completion_wait - replaced with rpma_conn_get_cq and rpma_cq_wait,
  - rpma_conn_get_completion_fd - replaced with rpma_conn_get_cq and rpma_cq_get_fd.

### Fixed
- rpma_flush_apm_new() fixed, so that rpma_mr_reg() can be called after ibv_fork_init() ([#866][866]).

[bench]: https://github.com/pmem/rpma/blob/master/tools/perf/BENCHMARKING.md
[ddio]: https://github.com/pmem/rpma/blob/master/tools/ddio.sh
[distros]: https://github.com/pmem/rpma/blob/master/.github/workflows/nightly.yml
[nightly]: https://github.com/pmem/rpma/actions/workflows/nightly.yml
[1220]: https://github.com/pmem/rpma/pull/1220
[1080]: https://github.com/pmem/rpma/pull/1080
[1044]: https://github.com/pmem/rpma/pull/1044
[866]: https://github.com/pmem/rpma/pull/866
[856]: https://github.com/pmem/rpma/pull/856
[802]: https://github.com/pmem/rpma/pull/802
[713]: https://github.com/pmem/rpma/pull/713
[603]: https://github.com/pmem/rpma/pull/603
[597]: https://github.com/pmem/rpma/pull/597

## [0.9.0] - 2020-10-01
### Added
- This is the first official release of the librpma library.
- The API provides the most flexible implementation of remote persistency
via Appliance Persistency Method.
- Multiple examples show how to use this API (including an example of how to
build your own General Purpose Persistency Method implementation basing on
the services provided by the librpma library).
- The extensive documentation describes all guiding principles.
