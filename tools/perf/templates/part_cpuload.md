{% if menu is true %}
<a class="pure-menu-heading" href="#cpuload">CPU load impact</a>
<ul class="pure-menu-list">
    <li class="pure-menu-item"><a href="#cpuload-lat" class="pure-menu-link">Latency</a></li>
    <li class="pure-menu-item"><a href="#cpuload-bw" class="pure-menu-link">Bandwidth</a></li>
</ul>
{% else %}

<h2 id="cpuload" class="page-break">CPU load impact</h2>

Benchmarking an impact of the CPU load running on **the RPMA Target** on writing data persistently using both ways of achieving remote persistency: **APM** and **GPSPM(-RT)**.

As a starting point are chosen well-performing workload configurations in respect of either latency or bandwidth. The CPU load on **the RPMA Target** is expected to affect all persistency methods requiring active participation of the process running on **the RPMA Target** namely **GPSPM-RT** and **GPSPM**. Whereas **APM** since it does not make use of the CPU of **the RPMA Target** should not be significantly affected.

<h3 id="cpuload-lat">CPU load impact: Latency</h3>

Comparing the latency of **APM** to PMem on **the RPMA Target** vs the latency of **GPSPM(-RT)** to PMem on **the RPMA Target** (with *Direct Write to PMem* disabled) affected by various CPU loads running on **the RPMA Target**.

{{config.lat}}

{{figure.apm_vs_gpspm_cpu_00_99.lat_avg | figure}}

{{figure.apm_vs_gpspm_cpu_00_99.lat_pctl_999 | figure}}

{{figure.apm_vs_gpspm_cpu_00_99.lat_pctl_9999 | figure}}

{{figure.apm_vs_gpspm_cpu_75_99.lat_avg | figure}}

{{figure.apm_vs_gpspm_cpu_75_99.lat_pctl_999 | figure}}

{{figure.apm_vs_gpspm_cpu_75_99.lat_pctl_9999 | figure}}

<h3 id="cpuload-bw" class="page-break">CPU load impact: Bandwidth</h3>

Comparing the bandwidth of **APM** to PMem on **the RPMA Target** vs the bandwidth of **GPSPM(-RT)** to PMem on **the RPMA Target** affected by various CPU loads running on **the RPMA Target**.

{{config.bw}}

{{figure.apm_vs_gpspm_cpu_00_99.bw | figure}}

{{figure.apm_vs_gpspm_cpu_00_99.bw_mt | figure}}

{{figure.apm_vs_gpspm_cpu_75_99.bw | figure}}

{{figure.apm_vs_gpspm_cpu_75_99.bw_mt | figure}}

{% endif %}
