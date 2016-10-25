
    #include <TLB.h>
    
    #define FALSE  0
    #define TRUE   1
    struct TLB_entry TLB_entries[]={
         {
         .virtual=0x80000000,
         .physical=0x10000000,
         .pgsize_enum=0b110,
         .permissions=16,
         .wimge=0,
         .pid=1,
         .is_resident= False,
         },
         {
         .virtual=0x80000000,
         .physical=0x10010000,
         .pgsize_enum=0b100,
         .permissions=16,
         .wimge=0,
         .pid=2,
         .is_resident= False,
         },
         {
         .virtual=0x80004000,
         .physical=0x10014000,
         .pgsize_enum=0b10,
         .permissions=4,
         .wimge=0,
         .pid=2,
         .is_resident= False,
         },
         {
         .virtual=0x80005000,
         .physical=0x10015000,
         .pgsize_enum=0b10,
         .permissions=1,
         .wimge=0,
         .pid=2,
         .is_resident= False,
         },
         {
         .virtual=0x80005000,
         .physical=0x10016000,
         .pgsize_enum=0b10,
         .permissions=4,
         .wimge=0,
         .pid=1,
         .is_resident= False,
         },
         {
         .virtual=0x80006000,
         .physical=0x10017000,
         .pgsize_enum=0b10,
         .permissions=1,
         .wimge=0,
         .pid=1,
         .is_resident= False,
         }
        };