Examples for librpma
===

This directory contains examples for librpma,
the library to simplify accessing persistent memory
on remote hosts over Remote Direct Memory Access (RDMA).

If you're looking for documentation to get you started using RPMA,
start here: https://pmem.io/rpma and follow the links
to examples and man pages.

## Running examples on SoftRoCE

This directory contains also the 'run-all-on-SoftRoCE.sh' script
for running all examples on SoftRoCE (they can be also run
from the CMake build directory using 'make run_all_examples'
or 'make run_all_examples_under_valgrind' command).
The '../tools/configure_softroce.sh' script can be used to enable SoftRoCE.
