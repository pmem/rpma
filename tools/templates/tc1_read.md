Benchmarking the `rpma_read()` operation against various data sources (PMem, DRAM) and comparing the obtained results to standard RDMA benchmarking tools: `ib_read_lat` and `ib_read_bw`.

<h3 id="read-lat">Test Case 1A: Read from PMem: Latency</h3>

Comparing the latency of `rpma_read()` from PMem on **the RPMA Target** to the latency of `rpma_read()` from DRAM on **the RPMA Target** using the `ib_read_lat` as the baseline.

{{tc\_read\_lat\_config}}

![](./Figure_001_ib_read_lat_vs_rpma_read_dram_lat_avg.png)

![](./Figure_002_ib_read_lat_vs_rpma_read_dram_lat_pctls.png)

![](./Figure_003_rpma_read_dram_vs_pmem_lat_avg.png)

![](./Figure_004_rpma_read_dram_vs_pmem_lat_pctls.png)

<h3 id="read-bw">Test Case 1B: Read from PMem: Bandwidth</h3>

Comparing the bandwidth of `rpma_read()` from PMem on **the RPMA Target** to the bandwidth of `rpma_read()` from DRAM on **the RPMA Taraget** using the `ib_read_bw` as the baseline.

{{tc\_read\_bw\_config}}

![](./Figure_005_ib_read_bw_vs_rpma_read_dram_bw-bs.png)

![](./Figure_006_ib_read_bw_vs_rpma_read_dram_bw-th.png)

![](./Figure_007_rpma_read_dram_vs_pmem_bw-bs.png)

![](./Figure_008_rpma_read_dram_vs_pmem_bw-th.png)
