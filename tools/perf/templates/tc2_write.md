Benchmarking two ways of writing data persistently to **the RPMA Target**: *Appliance Persistency Method* (**APM**) and *General Purpose Persistency Method* (**GPSPM**). Where:

- **APM** uses `rpma_flush()` following a sequence of `rpma_write()` operations to provide the remote persistency. This method requires **the RPMA Target** to be capable of *Direct Write to PMem* (for details please see [Direct Write to PMem][direct-write-to-pmem]).
- **GPSPM** uses `rpma_send()` and `rpma_recv()` operations for sending requests to **the RPMA Target** to assure persistency of the data written using `rpma_write()`. **The RPMA Target** has to provide a thread handling these requests e.g. persisting the data using the `pmem_persist()` operation. When the persistency of the data is assured the response is sent back using also `rpma_send()` and `rpma_recv()`. Depending on how the thread polls for incoming requests you may distinguish two modes:
    - **GPSPM-RT** where the thread polling for incoming requests busy-wait for them (`busy\_wait\_polling=1`) and
    - **GPSPM** where the thread schedule to be wakened up when a request will appear (`busy\_wait\_polling=0`). Picking one of these polling modes over another introduces specific challenges and benefits.

For more details on **APM** and **GPSPM** please see: "[Persistent Memory Replication Over Traditional RDMA Part 1: Understanding Remote Persistent Memory][rpmem-wp]" Chapter "Two Remote Replication Methods".

As a baseline is used **APM** to DRAM on **the RPMA Target** with *Direct Write to PMem* capability **disabled**. Such a configuration allows benefiting from available caching on **the RPMA Target**. **Note that data, in this case, is not written persistently on the RPMA Target** but this configuration is used to show the limit of what is possible regarding data transmission in general and how big is potential overhead when transmitting data using **APM** or **GPSPM** while both of these provide the remote persistency additionally.

[direct-write-to-pmem]: https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html
[rpmem-wp]: https://software.intel.com/content/www/us/en/develop/articles/persistent-memory-replication-over-traditional-rdma-part-1-understanding-remote-persistent.html

<h3 id="write-lat">Test Case 2A: Write to PMem: Latency</h3>

Comparing the latency of **APM** to PMem on **the RPMA Target** (with *Direct Write to PMem*) vs the latency of **APM** to DRAM on **the RPMA Target** (with *Direct Write to PMem* disabled) (as a baseline) vs the latency of **GPSPM(-RT)** to PMem on **the RPMA Target** (with *Direct Write to PMem* disabled). 

{{tc\_write\_lat\_config}}

![](./Figure_009_apm_dram_vs_pmem_lat_avg.png)

![](./Figure_010_apm_dram_vs_pmem_lat_pctls.png)

![](./Figure_011_apm_pmem_vs_gpspm_pmem_lat_avg.png)

![](./Figure_012_apm_pmem_vs_gpspm_pmem_lat_pctls.png)

<h3 id="write-bw">Test Case 2B: Write to PMem: Bandwidth</h3>

Comparing the bandwidth of **APM** to PMem on **the RPMA Target** (with *Direct Write to PMem*) vs the bandwidth of **APM** to DRAM on **the RPMA Target** (with *Direct Write to PMem* disabled) (as a baseline) vs the bandwidth of **GPSPM(-RT)** to PMem on **the RPMA Target** (with *Direct Write to PMem* disabled). 

{{tc\_write\_bw\_config}}

![](./Figure_013_apm_dram_vs_pmem_bw-bs.png)

![](./Figure_014_apm_dram_vs_pmem_bw-th.png)

![](./Figure_015_apm_pmem_vs_gpspm_pmem_bw-bs.png)

![](./Figure_016_apm_pmem_vs_gpspm_pmem_bw-th.png)
