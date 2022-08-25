Examples for librpma
===

This directory contains examples for librpma,
the library to simplify accessing persistent memory
on remote hosts over Remote Direct Memory Access (RDMA).

If you're looking for documentation to get you started using RPMA,
start here: https://pmem.io/rpma and follow the links
to examples and man pages.

## Requirements

In order to build and run all examples you need to have installed additional packages:

- libpmem-dev(el) >= 1.6 or libpmem2-dev(el) >= 1.11 for examples: 3, 4, 5, 7, 9, 9s
- libprotobuf-c-dev(el) >= 1.0 for examples: 9, 9s

**Note**: for more information please check out [this section](../INSTALL.md#for-some-examples-you-also-need).

## Running examples on a configured RDMA-capable network interface

This directory contains also the 'run-all-examples.sh' script
for running all examples on a configured RDMA-capable network interface
(it can be either SoftRoCE or RDMA HW loopback).

The examples can be run also from the CMake build directory using 'make run_all_examples'
or 'make run_all_examples_under_valgrind' command.

The '../tools/configure_softroce.sh' script can be used to configure and enable SoftRoCE.
