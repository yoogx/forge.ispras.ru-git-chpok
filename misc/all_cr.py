import tool
import checker
import creat
import create_div
import create_TLB1_c
import create_partition_ld
import json


def create(target, source, env):
    create_partition_ld.main(env)#['SCONSTRUCT_DIR'],env['PARTITIONS'],env['POK_PATH']) #SCONSTRUCT_DIR'])
    creat.main()
    create_div.main(env['SCONSTRUCT_DIR'],env['PARTITIONS'])
    tool.main()
    checker.main()
    create_TLB1_c.main(env['BUILD_DIR'])

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
