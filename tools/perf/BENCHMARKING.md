# BENCHMARKING

This document describes how to automate collection, processing and presentation of performance numbers (latency and bandwidth) from the RPMA-dedicated FIO engine.

As a baseline the generally accepted tools like `fio`, `ib_read_lat` and `ib_read_bw` are used which execution, processing and presentation are also automated.

## Requirements

You must have several components installed in your system in order to use the benchmarking tools:
 - python3
 - python3-pip
 - matplotlib
 - perftest (providing ib tools like `ib_read_lat`, `ib_read_bw` etc)
 - fio >= 3.27
 - numactl
 - pciutils (needed by ddio.sh, required only in case of the Cascade Lake platforms)

*Note*: The newest features are available on the development branch: https://github.com/pmem/fio.git

```sh
$ sudo yum install python3 python3-pip numactl
$ pip3 install --user matplotlib

$ sudo yum install perftest
or
$ git clone https://github.com/linux-rdma/perftest.git
$ cd perftest
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install

$ git clone https://github.com/axboe/fio.git
$ cd fio
$ ./configure
$ make
$ sudo make install
```

To use the reporting tools (e.g. `report_bench.py`, `report_figures.py`, `report_create.py`), you must additionally install:
 - deepdiff
 - jinja2
 - markdown2
 - paramiko
 - PIL
 - scp

```sh
$ pip3 install --user deepdiff
$ pip3 install --user jinja2
$ pip3 install --user markdown2
$ pip3 install --user paramiko
$ pip3 install --user PIL
$ pip3 install --user scp
```

*Note*: All of the scripts presented in the following sections must be run on the RPMA initiator side.


## Generating a report

In order to generate the RPMA performance report, follow these steps:

### 1) Prepare a config.json file

Make a copy of config.json.example (config.json) and adjust it to describe your configuration. For details on all available options please see [CONFIG.JSON.md](CONFIG.JSON.md).

### 2) Run benchmarks

You can choose from few predefined sets of benchmarks covering different aspects. The predefined benchmarks are stored in the `./figures/*.json` files. You can run one or more of them at once e.g.:

```sh
$ ./report_bench.py run --config config.json --figures figures/read.json figures/write.json --result_dir results
```

If you want to continue an interrupted benchmarking process or if you want to run more benchmarks for different configuration parameters, you can change them in the 'results/bench.json' file and run:

```sh
$ ./report_bench.py continue --bench results/bench.json
```

For example, in case of Intel Ice Lake platforms, in order to continue a benchmark for a different value of `DIRECT_WRITE_TO_PMEM`, a state of DDIO has to be changed in the BIOS and the benchmark can be continued after the reboot and changing a value of `DIRECT_WRITE_TO_PMEM` in the 'results/bench.json' file.

To see all available configuration options please take a look at the help:

```sh
$ ./report_bench.py -h
```

### 3) Generate figures

```sh
$ ./report_figures.py generate --bench results/bench.json
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_figures.py generate -h
```

### 4) Generate the performance report

Make a copy of report.json.example (report.json) and adjust it to describe your configuration.

```sh
$ ./report_create.py --bench results/bench.json --report report.json
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_create.py -h
```
### 5) Watermarking (optional)

An HTML document can be easily transformed into a PDF document using one of many available `Print to PDF...` software. You may find useful also an option to overlay its contents with a watermark of your choice.

One way of preparing a watermark is by using Microsoft PowerPoint:

1. Prepare a single slide and adjust its size to the size of the PDF document.
2. Make the background of the slide 100% transparent.
3. Add watermark elements of your choice.
4. Export the slide to a PDF file.

Having the `watermark.pdf` file prepared you can overlay it on all pages of your PDF report as follows:

```sh
$ pdftk report.pdf stamp watermark.pdf output report_with_watermark.pdf
```

## Comparing

```sh
$ ./report_figures.py compare --benches results_1/bench.json results_2/bench.json
--prefixes results_1 results_2 --result_dir compare
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_figures.py compare -h
```

## Running simple workloads

Instead of running a comprehensive set of workloads you can run a simple subset of them.
In order to do so you have to modify the existing [figures](./figures/).
An example of a simple figure you can find [here](./figures/example_read.json).

## Prerequisites for benchmarking

Run ssh manually from the client to the server at least once before report_bench.py is run:
``` sh
$ ssh ${REMOTE_USER}@${SERVER_IP}
```

Before starting any work, make sure that you have the correct date in the operating system:
``` sh
$ sudo date -s "01/28/2022 10:03:00"
```

If you have set up FsDAX or DevDAX make sure you have the RW access to these devices:
``` sh
$ sudo chmod 777 /mnt/pmem0 
$ sudo chmod 666 /dev/dax0.0
```
