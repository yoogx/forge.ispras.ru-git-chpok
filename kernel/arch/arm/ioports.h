#include <stdint.h>

static inline void iowrite32(uint32_t reg, uint32_t data)
{
    *(volatile uint32_t *)reg = data;
}

static inline uint32_t ioread32(uint32_t reg)
{
    return *(volatile uint32_t *)reg;
}
