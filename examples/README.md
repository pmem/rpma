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
for running all examples on SoftRoCE (it can be also run
from the CMake build directory using 'make test_softroce').
To enable SoftRoCE '../tools/configure_softroce.sh' script can be used.
