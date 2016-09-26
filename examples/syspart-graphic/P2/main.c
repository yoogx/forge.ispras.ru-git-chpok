/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

#include <pci.h>
#include <fb.h>
//#include <net/network.h>
//#include <depl.h>
//#include <port_info.h>

#define SECOND 1000000000LL

uint32_t rgba_to_argb(uint32_t rgba_color);

struct gimp_image {
  unsigned int  width;
  unsigned int  height;
  unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char pixel_data[];
};

extern const struct gimp_image gimp_image;

static void draw_image(struct uwrm_scm_direct_fb *fb, int x_start, int y_start)
{
    uint32_t *addr;
    for (int y = 0; y < gimp_image.height; y++) {
        for (int x = 0; x < gimp_image.width; x++) {
            addr = (uint32_t *) fb->back_surface + (y+y_start)*fb->width + x + x_start;
            uint32_t rgba_color = (((uint32_t*)gimp_image.pixel_data)[y*gimp_image.width + x]);
            *addr = rgba_to_argb(rgba_color);
        }
    }

}

static void first_process(void)
{
    RETURN_CODE_TYPE ret;

    struct uwrm_scm_direct_fb fb;
    uwrm_scm_get_direct_fb(&fb);

    int y = fb.height - 100;
    int x = 0;

    while (y > 0) {
        draw_image(&fb, x, y);
        uwrm_scm_fb_swap(&fb);
        y--;
        x++;
        TIMED_WAIT(SECOND/50, &ret);
    }
    STOP_SELF();
}

void vga_init();

static int real_main(void)
{
    RETURN_CODE_TYPE ret;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

    // create process 1
    process_attrs.ENTRY_POINT = first_process;
    strncpy(process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 1 created\n");
    }

    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 1 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }

    printf("System partition is starting\n");
    pci_init();
    vga_init();
    //p3041_test();


    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    printf("going to NORMAL mode...\n");
    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("couldn't transit to normal operating mode: %d\n", (int) ret);
    }

    STOP_SELF();
    return 0;
}

void main(void) {
    real_main();
    STOP_SELF();
}
