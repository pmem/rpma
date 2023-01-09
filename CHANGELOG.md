# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.1] - 2023-01-09
### Added
- native atomic write support (required support in the kernel and in an RNIC's driver)
- security warnings to the documentation of the following functions of the API:
  - rpma_conn_get_private_data()
  - rpma_conn_req_get_private_data()
  - rpma_mr_get_descriptor()
  - rpma_mr_remote_from_descriptor()
  - rpma_peer_cfg_from_descriptor()
  - rpma_peer_cfg_get_descriptor()
- offset of the beginning of the used persistent memory in the clients using PMem in the examples
- one suppression for Memcheck on Ubuntu 22.04
- CI Coverity build run once a day over the night
- a check for the native atomic write support in libibverbs
- internal APIs:
  - rpma_utils_ibv_context_is_atomic_write_capable() - checks if kernel supports native atomic write
- Rocky Linux 8 and 9 builds to the "on_pull_request" workflow
- show git diff of changed documentation when pull requests with updated documentation are not generated

### Fixed
- DEVELOPMENT.md file - `CMAKE_BUILD_TYPE` must be set to `Debug` when running the tests
- build system for CentOS 7 (use cmake3 instead of cmake if a version of cmake is v2.x)
- check-headers.sh file - corrected the path of check-ms-license.pl and removed
  unneeded '*' at the start of the grep expressions
- (examples) use HELLO_STR_SIZE instead of KILOBYTE in case of the hello string
- the common_pmem_map_file_with_signature_check() function in examples
- `wr` passed to ibv_post_send(), ibv_post_recv() and ibv_post_srq_recv() is initialized to 0
- `sge` passed to a log message (in rpma_mr_*() functions) is initialized to 0
- `rq_size` in rpma_peer_create_srq() initialized to 0
- detecting no free slot for a new connection request in example 13
- memory allocations in example 07
- minor issues detected by Coverity
- sleep(1) added to mtt_client_connect() before the next connection retry

### Changed
- the default 'master' branch has been renamed to 'main'
- logging of the source and the destination GID addresses in rpma_conn_req_new_from_id()
  has been restricted to only one case when CMAKE_BUILD_TYPE is set to 'Debug'
- rpma_peer_new() to check the native atomic write support in kernel
- rpma_peer_setup_qp() to enable native atomic write if both kernel and libibverbs supported it
- rpma_mr_atomic_write() to use native atomic write if the created QP supported it
- only the labeled (latest/stable/rolling etc.) versions of docker images (if available) are used in CI - it makes the CI self-updating

### Removed
- whole benchmarking framework for librpma (the last commit with the benchmarking framework present is marked with the "[benchmarking-framework][bench-frame]" tag)
- unused doc_snippets
- meaningless template-example
- meaningless template unit test
- Debian 10 from the on_pull_request CI workflow

[bench-frame]: https://github.com/pmem/rpma/tree/benchmarking-framework/tools/perf

## [1.1.0] - 2022-09-08
### Added
- (tools) description of the 'schematic' variable (from the report.json file) in the 'tools/perf/BENCHMARKING.md' file

### Fixed
- removed unnecessary rpma_conn_req_delete() calls from examples
- (tools) added checking if a path saved in the 'schematic' variable exists
- common source code of GPSMP examples moved to one folder
- changed size of the read-after-write (RAW) buffer in the example 04
- (examples) changed the write size from KILOBYTE to HELLO_STR_SIZE

### Changed
- unified coding style in the source, the test and the example files

## [1.0.0] - 2022-08-25
### Added
- DEVELOPMENT.md file containing the most important information needed during development of the library
- THREAD_SAFETY.md file containing the analysis of thread safety of the librpma library
- APIs:
  - rpma_conn_cfg_get_compl_channel - gets if the completion event channel can be shared by CQ and RCQ
  - rpma_conn_cfg_get_srq - gets the shared RQ object from the connection
  - rpma_conn_cfg_set_compl_channel - sets if the completion event channel can be shared by CQ and RCQ
  - rpma_conn_cfg_set_srq - sets a shared RQ object for the connection
  - rpma_conn_get_compl_fd - gets a file descriptor of the shared completion channel from the connection
  - rpma_conn_wait - waits for a completion event on the shared completion channel from CQ or RCQ
  - rpma_srq_cfg_delete - deletes the shared RQ configuration object
  - rpma_srq_cfg_get_rcq_size - gets the receive CQ size of the shared RQ
  - rpma_srq_cfg_get_rq_size - gets the RQ size of the shared RQ
  - rpma_srq_cfg_new - creates a new shared RQ configuration object
  - rpma_srq_cfg_set_rcq_size - sets the receive CQ size of the shared RQ
  - rpma_srq_cfg_set_rq_size - sets the RQ size of the shared RQ
  - rpma_srq_delete - deletes the shared RQ object
  - rpma_srq_get_rcq - gets the receive CQ from the shared RQ object
  - rpma_srq_new - creates a new shared RQ object
  - rpma_srq_recv - initiates the receive operation in shared RQ
  - error RPMA_E_SHARED_CHANNEL - the completion event channel is shared and cannot be handled by any particular CQ
  - error RPMA_E_NOT_SHARED_CHNL - the completion event channel is not shared

- examples:
  - 08srq-simple-messages-ping-pong-with-srq - a single-connection example for shared RQ with ping-pong messages
  - 13-messages-ping-pong-with-srq - a multi-connection example for shared RQ with ping-pong messages

- logging of the source and the destination GID addresses in rpma_conn_req_new_from_id()
- error message for RPMA_E_AGAIN: "Temporary error, try again"
- peer_cfg: get/set_direct_write_to_pmem and get_descriptor are now thread-safe
- conn_cfg: all get and set functions for cq, rq, sq, rcq, timeout and compl_channel are now thread-safe
- multi-threaded tests:
  - rpma_conn_apply_remote_peer_cfg
  - rpma_conn_cfg_get_srq
  - rpma_conn_cfg_set_srq
  - rpma_conn_req_connect
  - rpma_ep_next_conn_req
  - rpma_log_set_function
  - rpma_log_set_get_threshold
  - rpma_log_set_threshold
  - rpma_peer_cfg_set_direct_write_to_pmem
  - rpma_srq_cfg_new
  - rpma_srq_cfg_get_rcq_size
  - rpma_srq_cfg_get_rq_size
  - rpma_srq_cfg_set_rcq_size
  - rpma_srq_cfg_set_rq_size
  - rpma_srq_delete
  - rpma_srq_get_rcq
  - rpma_srq_new

### Changed
- APIs:
  - rpma_cq_wait - returns RPMA_E_SHARED_CHANNEL if the completion channel is shared

- Renamed CMake variables:
  - COVERAGE to TESTS_COVERAGE
  - DEVELOPER_MODE to BUILD_DEVELOPER_MODE
  - TEST_PYTHON_TOOLS to TESTS_PERF_TOOLS
  - TRACE_TESTS to TESTS_VERBOSE_OUTPUT
  - USE_ASAN to DEBUG_USE_ASAN
  - USE_UBSAN to DEBUG_USE_UBSAN

- Changed default values of CMake variables:
  - CMAKE_BUILD_TYPE from Debug to Release
  - TESTS_PERF_TOOLS - from ON to OFF

 - all examples and internal API files now comply with the new character limit per line (100 characters)

### Fixed
- APIs:
  - rpma_peer_delete - fixed memory leak when ibv_dealloc_pd() fails

### Removed
- CMake variables:
  - CHECK_CSTYLE
  - TESTS_LONG
  - TESTS_USE_VALGRIND

- old integration tests
- suppressions for get and set functions for cq, rq, sq and timeout has been removed

## [0.14.0] - 2022-03-15
### Added
- APIs:
  - RPMA_CONN_UNREACHABLE enum rpma_conn_event to handle RDMA_CM_EVENT_UNREACHABLE

### Fixed
- APIs:
  - rpma_log_init - cannot fail to set the default log function now
- unit tests of rpma_log_set_threshold and RPMA_LOG_* macros

### Removed
- APIs:
  - rpma_write_atomic - replaced with rpma_atomic_write

## [0.13.0] - 2022-03-09
### Added
- APIs:
  - rpma_atomic_write - initiates the atomic 8 bytes write operation

### Changed
- ibv_qp_cap.max_inline_data set to 8 bytes to allow implementation of atomic write over ibv_post_send with IBV_SEND_INLINE
- example 07 uses rpma_atomic_write() instead of rpma_write_atomic()

### Deprecated
- API:
  - rpma_write_atomic - replaced with rpma_atomic_write

### Fixed
- the part_write.json template in the benchmarking framework
- documentation of rpma_utils_conn_event_2str()

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
  - Benchmarking framework for librpma.

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

[ddio]: https://github.com/pmem/rpma/blob/main/tools/ddio.sh
[distros]: https://github.com/pmem/rpma/blob/main/.github/workflows/nightly.yml
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
