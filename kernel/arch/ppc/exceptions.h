#ifndef __POK_EXCEPTIONS_H__
#define __POK_EXCEPTIONS_H__

#define SET_IVOR(vector_number, vector_label)           \
                li      r26,vector_label@l;             \
                mtspr   SPRN_IVOR##vector_number,r26;   \
                sync

/*
 * Exception vectors.
 */
#define START_EXCEPTION(label)                                               \
        .align 5;                                                            \
label:

#define STUB_EXCEPTION(label) \
    START_EXCEPTION(label) \
    b label

#endif // __POK_EXCEPTIONS_H__
