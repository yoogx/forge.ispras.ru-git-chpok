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

/* 
 * Helper for transform return codes returned by system calls to
 * ARINC return codes.
 * 
 * Macros should be used in function with parameter
 * 
 *  RETURN_CODE_TYPE* RETURN_CODE
 * 
 * Common usage pattern is:
 * 
 * MAP_ERROR_BEGIN(core_ret)
 *   MAP_ERROR(EOK, NO_ERROR) // Map "successfull" codes
 *   MAP_ERROR(..., ...) // Map other codes which means error
 *   ...
 *   MAP_ERROR_EFAULT() // If syscall may return EFAULT.
 *   MAP_ERROR_CANCELLED() // If syscall may return JET_CANCELLED for IPPC handler threads.
 *   MAP_ERROR_DEFAULT() // This should emit some error in case of unexpected return value from syscall.
 * MAP_ERROR_END()
 */

#ifndef __LIBJET_ARINC_MAP_ERROR_H__
#define __LIBJET_ARINC_MAP_ERROR_H__

#define MAP_ERROR_BEGIN(err) switch(err) {
#define MAP_ERROR_END() }

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
// TODO: Should raise an error
#define MAP_ERROR_DEFAULT() default: *RETURN_CODE = INVALID_CONFIG; break
// TODO: This should emit memory violation error.
#define MAP_ERROR_EFAULT() case EFAULT: *RETURN_CODE = INVALID_PARAM; break
// Map JET_CANCELLED return code, which is returned only for IPPC server handlers.
// TODO: this should be changed to something unique.
#define MAP_ERROR_CANCELLED() case EFAULT: *RETURN_CODE = INVALID_MODE; break

#endif /* __LIBJET_ARINC_MAP_ERROR_H__ */
