# DEVELOPMENT ENVIRONMENT SETTINGS

XXX

# Testing

This section describes how to prepare the environment for execution of all available kinds of tests:
- unit tests,
- multi-threaded (MT) tests and
- integration tests.

## Run unit tests

One of the basic principles of delivering new functionality to a library is to deliver it simultaneously as the unit tests to ensure that the new features behave correctly.
You have to expect that code that degrades test coverage will not go through the review process unless it is explicitly marked as a draft.

The unit tests are written using the cmocka framework, for more information please see [cmocka](https://cmocka.org/).

Please see ``./tests/unit/`` directory for examples

To run unit tests you need to:

```shell
$ cd build
$ cmake ..
$ make -j${nproc}
$ make test
```
``ctest`` command can be used instead of ``make test``

## Run multi-threaded tests

To run all multithreaded tests we need configured RDMA-capable network interface.

### Run multi-threaded tests on SoftRoCE:
- configure SoftRoCE with tools/config_softroce.sh script or ``make config_softroce``
- set the environment variable RPMA_TESTING_IP
```shell
$ export RPMA_TESTING_IP=127.0.0.0
```
- turn on building the multi-threaded tests and run them
```shell
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=ON -DTESTS_USE_VALGRIND=ON ..
$ make -j${nproc}
$ make test
```
``ctest`` command can be used instead of ``make test``

### Run multi-threaded tests on RDMA HW:
- configure RDMA HW
- set the environment variable RPMA_TESTING_IP
```shell
$ export RPMA_TESTING_IP=127.0.0.0
```
- turn on building the multi-threaded tests and run them
```shell
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DTESTS_RDMA_CONNECTION=ON -DTESTS_USE_VALGRIND=ON ..
$ make -j${nproc}
$ make test
```
``ctest`` command can be used instead of ``make test``

## Run integration tests on SoftRoCE

XXX

## Run integration tests on RDMA HW

XXX
