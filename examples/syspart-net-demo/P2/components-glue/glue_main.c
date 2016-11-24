
#include <stdio.h>

void __components_activity__();
void __components_init__();

void glue_activity()
{
    __components_activity__();
}

void glue_main()
{
    __components_init__();
}
