Benchmarking an impact of the CPU load running on **the RPMA Target** on writing data persistently using both ways of achieving remote persistency: **APM**, **GPSPM-RT**, and **GPSPM**.

As a starting point are chosen well-performing workload configurations in respect of either latency or bandwidth. The CPU load on **the RPMA Target** is expected to affect all persistency methods requiring active participation of the process running on **the RPMA Target** namely **GPSPM-RT** and **GPSPM**. Whereas **APM** since it does not make use of the CPU of **the RPMA Target** should not be significantly affected.

<h3 id="cpu-lat">Test Case 4A: CPU load impact: Latency</h3>

Comparing the latency of **APM** to PMem on **the RPMA Target** vs the latency of **GPSPM(-RT)** to PMem on **the RPMA Target** (with *Direct Write to PMem* disabled) affected by various CPU loads running on **the RPMA Target**

{{tc\_cpu\_lat\_config}}

![](./Figure_025_apm_pmem_vs_gpspm_pmem_cpu_00_100_lat_avg.png)

![](./Figure_026_apm_pmem_vs_gpspm_pmem_cpu_75_100_lat_avg.png)

![](./Figure_027_apm_pmem_vs_gpspm_pmem_cpu_00_100_lat_pctls.png)

![](./Figure_028_apm_pmem_vs_gpspm_pmem_cpu_75_100_lat_pctls.png)

<h3 id="cpu-bw">Test Case 4B: CPU load impact: Bandwidth</h3>

Comparing the bandwidth of **APM** to PMem on **the RPMA Target** vs the bandwidth of **GPSPM(-RT)** to PMem on **the RPMA Target** affected by various CPU loads running on **the RPMA Target**

{{tc\_cpu\_bw\_config}}

![](./Figure_029_apm_pmem_vs_gpspm_pmem_cpu_00_100_bw.png)

![](./Figure_030_apm_pmem_vs_gpspm_pmem_cpu_75_100_bw.png)

![](./Figure_031_apm_pmem_vs_gpspm_pmem_cpu_00_100_bw_mt.png)

![](./Figure_032_apm_pmem_vs_gpspm_pmem_cpu_75_100_bw_mt.png)