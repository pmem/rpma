# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.12.0] - 2022-02-21
### Fixed
- links to https://pmem.io in the benchmarking framework
- templates with fio configuration in the benchmarking framework

### Removed
- APIs:
  - rpma_cq_get_completion - replaced with rpma_cq_get_wc
  - struct rpma_completion - replaced with struct ibv_wc from libibverbs
  - enum rpma_op - replaced with enum ibv_wc_opcode from libibverbs

## [0.11.0] - 2022-02-08
### Added
- Example (#12) for separate receive completion queue (RCQ).
- Documented the default values of struct rpma_conn_cfg.

- APIs:
  - rpma_cq_get_wc - receive one or more completions

- Tools:
  - Benchmarking framework basing on python scripts.

### Deprecated
- APIs:
  - rpma_cq_get_completion - replaced with rpma_cq_get_wc
  - struct rpma_completion - replaced with struct ibv_wc from libibverbs
  - enum rpma_op - replaced with enum ibv_wc_opcode from libibverbs

### Fixed
- Examples 07 and 08.

### Removed
- APIs:
  - rpma_conn_completion_get - replaced with rpma_conn_get_cq and rpma_cq_get_wc,
  - rpma_conn_completion_wait - replaced with rpma_conn_get_cq and rpma_cq_wait,
  - rpma_conn_get_completion_fd - replaced with rpma_conn_get_cq and rpma_cq_get_fd.

- Tools:
  - Benchmarking framework basing on bash scripts.

## [0.10.0] - 2022-01-24
### Added
- Unblocked performance of File System DAX by adding the possibility to use ibv_advice_mr() for registered memory ([#1220][1220]).
- Separate receive completion queue (RCQ) ([#1080][1080]).
- Support for iWARP protocol ([#1044][1044]).
- Write operation with immediate data (enum rpma_op RPMA_OP_RECV_RDMA_WITH_IMM added) ([#856][856]).
- Send operation with immediate data ([#713][713]).
- Handling the RDMA_CM_EVENT_REJECTED event (enum rpma_conn_event RPMA_CONN_REJECTED added) ([#802][802]).
- Support for many [Linux distributions and versions][distros] each of them tested [once a day with CI][nightly].

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

- Tools:
  - [ddio.sh script][ddio] to toggle and query the DDIO (Intel&reg; Data Direct I/O Technology) state per PCIe root port on Intel&reg; Cascade Lake platforms ([#597][597]).
  - [Benchmarking framework][bench] for the librpma.

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

- Tools:
  - This is the last release with the benchmarking framework basing on Bash scripts. In the next release it will be replaced with a Python-based benchmarking framework.

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
