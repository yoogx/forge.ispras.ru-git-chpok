#include <sysconfig.h>
#include <pci.h>
enum {
    PCI_DRIVER_TABLE_SIZE = 10
};

struct pci_driver pci_driver_table[PCI_DRIVER_TABLE_SIZE];
unsigned pci_driver_table_size = PCI_DRIVER_TABLE_SIZE;


