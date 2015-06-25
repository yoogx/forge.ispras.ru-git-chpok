#define SAVE_FP(num) stfd %f##num,(92+8*num)(%r1)
#define LOAD_FP(num) lfd %f##num,(92+8*num)(%r1)
//352 = 0x160 = align_up(96(old) + (31+1)*8)
#define FRAME_SIZE 352
        .text
        .globl pok_context_switch
pok_context_switch:
        /* r3: *old_sp, r4: new_sp */

        /* Save registers.  */
        mflr    %r0
        stw     %r0,4(%r1)
        stwu    %r1,-FRAME_SIZE(%r1)
        mfcr    %r0
        stw     %r0,8(%r1)
        stw     %r2,12(%r1)
        stw     %r13,16(%r1)
        stw     %r14,20(%r1)
        stw     %r15,24(%r1)
        stw     %r16,28(%r1)
        stw     %r17,32(%r1)
        stw     %r18,36(%r1)
        stw     %r19,40(%r1)
        stw     %r20,44(%r1)
        stw     %r21,48(%r1)
        stw     %r22,52(%r1)
        stw     %r23,56(%r1)
        stw     %r24,60(%r1)
        stw     %r25,64(%r1)
        stw     %r26,68(%r1)
        stw     %r27,72(%r1)
        stw     %r28,76(%r1)
        stw     %r29,80(%r1)
        stw     %r30,84(%r1)
        stw     %r31,88(%r1)
        SAVE_FP (0)
        SAVE_FP (1)
        SAVE_FP (2)
        SAVE_FP (3)
        SAVE_FP (4)
        SAVE_FP (5)
        SAVE_FP (6)
        SAVE_FP (7)
        SAVE_FP (8)
        SAVE_FP (9)
        SAVE_FP (10)
        SAVE_FP (11)
        SAVE_FP (12)
        SAVE_FP (13)
        SAVE_FP (14)
        SAVE_FP (15)
        SAVE_FP (16)
        SAVE_FP (17)
        SAVE_FP (18)
        SAVE_FP (19)
        SAVE_FP (20)
        SAVE_FP (21)
        SAVE_FP (22)
        SAVE_FP (23)
        SAVE_FP (24)
        SAVE_FP (25)
        SAVE_FP (26)
        SAVE_FP (27)
        SAVE_FP (28)
        SAVE_FP (29)
        SAVE_FP (30)
        SAVE_FP (31)

        /* Switch stack. */
        stw     %r1,0(%r3)
        mr      %r1,%r4
        
        /* Restore registers.  */
        LOAD_FP (31)
        LOAD_FP (30)
        LOAD_FP (29)
        LOAD_FP (28)
        LOAD_FP (27)
        LOAD_FP (26)
        LOAD_FP (25)
        LOAD_FP (24)
        LOAD_FP (23)
        LOAD_FP (22)
        LOAD_FP (21)
        LOAD_FP (20)
        LOAD_FP (19)
        LOAD_FP (18)
        LOAD_FP (17)
        LOAD_FP (16)
        LOAD_FP (15)
        LOAD_FP (14)
        LOAD_FP (13)
        LOAD_FP (12)
        LOAD_FP (11)
        LOAD_FP (10)
        LOAD_FP (9)
        LOAD_FP (8)
        LOAD_FP (7)
        LOAD_FP (6)
        LOAD_FP (5)
        LOAD_FP (4)
        LOAD_FP (3)
        LOAD_FP (2)
        LOAD_FP (1)
        LOAD_FP (0)
        lwz     %r31,88(%r1)
        lwz     %r30,84(%r1)
        lwz     %r29,80(%r1)
        lwz     %r28,76(%r1)
        lwz     %r27,72(%r1)
        lwz     %r26,68(%r1)
        lwz     %r25,64(%r1)
        lwz     %r24,60(%r1)
        lwz     %r23,56(%r1)
        lwz     %r22,52(%r1)
        lwz     %r21,48(%r1)
        lwz     %r20,44(%r1)
        lwz     %r19,40(%r1)
        lwz     %r18,36(%r1)
        lwz     %r17,32(%r1)
        lwz     %r16,28(%r1)
        lwz     %r15,24(%r1)
        lwz     %r14,20(%r1)
        lwz     %r13,16(%r1)
        lwz     %r2,12(%r1)
        lwz     %r0,8(%r1)
        mtcr    %r0
        lwz     %r1,0(%r1)
        lwz     %r0,4(%r1)
        mtlr    %r0
        blr

        .size pok_context_switch, . - pok_context_switch

        .globl pok_arch_thread_start
pok_arch_thread_start:
        mr      %r3, %r14
        mr      %r4, %r15
        
        xor     %r0,%r0,%r0
        stw     %r0,4(%r1)
        bl      pok_thread_start

        .size pol_arch_thread_start, . - pok_arch_thread_start
