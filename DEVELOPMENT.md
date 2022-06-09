# DEVELOPMENT ENVIRONMENT SETTINGS

XXX

# Testing

This section describes how to prepare the environment for execution of all available kinds of tests:
- unit tests,
- multi-threaded (MT) tests and
- integration tests.

## Run unit and multi-threaded tests on SoftRoCE

XXX

## Run unit and multi-threaded tests on RDMA HW

XXX

## Run integration tests on SoftRoCE/RDMA HW

The integration tests are implemented as examples run together with the fault injection mechanism.

In order to run the integration tests on SoftRoCE/RDMA HW, the `RPMA_TESTING_IP` environment variable
has to be set to an IP address of a configured RDMA-capable network interface
and then they can be started with the following command:

```sh
$ make run_all_examples_with_fault_injection
```

By default the integration tests do not stop on failures. In order to stop on the first failure,
the `RPMA_INTEGRATION_TESTS_STOP_ON_FAILURE` environment variable has to be set to `ON`
or the following command has to be run:

```sh
$ ./examples/run-all-examples.sh ./build/examples/ --fault-injection --stop-on-failure
```

from the main directory of the librpma repository.
