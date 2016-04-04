#include <sysconfig.h>
#include <pci.h>
enum {
    PCI_DRIVER_TABLE_SIZE = 10,
    DYNAMIC_MEMORY_SIZE = 0x100000,
};

struct pci_driver pci_driver_table[PCI_DRIVER_TABLE_SIZE];
unsigned pci_driver_table_size = PCI_DRIVER_TABLE_SIZE;


char dynamic_memory[DYNAMIC_MEMORY_SIZE];
unsigned dynamic_memory_size = DYNAMIC_MEMORY_SIZE;
