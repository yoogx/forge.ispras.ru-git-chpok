#include <board/cpu_config.h>

#if defined(JET_PPC_CONFIG_E500v2)
/* E500v2 processor family */

#define JET_PPC_CONFIG_SPE_64

#elif defined(JET_PPC_CONFIG_E500MC)

/* E500mc processor family */

#else

#error "CPU family is not selected"

#endif
