Examples for librpma
===

This directory contains examples for librpma,
the library to simplify accessing persistent memory
on remote hosts over Remote Direct Memory Access (RDMA).

If you're looking for documentation to get you started using RPMA,
start here: https://pmem.io/rpma and follow the links
to examples and man pages.

## Running examples on SoftRoCE

This directory contains also the 'config_softroce.sh' script
dedicated to configuring SoftRoCE (it can be also run
from the CMake build directory using 'make config_softroce')
and the 'examples/run-all-on-SoftRoCE.sh' script
for running all examples on SoftRoCE (it can be also run
from the CMake build directory using 'make test_softroce').
