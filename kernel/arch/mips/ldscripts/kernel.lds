OUTPUT_FORMAT("elf32-tradbigmips");
OUTPUT_ARCH("mips")
ENTRY(_pok_reset)

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = 0x80000000;

  .text           :
  {
    *(.start)
    *(.text .text.* .gnu.linkonce.t.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    __text_end = .;
    . = ALIGN(4);
  } = 0

  .rodata :
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.rodata1)
    . = ALIGN(4);

    __archive_begin = .;
    *(.archive)
    __archive_end = .;
    . = ALIGN(4);
    __archive2_begin = .;
    *(.archive2)
    __archive2_end = .;

    __rodata_end = .;
  } =0

  /* . = 0x20000; */
  . = ALIGN(16);

  .sdata2 : 
  {
    __sdata2_start = .;
    PROVIDE (_SDA2_BASE_ = 32768);
    *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
    . = ALIGN(4);
    __sdata2_end = .;
  }
  
  .data :
  {
    __data_start = .;
    *(.data .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    SORT(CONSTRUCTORS)
    *(.data1)
    PROVIDE (_SDA_BASE_ = 32768);
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    . = ALIGN(4);
    __data_end = .;
  }
  .sbss :
  {
    __sbss_start = .;
    *(.dynsbss)
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
    . = ALIGN(4);
    __sbss_end = .;
  }
  .sbss2 :
  {
    __sbss2_start = .;
    *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
    . = ALIGN(4);
    __sbss2_end = .;
  }
  .eh_frame_hdr : { *(.eh_frame_hdr) *(.eh_frame_entry .eh_frame_entry.*) }
  .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) *(.eh_frame.*) }
  .bss  :
  {
   __bss_start = .;
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;

   /* Allocate the main stack area.  Make sure it starts 16bytes aligned
      to accommodate any possible program expectation.  */
   _stack = ALIGN(16) + 0x1000;
   _end = _stack;
  }
}
