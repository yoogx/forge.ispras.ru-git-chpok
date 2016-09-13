/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (P2/components-glue/config.yaml).
 */
    #include <X_gen.h>
        void __X_init__(X*);
        X x_1 = {
            .state = {
                .x = 1,
            }
        };
        void __X_init__(X*);
        X x_2 = {
            .state = {
                .x = 2,
            }
        };

    #include <Y_gen.h>
        void __Y_init__(Y*);
        Y y_1 = {
            .state = {
                .y = 1,
            }
        };
        void __Y_init__(Y*);
        Y y_2 = {
            .state = {
                .y = 2,
            }
        };



//#include <Y/y_gen.h>
void __components_init__()
{
            __X_init__(&x_1);
            __X_init__(&x_2);

            __Y_init__(&y_1);
            __Y_init__(&y_2);


        y_1.out.portB.ops = &x_1.in.portC.ops;
        y_1.out.portB.owner = &x_1;
        y_2.out.portB.ops = &x_2.in.portC.ops;
        y_2.out.portB.owner = &x_2;

}
