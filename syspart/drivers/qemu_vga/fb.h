#ifndef __QEMU_VGA_FB_H__
#define __QEMU_VGA_FB_H__

typedef int uwrm_handle;

enum FB_FORMAT {
    UWRM_FORMAT_RGB888,
    UWRM_FORMAT_RGBA8888,
    UWRM_FORMAT_ARGB8888,
    UWRM_FORMAT_RGB565,
};

enum UWRM_RET {
    UWRM_OK,
    UWRM_ERROR
};

struct uwrm_scm_direct_fb {
    uwrm_handle hfb; // fb descriptor
    void* back_surface;
    void* front_surface;
    int width;
    int height;
    int format; //enum FB_FORMAT
};

int uwrm_scm_get_direct_fb(struct uwrm_scm_direct_fb *fb);

int uwrm_scm_fb_swap(struct uwrm_scm_direct_fb *fb);

#endif
