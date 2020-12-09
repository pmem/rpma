# BENCHMARKING

This document describes how to automate collection, processing and presentation of performance numbers (latency and bandwidth) from the RPMA-dedicated FIO engine.

As a baseline the generally accepted tools like `ib_read_lat`, `ib_read_bw` etc. are used which execution, processing and presentation are also automated.

## Requirements

To use these tools, you must have several components installed in your system:
 - python3
 - python3-pip
 - pandas
 - matplotlib

```sh
$ sudo yum install python3 python3-pip
$ pip3 install --user pandas
$ pip3 install --user matplotlib
```

*Note*: All of the scripts presented in the following sections must be run on the client side.

## Comparing the RPMA read with the baseline

### Latency

Generate the baseline latency numbers using `./ib_read_lat.sh` tool which single-sidedly runs `ib_read_lat` with various data sizes:

```sh
$ export JOB_NUMA=0

# -d <IB device> -R (enable rdma_cm QP)
$ export AUX_PARAMS='-d mlx5_0 -R'
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0
$ export REMOTE_AUX_PARAMS='-d mlx5_0 -R'

$ ./ib_read.sh lat <SERVER_IP>
```

Generate latency numbers from the RPMA-dedicated FIO engine using `./rpma_read_lat.sh`:

```sh
$ export JOB_NUMA=0
$ export FIO_PATH=/custom/fio/path
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0
$ export REMOTE_FIO_PATH=/custom/fio/path

# optional, by default: /dev/shm/librpma-server-${TIMESTAMP}.fio
$ export REMOTE_JOB_PATH=/custom/jobs/path

# optional, by default: malloc
$ export REMOTE_JOB_MEM=mmap:/path/to/mem

$ ./rpma_read_lat.sh <SERVER_IP>
```

Generate a comparison using the `csv_compare.py` tool:

```sh
$ ./csv_compare.py --output_layout lat file-1.csv [... file-n.csv]
# generates a png chart using python + pandas + matplotlib
```

With the help of additional parameters, we can also adjust various output aspects.

### Bandwidth

Generate the baseline bandwidth numbers using `./ib_read_bw.sh` tool which single-sidedly runs `ib_read_bw` with various data sizes:

```sh
$ # XXX
```

Generate bandwidth numbers from the RPMA-dedicated FIO engine using `./rpma_read_bw.sh`:

```sh
$ # XXX
```

Generate a comparison using XXX tool:

```sh
$ ./XXX --ib_read_bw ib_read_bw.csv --rpma_read_bw rpma_read_bw.csv
# generates a png chart using python + pandas + matplotlib
```
