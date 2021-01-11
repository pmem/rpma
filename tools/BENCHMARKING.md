# BENCHMARKING

This document describes how to automate collection, processing and presentation of performance numbers (latency and bandwidth) from the RPMA-dedicated FIO engine.

As a baseline the generally accepted tools like `ib_read_lat`, `ib_read_bw` etc. are used which execution, processing and presentation are also automated.

## Requirements

To use these tools, you must have several components installed in your system:
 - python3
 - python3-pip
 - pandas
 - matplotlib
 - perftest (providing ib tools like `ib_read_lat`, `ib_read_bw` etc)
 - fio (supporting the librpma fio engine)
 - numactl
 - sshpass

*Note*: Currently only https://github.com/pmem/fio.git supports the librpma fio engine.

```sh
$ sudo yum install python3 python3-pip numactl sshpass
$ pip3 install --user pandas
$ pip3 install --user matplotlib

$ git clone https://github.com/linux-rdma/perftest.git
$ cd perftest
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
or
$ sudo yum install perftest

$ git clone https://github.com/pmem/fio.git
$ cd fio
$ ./configure
$ make
$ sudo make install
```

*Note*: All of the scripts presented in the following sections must be run on the client side.

## Comparing the RPMA read with the baseline

### Latency

Generate the baseline latency numbers using `./ib_read.sh` tool which single-sidedly runs `ib_read_lat` with various data sizes:
 - iterates over the block sizes (1024B, 4096B, 65536B)
 - thread = 1
 - tx_depth = 1

```sh
$ export JOB_NUMA=0

# -d <IB device> -R (enable rdma_cm QP)
$ export AUX_PARAMS='-d mlx5_0 -R'
$ export REMOTE_USER=user
$ export REMOTE_PASS=pass
$ export REMOTE_JOB_NUMA=0
$ export REMOTE_AUX_PARAMS='-d mlx5_0 -R'

$ ./ib_read.sh <SERVER_IP> lat
```

Generate latency numbers from the RPMA-dedicated FIO engine using `./rpma_fio_bench.sh`:
 - iterates over the block sizes (256B, 1024B, 4096B, 8192B, 16384B, 32768B, 65536B)
 - thread = 1
 - iodepth = 1

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
$ export REMOTE_JOB_MEM_PATH=/path/to/mem

$ ./rpma_fio_bench.sh <SERVER_IP> apm read lat
```

### Bandwidth

Generate the baseline bandwidth numbers using `./ib_read.sh` tool which single-sidedly runs `ib_read_bw` with various data sizes:

 - iterates over the block sizes (256B, 1024B, 4096B, 8192B, 65536B)
 - thread = 1
 - tx_depth = 2

```sh
$ ./ib_read.sh <SERVER_IP> bw-bs
```

 - iterates over the numbers of threads (1, 2, 4, 8, 12)
 - block size = 4096B
 - tx_depth = 2

```sh
$ ./ib_read.sh <SERVER_IP> bw-th
```

 - iterates over the tx_depth values increasing linearly (1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
 - thread = 1
 - block size = 4096B

```sh
$ ./ib_read.sh <SERVER_IP> bw-dp-lin
```

 - iterates over the tx_depth values increasing exponentially (1, 2, 4, 8, 16, 32, 64, 128)
 - thread = 1
 - block size = 4096B

```sh
$ ./ib_read.sh <SERVER_IP> bw-dp-exp
```

Generate bandwidth numbers from the RPMA-dedicated FIO engine using `./rpma_fio_bench.sh`:

 - iterates over the block sizes (256B, 1024B, 4096B, 8192B, 16384B, 32768B, 65536B)
 - thread = 1
 - iodepth = 2

```sh
$ ./rpma_fio_bench.sh <SERVER_IP> apm read bw-bs
```

 - iterates over the numbers of threads (1, 2, 4, 8, 12, 16)
 - block size = 4096B
 - iodepth = 2

```sh
$ ./rpma_fio_bench.sh <SERVER_IP> apm read bw-th
```
 - iterates over the iodepth values increasing linearly (1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
 - thread = 1
 - block size = 4096B

```sh
$ ./rpma_fio_bench.sh <SERVER_IP> apm read bw-dp-lin
```

 - iterates over the iodepth values increasing exponentially (1, 2, 4, 8, 16, 32, 64, 128)
 - thread = 1
 - block size = 4096B

```sh
$ ./rpma_fio_bench.sh <SERVER_IP> apm read bw-dp-exp
```

### Comparison

Generate a comparison using the `csv_compare.py` tool:

```sh
$ ./csv_compare.py --output_layout [lat/bw] file-1.csv [... file-n.csv]
# generates a png chart using python + pandas + matplotlib
```

With the help of additional parameters, we can also adjust various aspects of the output.
