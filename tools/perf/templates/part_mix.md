{% if menu is true %}
<a class="pure-menu-heading" href="#mix">Mix against PMem</a>
<ul class="pure-menu-list">
    <li class="pure-menu-item"><a href="#mix-lat" class="pure-menu-link">Latency</a></li>
    <li class="pure-menu-item"><a href="#mix-bw" class="pure-menu-link">Bandwidth</a></li>
</ul>
{% else %}

<h2 id="mix" class="page-break">Mix against PMem</h2>

Benchmarking mixed reads from PMem (`rpma_read()`) on the **RPMA Target** and writes (**APM**) to the same PMem on **the RPMA Target** side in the configured ratio (**MIX**) and comparing it to pure `rpma_read()` (100% reads) and **APM** (100% writes). **The RPMA Target** is configured to be capable of *Direct Write to PMem* (**DW2PMem**)

<h3 id="mix-lat">Mix against PMem: Latency</h3>

Comparing the latency of `rpma_read()` and **APM** in the **MIX** workload vs the latency of 100% reads and 100% writes configurations.
{{config.lat}}

{{figure.mix_seq.lat_avg | figure}}

{{figure.mix_seq.lat_pctl_999 | figure}}

{{figure.mix_seq.lat_pctl_9999 | figure}}

{{figure.mix_rand.lat_avg | figure}}

{{figure.mix_rand.lat_pctl_999 | figure}}

{{figure.mix_rand.lat_pctl_9999 | figure}}

<h3 id="mix-bw" class="page-break">Mix against PMem: Bandwidth</h3>

Comparing the bandwidth of `rpma_read()` and **APM** in the **MIX** workload vs the latency of 100% reads and 100% writes configurations.

{{config.bw}}

{{figure.mix_seq.bw_bs | figure}}

{{figure.mix_seq.bw_threads | figure}}

{{figure.mix_rand.bw_bs | figure}}

{{figure.mix_rand.bw_threads | figure}}

{% endif %}
