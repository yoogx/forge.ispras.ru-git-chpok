import tool
import checker
import creat
import create_div
import create_TLB1_c
import create_partition_ld
import json


def create(target, source, env):
    create_partition_ld.main(env['BUILD_DIRS'],env['PARTITIONS']) #SCONSTRUCT_DIR'])
    creat.main()
    create_div.main(env['BUILD_DIRS'])
    tool.main()
    checker.main()
    create_TLB1_c.main(env['SCONSTRUCT_DIR'])

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
