
#include <stdio.h>

#include <x_gen.h>
#include <y_gen.h>
/* _________                 _________
   \ portA /                 \ portA /
    \     /                   \     /
     \   /                     \   /
 _____\_/_____             _____\_/_____
|             |           |             |
|     Y1      |           |     Y2      |
|   _______   |           |   _______   |
|    y = 1    |           |    y = 2    |
|_____________|           |_____________|
   \ portB /                 \ portB /
    \     /                   \     /
     \   /                     \   /
      \ /                       \ /
       |                         |
       |                         |
       |                         |
       |                         |
   ___\_/___                 ___\_/___
   \ portC /                 \ portC /
    \     /                   \     /
     \   /                     \   /
 _____\_/____              _____\_/____
|            |            |            |
|     X1     |            |     X2     |
|   _______  |            |   _______  |
|    x = 1   |            |    x = 2   |
|____________|            |____________|


portA: tick_t(tick)
portB, portC: send_flush_t(send, flush)
*/

extern Y y_1;
extern Y y_2;
void __components_init__();
void y_tick(Y *);

int glue_main()
{
    __components_init__();
    printf("\n");
    y_tick(&y_1);
    printf("\n");
    y_tick(&y_2);
}
