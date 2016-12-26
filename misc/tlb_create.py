#******************************************************************
#
# Institute for System Programming of the Russian Academy of Sciences
# Copyright (C) 2016 ISPRAS
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, Version 3.
#
# This program is distributed in the hope # that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License version 3 for more details.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

import tool
import checker
import create_target
import divided
import create_TLB_c
import create_partition_ld
import json
import merger

def create(target, source, env):
    create_partition_ld.main(env)#do separately
    create_target.main()#do separately
    divided.main(env['PARTITION_BUILD_DIRS'])
    merger.main()
    tool.main()
    checker.main()
    create_TLB_c.main(env['BUILD_DIR'])

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
