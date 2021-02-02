Benchmarking mixing `rpma_read()` from PMem on the remote side and **APM** to the same PMem on the remote side in 70:30 ratio (**MIX**) with the results obtained for 100% reads (`rpma_read()` from PMem on the remote side) and 100% writes (writing data using `rpma_write()` and assuring it is written persistently using **APM**).

<h3 id="mix-lat">Test Case 3A: Mix against PMem: Latency</h3>

Comparing the latency of reads and writes in the **MIX** workload (70% reads and 30% writes) vs the latency of `rpma_read()` from PMem on the remote side (100% reads) vs the latency of writing data persistently to PMem on the remote side using **APM** (100% writes).

{{tc\_mix\_lat\_config}}

![](./Figure_017_mix_pmem_vs_rpma_read_apm_pmem_lat_avg.png)

![](./Figure_018_mix_pmem_vs_rpma_read_apm_pmem_lat_pctls.png)

![](./Figure_019_mix_pmem_vs_rpma_read_apm_pmem_lat_avg.png)

![](./Figure_020_mix_pmem_vs_rpma_read_apm_pmem_lat_pctls.png)

<h3 id="mix-bw">Test Case 3B: Mix against PMem: Bandwidth</h3>

Comparing the bandwidth of reads and writes in the **MIX** workload (70% reads and 30% writes) vs the bandwidth of `rpma_read()` from PMem on the remote side (100% reads) vs the bandwidth of writing data persistently to PMem on the remote side using **APM** (100% writes).

{{tc\_mix\_bw\_config}}

![](./Figure_021_mix_pmem_vs_rpma_read_apm_pmem_bw-bs.png)

![](./Figure_022_mix_pmem_vs_rpma_read_apm_pmem_bw-th.png)

![](./Figure_023_mix_pmem_vs_rpma_read_apm_pmem_bw-bs.png)

![](./Figure_024_mix_pmem_vs_rpma_read_apm_pmem_bw-th.png)
