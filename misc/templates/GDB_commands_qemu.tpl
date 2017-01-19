target remote :1234

python
import re
class load_partition (gdb.Command):
    """
    Load partition elf to debug partition. Overwrite previously loaded
    partitions.
    """
    def __init__ (self):
        super (load_partition, self).__init__ ("load_partition",
                gdb.COMMAND_USER,
                gdb.COMMAND_FILES
                )
        self.map = {{partitions_elf_map}}

    def get_text_addr (self, elf):
        """
        Return addr (as string) of text section in elf
        NOTE: This function has a side effect. It loads new symbol file by
        "unload" the old one. So you should reload elf after using it!
        """
        gdb.execute('file' + elf)
        out = gdb.execute('info files', to_string=True)
        return re.search("(0x\d+).*.text", out).group(1)

    def invoke (self, arg, from_tty):
        if arg not in self.map:
            print('Wrong partition name. Names: ' + ', '.join(self.map.keys()))
            return 0
        part_elf = self.map[arg]
        #save partition load addr to use later
        part_load_addr = self.get_text_addr(part_elf)

        gdb.execute('symbol-file')
        gdb.execute('file {{pok_target_file}}')
        gdb.execute('add-symbol-file ' + part_elf + ' ' + part_load_addr)
        return 0

load_partition() # register command
end
