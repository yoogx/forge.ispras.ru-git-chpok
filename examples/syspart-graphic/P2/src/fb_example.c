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
#include <arinc653/time.h>
#include <arinc653/process.h>

#include <fb.h>
#define MILLISECOND 1000000

uint32_t rgba_to_argb(uint32_t rgba_color);

struct gimp_image {
  unsigned int  width;
  unsigned int  height;
  unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char pixel_data[];
};

extern const struct gimp_image gimp_image; //Image of airplane


static void draw_image(struct uwrm_scm_direct_fb *fb, int x_start, int y_start)
{
    uint32_t *addr;
    for (int y = 0; y < gimp_image.height; y++) {
        for (int x = 0; x < gimp_image.width; x++) {
            addr = (uint32_t *) fb->back_surface + (y+y_start)*fb->width + x + x_start;
            uint32_t rgba_pixel = (((uint32_t*)gimp_image.pixel_data)[y*gimp_image.width + x]);
            *addr = rgba_to_argb(rgba_pixel);
        }
    }
}

void fb_example_run(void)
{
    RETURN_CODE_TYPE ret;

    struct uwrm_scm_direct_fb fb;
    if (uwrm_scm_get_direct_fb(&fb) != UWRM_OK) {
        printf("Can't get framebuffer\n");
        STOP_SELF();
    }

    int y = fb.height - 100;
    int x = 0;

    while (y > 0) {
        draw_image(&fb, x, y);
        uwrm_scm_fb_swap(&fb);
        y--;
        x++;
        TIMED_WAIT(MILLISECOND, &ret);
    }
}
