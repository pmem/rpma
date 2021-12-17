# BENCHMARKING

This document describes how to automate collection, processing and presentation of performance numbers (latency and bandwidth) from the RPMA-dedicated FIO engine.

As a baseline the generally accepted tools like `ib_read_lat`, `ib_read_bw` etc. are used which execution, processing and presentation are also automated.

## Requirements

You must have several components installed in your system in order to use the benchmarking tools:
 - python3
 - python3-pip
 - pandas
 - matplotlib
 - perftest (providing ib tools like `ib_read_lat`, `ib_read_bw` etc)
 - fio >= 3.27
 - numactl
 - sshpass
 - pciutils (for ddio.sh which may be invoked from `rpma_fio_bench.sh`)

*Note*: The newest features are available on the development branch: https://github.com/pmem/fio.git

```sh
$ sudo yum install python3 python3-pip numactl sshpass
$ pip3 install --user pandas
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

To use the reporting tools (e.g. `csv_compare.py`, `report_bench.py`, `report_figures.py`, `report_create.py`), you must additionally install:
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

*Note*: All of the scripts presented in the following sections must be run on the client side.


## Generating a report

In order to generate the RPMA performance report, follow these steps:

### 1) Prepare a config.json file

Make a copy of config.json.example (config.json) and adjust it to describe your configuration. For details on all available options please see [CONFIG.JSON.md](CONFIG.JSON.md).

### 2) Run benchmarks

You can choose from few predefined sets of benchmarks covering different aspects. The predefined benchmarks are stored in the `./figures/*.json` files. You can run one or more of them at once e.g.:

```sh
$ ./report_bench.py run --config config.json --figures figures/read.json figures/write.json --result_dir results
```

If you want to continue an interrupted benchmark or if you want to run more benchmarks for different configuration parameters, you can change them in the 'results/bench.json' file and run:

```sh
$ ./report_bench.py continue --bench results/bench.json
```

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
--names results_1 results_2 --result_dir compare
```

To see all available configuration options please take a look at the help:

```sh
$ ./report_figures.py compare -h
```

## Running separate workloads
Instead of running a comprehensive set of workloads you can run any separate subset of them. There are a few tools that can be used for automatic running RPMA-related workloads:

- `ib_read.sh` - a tool using `ib_read_lat` and `ib_read_bw` to benchmark the baseline performance of RDMA read operation
- `rpma_fio_bench.sh` - a tool using librpma-dedicated FIO engines for benchmarking remote memory manipulation (reading, writing APM-style, writing GPSPM-style, mixed). These workloads can be run against PMem and DRAM as well.

### Example of `ib_read.sh` use

Generate the baseline latency numbers using `./ib_read.sh` tool:

```sh
$ export JOB_NUMA=0
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0

$ ./ib_read.sh $SERVER_IP lat
```

To see all available configuration options please take a look at the help:

```sh
$ ./ib_read.sh
```

### Example of `rpma_fio_bench.sh` use

Generate latency numbers from the RPMA-dedicated FIO engine using `./rpma_fio_bench.sh`:

```sh
$ export JOB_NUMA=0
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0
$ export REMOTE_JOB_MEM_PATH=/dev/dax1.0

$ ./rpma_fio_bench.sh $SERVER_IP apm read lat
```

To see all available configuration options please take a look at the help:

```sh
$ ./rpma_fio_bench.sh
```

## Analyzing the results

All of the benchmarking tools described above generate standardized CSV format output files, which can be further processed using `csv_compare.py` to generate comparative charts.

### Example of comparing the obtained results

To generate a chart comparing the obtained results you can feed them into the script as follows:

```sh
$ ./csv_compare.py --output_layout lat_avg ib_read_lat-21-01-31-072033.csv rpma_fio_apm_read_lat_th1_dp1_dev_dax1.0-21-01-31-073733.csv --output_with_tables
```

With the help of additional parameters, you can adjust various aspects of the output.
