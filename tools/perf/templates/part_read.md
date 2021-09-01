{% if menu is true %}
<a class="pure-menu-heading" href="#read">Read from PMem</a>
<ul class="pure-menu-list">
    <li class="pure-menu-item"><a href="#read-lat" class="pure-menu-link">Latency</a></li>
    <li class="pure-menu-item"><a href="#read-bw" class="pure-menu-link">Bandwidth</a></li>
</ul>
{% else %}

<h2 id="read" class="page-break">Read from PMem</h2>

Benchmarking the `rpma_read()` operation against various data sources (PMem, DRAM) and comparing the obtained results to standard RDMA benchmarking tools: `ib_read_lat` and `ib_read_bw`.

<h3 id="read-lat">Read from PMem: Latency</h3>

Comparing the latency of `rpma_read()` from PMem on **the RPMA Target** to the latency of `rpma_read()` from DRAM on **the RPMA Target** using the `ib_read_lat` as the baseline.

{{config.lat}}

{{figure.ib_read_vs_rpma_read_dram.lat_avg | figure}}

{{figure.ib_read_vs_rpma_read_dram.lat_pctl_999 | figure}}

{{figure.ib_read_vs_rpma_read_dram.lat_pctl_9999 | figure}}

{{figure.rpma_read_dram_vs_pmem.lat_avg | figure}}

{{figure.rpma_read_dram_vs_pmem.lat_pctl_999 | figure}}

{{figure.rpma_read_dram_vs_pmem.lat_pctl_9999 | figure}}

<h3 id="read-bw" class="page-break">Read from PMem: Bandwidth</h3>

Comparing the bandwidth of `rpma_read()` from PMem on **the RPMA Target** to the bandwidth of `rpma_read()` from DRAM on **the RPMA Target** using the `ib_read_bw` as the baseline.

{{config.bw}}

{{figure.ib_read_vs_rpma_read_dram.bw_bs | figure}}

{{figure.ib_read_vs_rpma_read_dram.bw_threads | figure}}

{{figure.rpma_read_dram_vs_pmem.bw_bs | figure}}

{{figure.rpma_read_dram_vs_pmem.bw_threads | figure}}

{% endif %}
