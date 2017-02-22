<Partition>
    <Definition Name="P2" System="true"/>
    <!-- Amount of ram allocated (code + stack + static variables) + heap -->
    <Memory Bytes="1M" Heap="1M"/>

    <!-- Number of threads that can be created in this partition.
         Note that this number doesn't include main and error handler threads,
         (the former always exists, and the latter can always be created).

         Values less than 1 probably don't make sense, because otherwise
         you won't be able to create any threads that can be run in
         NORMAL partition state.
        -->
    <Threads Count="10" />

    <ARINC653_Buffers Data_Size="4096" Count="16" />
    <ARINC653_Blackboards Data_Size="4096" Count="16" />
    <ARINC653_Events Count="16" />
    <ARINC653_Semaphores Count="16" />

    <ARINC653_Ports>
        <!-- Those correspond to 
             A653_SamplingPortType and A653_QueueingPortType
             defined in the standard
        -->
        <Queueing_Port Name="UIN"  Protocol="UDP" MaxMessageSize="64" Direction="SOURCE" MaxNbMessage="10" />
        <Queueing_Port Name="UOUT" Protocol="UDP" MaxMessageSize="64" Direction="DESTINATION" MaxNbMessage="10" />
    </ARINC653_Ports>

    <Memory_Blocks>
{%if arch_name == 'ppc' %}
        <!-- Powerpc only -->
            <Memory_Block Name="PCI_IO" Size="0x10000" Access="RW" PhysicalAddress="{{bsp.PCI_IO}}" Contiguous="true" CachePolicy="IO"/>
            <Memory_Block Name="PCI_Express_1" Size="0x1000" Access="RW" PhysicalAddress="{{bsp.PCI_EXPRESS_CONTROLLER_1}}" Contiguous="true" CachePolicy="IO"/>
{%endif%}
        <Memory_Block Name="Virtio_Heap" Size="0xc3000" Access="RW" Contiguous="true" CachePolicy="IO"/>
    </Memory_Blocks>

</Partition>
