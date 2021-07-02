# Contributing to the RPMA performance tools

Down below you'll find instructions on how to contribute to the RPMA performance tools.

**Note**: [The general contributing rules of this repository](https://github.com/pmem/rpma/blob/master/CONTRIBUTING.md) still applies. Moreover what is stated below.

## Requirements

To develop the Python performance tools, you must have several components installed in your system:

- pytest
- coverage

```shell
$ pip3 install --user pytest coverage
```

## Code validation

As of now, Python scripts are not yet automatically tested in our CIs. To test introduced changes please use `pytest`:

```shell
$ pytest
```

## Code coverage

As of now, Python scripts is not yet automatically tested for coverage. To test introduced changes for coverage please use `coverage`:

```shell
$ coverage run --source=report_bench,report_create,report_figures,lib -m pytest
$ coverage report
```
