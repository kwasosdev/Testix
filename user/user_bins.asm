; user/user_bins.asm

[bits 64]
section .rodata

global prog_a_bin
global prog_a_end
prog_a_bin:
    incbin "build/user/prog_a.bin"
prog_a_end:

global prog_b_bin
global prog_b_end
prog_b_bin:
    incbin "build/user/prog_b.bin"
prog_b_end:

global shell_bin
global shell_end
shell_bin:
    incbin "build/user/shell.bin"
shell_end:

