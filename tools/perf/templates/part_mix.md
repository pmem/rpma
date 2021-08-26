{% if menu is true %}
<a class="pure-menu-heading" href="#mix">Mix against PMem</a>
<ul class="pure-menu-list">
    <li class="pure-menu-item"><a href="#mix-lat" class="pure-menu-link">Latency</a></li>
    <li class="pure-menu-item"><a href="#mix-bw" class="pure-menu-link">Bandwidth</a></li>
</ul>
{% else %}

<h2 id="mix">Mix against PMem</h2>

Benchmarking mixing reads from PMem on the **RPMA Target** and writes (**APM**) to the same PMem on **the RPMA Target** side in 70:30 ratio (**MIX**).

<h3 id="mix-lat">Mix against PMem: Latency</h3>

Comparing the latency of reads and writes (**APM**) in the **MIX** workload (70% reads and 30% writes) vs the latency of `rpma_read()` from PMem on **the RPMA Target** (100% reads) vs the latency of writing data persistently to PMem on **the RPMA Target** using **APM** (100% writes).

{{config.lat}}

{{figure.mix_seq.lat_avg}}

{{figure.mix_seq.lat_pctl_999}}

{{figure.mix_seq.lat_pctl_9999}}

{{figure.mix_rand.lat_avg}}

{{figure.mix_rand.lat_pctl_999}}

{{figure.mix_rand.lat_pctl_9999}}

<h3 id="mix-bw">Mix against PMem: Bandwidth</h3>

Comparing the bandwidth of reads and writes (**APM**) in the **MIX** workload (70% reads and 30% writes) vs the bandwidth of `rpma_read()` from PMem on **the RPMA Target** (100% reads) vs the bandwidth of writing data persistently to PMem on **the RPMA Target** using **APM** (100% writes).

{{config.bw}}

{{figure.mix_seq.bw_bs}}

{{figure.mix_seq.bw_threads}}

{{figure.mix_rand.bw_bs}}

{{figure.mix_rand.bw_threads}}

{% endif %}
