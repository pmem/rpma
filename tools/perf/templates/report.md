**Testing Date**: {{test_date}}

{{authors}}

{{audience}}

{{disclaimer}}

<h2 id="test-setup">Test Setup</h2>

{{configuration_common}}

{{configuration_target}}

{{bios}}

{{security}}

<h2 id="introduction">Introduction to RPMA</h2>

{{introduction}}

<h2 id="read">Test Case 1: Read from PMem</h2>

{{tc1_read}}

<h2 id="write">Test Case 2: Write to PMem</h2>

{{tc2_write}}

<h2 id="mix">Test Case 3: Mix against PMem</h2>

{{tc3_mix}}

{% if cpu is true %}
<h2 id="cpu">Test Case 4: CPU load impact</h2>

{{tc4_cpu}}
{% endif %}

**Ref**: {{ref}}
