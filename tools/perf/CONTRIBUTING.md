# Contributing to the RPMA performance tools

Down below you'll find instructions on how to contribute to the RPMA performance tools.

**Note**: [The general contributing rules of this repository](https://github.com/pmem/rpma/blob/master/CONTRIBUTING.md) still applies. Moreover what is stated below.

## Prerequisites

Before you start developing the performance tools, ensure that you have installed all components required to run the tools as it is described in
[BENCHMARKING.md](https://github.com/pmem/rpma/blob/master/tools/perf/BENCHMARKING.md#requirements) document.

## Requirements

To develop the Python performance tools, you must have several components installed in your system:

- pdoc3
- pylint
- pytest
- coverage

```shell
$ pip3 install --user pdoc3 pylint pytest coverage
```

## Documentation

When you happen to build or fix Python files docstrings please use [Google docstrings](https://github.com/google/styleguide/blob/gh-pages/pyguide.md#38-comments-and-docstrings).

To build HTML or PDF documentation of Python scripts and interfaces available in our repository you can use `pdoc3`:

```shell
$ pdoc3 --html -o doc *.py lib
```

**Note**: `pdoc3` does not generate an HTML index file for Python scripts but you can open each one of them individually in a web browser. There is a regular `index.html` file inside the `lib/` subdirectory that you can use to easily navigate all the modules inside the `lib` package.

Alternatively, you can read docstrings directly inside Python files using your favourite text editor.

## Code validation

To test changes introduced to Python scripts and modules before testing them automatically in our CIs please use `pytest` and/or `pylint`:

```shell
# to run source code, bug and quality check on a given file
$ pylint --rcfile=../../utils/pylint.rc $FILE
# to run unittests
$ pytest
```

## Code coverage

As of now, Python scripts is not yet automatically tested for coverage. To test introduced changes for coverage please use `coverage`:

```shell
$ coverage run --source=report_bench,report_create,report_figures,lib -m pytest
$ coverage report
```
