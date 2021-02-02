Benchmarking two ways of writing data persistently on the remote-side: *Appliance Persistency Method* (**APM**) and *General Purpose Persistency Method* (**GPSPM**). Where:

- **APM** uses `rpma_flush()` following a sequence `rpma_write()` operations to provide the remote persistency. This method requires the remote-side to be capable of *Direct Write to PMem* (for details please see [Direct Write to PMem][direct-write-to-pmem]).
- **GPSPM** uses `rpma_send()` and `rpma_recv()` operations for sending requests to the remote-side to assure persistency of the data written using `rpma_write()`. The remote side has to provide a thread handling these requests e.g. persisting the data using the `pmem_persist()` operation. When the persistency of the data is assured the response is sent back using also `rpma_send()` and `rpma_recv()`.

As a baseline is used **APM** to DRAM on the remote side without *Direct Write to PMem* capability. Such a configuration allows benefiting from available caching on the remote side. Note data, in this case, is not written persistently on the remote side but it is used to show the limit of what is possible regarding data transmission in general and how big is potential overhead when transmitting data using **APM** or **GPSPM** while both of these provides the remote persistency at the same time.

[direct-write-to-pmem]: https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html

<h3 id="write-lat">Test Case 2A: Write to PMem: Latency</h3>

Comparing the latency of **APM** to PMem on the remote side (with *Direct Write to PMem*) vs the latency of **APM** to DRAM on the remote side (without *Direct Write to PMem*) (as a baseline) vs the latency of **GPSPM** to PMem on the remote side (without *Direct Write to PMem*). 

{{tc\_write\_lat\_config}}

![](./Figure_009_apm_dram_vs_pmem_lat_avg.png)

![](./Figure_010_apm_dram_vs_pmem_lat_pctls.png)

![](./Figure_011_apm_pmem_vs_gpspm_pmem_lat_avg.png)

![](./Figure_012_apm_pmem_vs_gpspm_pmem_lat_pctls.png)

<h3 id="write-bw">Test Case 2B: Write to PMem: Bandwidth</h3>

Comparing the bandwidth of **APM** to PMem on the remote side (with *Direct Write to PMem*) vs the bandwidth of **APM** to DRAM on the remote side (without *Direct Write to PMem*) (as a baseline) vs the bandwidth of **GPSPM** to PMem on the remote side (without *Direct Write to PMem*). 

{{tc\_write\_bw\_config}}

![](./Figure_013_apm_dram_vs_pmem_bw-bs.png)

![](./Figure_014_apm_dram_vs_pmem_bw-th.png)

![](./Figure_015_apm_pmem_vs_gpspm_pmem_bw-bs.png)

![](./Figure_016_apm_pmem_vs_gpspm_pmem_bw-th.png)
