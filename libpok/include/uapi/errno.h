/*
 * COPIED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify original one (kernel/include/uapi/errno.h).
 */
/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#ifndef __JET_UAPI_ERRNO_H__
#define __JET_UAPI_ERRNO_H__

typedef enum
{
    EOK = 0,
    EINVAL,
    EDOM,
    EILSEQ,
    ERANGE,
    EFAULT, //page fault
    EEXIST, //instance (of any type) already exists
    ETIMEDOUT, // Timeout
    EAGAIN, // resource currently is not available.

    /* Jet specific errors */
    JET_INVALID_CONFIG = 256,
    JET_INVALID_MODE, //wrong mode of current thread (partition, lock_preemption, etc)
    JET_INVALID_MODE_TARGET, //wrong mode of target thread (port, etc)
    JET_NOACTION, // object state remains unchanged.
    JET_CANCELLED, // stopped thread inside msection, cancelled IPPC call

    /* IPPC specific errors */
    JET_IPPC_RESETED,
    JET_IPPC_FAILED

} jet_ret_t;

#endif /* __JET_UAPI_ERRNO_H__ */
