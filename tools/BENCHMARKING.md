# BENCHMARKING

This document describes how to automate collection, processing and presentation of performance numbers (latency and bandwidth) from the RPMA-dedicated FIO engine.

As a baseline are used the generally accepted tools like `ib_read_lat`, `ib_read_bw` etc. which execution, processing and presentation are also automated.

## Comparing a RPMA read with the baseline

### Latency

Generate the baseline latency numbers using `./ib_read_lat.sh` tool which single-sidedly runs `ib_read_lat` with vaiours data sizes:

```sh
$ # XXX
```

Generate latency numbers from the RPMA-dedicated FIO engine using `./rpma_read_lat.sh`:

```sh
$ # XXX
```

Generate a comparison using XXX tool:

```sh
$ ./XXX --ib_read_lat ib_read_lat.csv --rpma_read_lat rpma_read_lat.csv
# generates a png chart using python + pandas + matplotlib
```

### Bandwidth

Generate the baseline bandwidth numbers using `./ib_read_bw.sh` tool which single-sidedly runs `ib_read_bw` with vaiours data sizes:

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
