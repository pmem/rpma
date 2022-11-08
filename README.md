# **librpma: Remote Persistent Memory Access Library**

[![GHA build status](https://github.com/pmem/rpma/workflows/GitHubActions/badge.svg?branch=main)](https://github.com/pmem/rpma/actions)
[![CircleCI build status](https://circleci.com/gh/pmem/rpma.svg?style=shield)](https://app.circleci.com/pipelines/github/pmem/rpma)
[![Coverity build status](https://scan.coverity.com/projects/21470/badge.svg)](https://scan.coverity.com/projects/pmem-rpma)
[![Coverage Status](https://codecov.io/github/pmem/rpma/coverage.svg?branch=main)](https://codecov.io/gh/pmem/rpma/branch/main)
[![librpma version](https://img.shields.io/github/tag/pmem/rpma.svg)](https://github.com/pmem/rpma/releases/latest)
[![Packaging status](https://repology.org/badge/tiny-repos/rpma.svg)](https://repology.org/project/rpma/versions)

The **Remote Persistent Memory Access (RPMA) Library** is a C library to simplify accessing persistent memory on remote hosts over **Remote Direct Memory Access (RDMA)**. For more information, see
[pmem.io](https://pmem.io).

## Installing

If you want to install this library, check out the [INSTALL.md](INSTALL.md) file.

## Examples

Examples of usage of this library are located in the [./examples/](./examples/) directory and described in the [./examples/README.md](./examples/README.md) file.

## Development

If you want to develop this library, read the [DEVELOPMENT.md](DEVELOPMENT.md) file.

## Contributing

If you want to contribute to the development of this library, you should get familiar with all the following files: [INSTALL.md](INSTALL.md), [DEVELOPMENT.md](DEVELOPMENT.md) and [CONTRIBUTING.md](CONTRIBUTING.md).

## Thread safety

The analysis of thread safety of the librpma library is located in the [THREAD_SAFETY.md](THREAD_SAFETY.md) file.

## Contact Us

For more information on this library, contact
Tomasz Gromadzki (tomasz.gromadzki@intel.com),
Lukasz Dorau (lukasz.dorau@intel.com),
Piotr Balcer (piotr.balcer@intel.com), or post to our
[Google group](https://groups.google.com/group/pmem).

## More Info

[Persistent Memory Over Traditional RDMA White Paper - Part 1](https://software.intel.com/content/www/us/en/develop/articles/persistent-memory-replication-over-traditional-rdma-part-1-understanding-remote-persistent.html) - which describes a technology behind RPMA. **Note:** Parts 2-4 relates directly to the librpmem library which is a predecessor of librpma. So parts 2-4 do not relate exactly to how librpma works.
