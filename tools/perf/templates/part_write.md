{% if menu is true %}
<a class="pure-menu-heading" href="#write">Write to PMem</a>
<ul class="pure-menu-list">
    <li class="pure-menu-item"><a href="#write-lat" class="pure-menu-link">Latency</a></li>
    <li class="pure-menu-item"><a href="#write-bw" class="pure-menu-link">Bandwidth</a></li>
</ul>
{% else %}

<h2 id="write">Write to PMem</h2>

Benchmarking two ways of writing data persistently to **the RPMA Target**: *Appliance Persistency Method* (**APM**) and *General Purpose Persistency Method* (**GPSPM**). Where:

- **APM** uses `rpma_flush()` following a sequence of `rpma_write()` operations to provide the remote persistency. This method requires **the RPMA Target** to be capable of *Direct Write to PMem* (for details please see [Direct Write to PMem​​](https://pmem.io/rpma/documentation/basic-direct-write-to-pmem.html)).
- **GPSPM** uses `rpma_send()` and `rpma_recv()` operations for sending requests to **the RPMA Target** to assure persistency of the data written using `rpma_write()`. **The RPMA Target** has to provide a thread handling these requests e.g. persisting the data using the `pmem_persist()` operation. When the persistency of the data is assured the response is sent back using also `rpma_send()` and `rpma_recv()`. Depending on how the thread polls for incoming requests you may distinguish two modes:
    - **GPSPM-RT** where the thread polling for incoming requests busy-wait for them (busy\_wait\_polling=1) and
    - **GPSPM** where the thread schedule to be wakened up when a request will appear (busy\_wait\_polling=0). Picking one of these polling modes over another introduces specific challenges and benefits.

For more details on **APM** and **GPSPM** please see: "[Persistent Memory Replication Over Traditional RDMA Part 1: Understanding Remote Persistent Memory](https://software.intel.com/content/www/us/en/develop/articles/persistent-memory-replication-over-traditional-rdma-part-1-understanding-remote-persistent.html)" Chapter "Two Remote Replication Methods".

As a baseline is used **APM** to DRAM on **the RPMA Target** with *Direct Write to PMem* capability **disabled**. Such a configuration allows benefiting from available caching on **the RPMA Target**. Note that data, in this case, is not written persistently on **the RPMA Target** but this configuration is used to show the limit of what is possible regarding data transmission in general and how big is potential overhead when transmitting data using **APM** or **GPSPM** while both of these provide the remote persistency additionally.

<h3 id="write-lat">Write to PMem: Latency</h3>

Comparing the latency of **APM** to PMem on **the RPMA Target** (with *Direct Write to PMem*) vs the latency of **APM** to DRAM on **the RPMA Target** (with *Direct Write to PMem* disabled) (as a baseline) vs the latency of **GPSPM(-RT)** to PMem on **the RPMA Target** (with *Direct Write to PMem* disabled).

{{config.lat}}

{{figure.apm_dram_vs_pmem.lat_avg | figure}}

{{figure.apm_dram_vs_pmem.lat_pctl_999 | figure}}

{{figure.apm_dram_vs_pmem.lat_pctl_9999 | figure}}

{{figure.apm_pmem_vs_gpspm_pmem.lat_avg | figure}}

{{figure.apm_pmem_vs_gpspm_pmem.lat_pctl_999 | figure}}

{{figure.apm_pmem_vs_gpspm_pmem.lat_pctl_9999 | figure}}

<h3 id="write-bw">Write to PMem: Bandwidth</h3>

Comparing the bandwidth of **APM** to PMem on **the RPMA Target** (with *Direct Write to PMem*) vs the bandwidth of **APM** to DRAM on **the RPMA Target** (with *Direct Write to PMem* disabled) (as a baseline) vs the bandwidth of **GPSPM(-RT)** to PMem on **the RPMA Target** (with *Direct Write to PMem* disabled).

{{config.bw}}

{{figure.apm_dram_vs_pmem.bw_bs | figure}}

{{figure.apm_dram_vs_pmem.bw_threads | figure}}

{{figure.apm_pmem_vs_gpspm_pmem.bw_bs | figure}}

{{figure.apm_pmem_vs_gpspm_pmem.bw_threads | figure}}

{% endif %}
