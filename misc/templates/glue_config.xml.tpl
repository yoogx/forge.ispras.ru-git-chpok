<Partition>
    <Definition Name="P2" System="true"/>
    <!-- Amount of ram required for ELF -->
    <Memory Bytes="1M" Heap="1M"/>

    <Threads Count="10" />

    <ARINC653_Buffers Data_Size="4096" Count="16" />
    <ARINC653_Blackboards Data_Size="4096" Count="16" />
    <ARINC653_Events Count="16" />
    <ARINC653_Semaphores Count="16" />

    <ARINC653_Ports>
        <Queueing_Port Name="UIN"  Protocol="UDP" MaxMessageSize="64" Direction="SOURCE" MaxNbMessage="10" />
        <Queueing_Port Name="UOUT" Protocol="UDP" MaxMessageSize="64" Direction="DESTINATION" MaxNbMessage="10" />
        <Queueing_Port Name="UOUT2" Protocol="UDP" MaxMessageSize="64" Direction="DESTINATION" MaxNbMessage="10" />
        <Sampling_Port Name="SUIN" Protocol="UDP" MaxMessageSize="64" Direction="SOURCE" Refresh="1s"/>
        <Sampling_Port Name="SUOUT" Protocol="UDP" MaxMessageSize="64" Direction="DESTINATION" Refresh="1s"/>
    </ARINC653_Ports>

    <Memory_Blocks>
{% raw %}
 {%if arch_name == 'ppc' %}
     <!-- PCI -->
         <Memory_Block Name="PCI_IO" Size="0x10000" Access="RW" PhysicalAddress="{{bsp.PCI_IO}}" Contiguous="true" CachePolicy="IO"/>
         <Memory_Block Name="PCI_Express_1" Size="0x1000" Access="RW" PhysicalAddress="{{bsp.PCI_EXPRESS_CONTROLLER_1}}" Contiguous="true" CachePolicy="IO"/>
 {%endif%}
{% endraw %}

{% for comp in components %}
 {% for mb in comp.memory_blocks %}
  {% if mb.is_per_instance %}
    {% for instance_name in components_instances[comp.name] %}
      <Memory_Block Name="{{instance_name}}_{{mb.name}}"
         Size="{{mb.size}}" Access="RW" {{'Contiguous="true"' if mb.contiguous}}  {{ 'PhysicalAdress="%s"'%mb.physical_address if mb.physical_address}} CachePolicy="{{mb.cachepolicy}}"/>
    {% endfor %}
  {% else %}
      <Memory_Block Name="{{mb.name}}"
         Size="{{mb.size}}" Access="RW" {{'Contiguous="true"' if mb.contiguous}}  {{ 'PhysicalAdress="%s"'%mb.physical_address if mb.physical_address}} CachePolicy="{{mb.cachepolicy}}"/>
  {% endif %}

 {% endfor %}
{% endfor %}

    </Memory_Blocks>

</Partition>
