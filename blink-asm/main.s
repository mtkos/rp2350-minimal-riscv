.set RESETS_BASE, 0x40020000
.set RESETS_RESET_OFFSET, 0x0
.set REG_ALIAS_CLR_BITS, 0x3000
.set RESETS_RESET_IO_BANK0_BITS, 1<<6
.set RESETS_RESET_PADS_BANK0_BITS, 1<<9
.set RESETS_RESET_DONE_OFFSET, 0x8
.set RESETS_RESET_DONE_IO_BANK0_BITS, 1<<6
.set RESETS_RESET_DONE_PADS_BANK0_BITS, 1<<9
.set IO_BANK0_BASE, 0x40028000
.set IO_BANK0_GPIO25_CTRL_OFFSET, 0x4 + 25*0x8
.set GPIO_FUNC_SIO, 5
.set PADS_BANK0_BASE, 0x40038000
.set PADS_BANK0_GPIO25_OFFSET, 0x4 + 25*0x4
.set REG_ALIAS_CLR_BITS, 0x3000
.set PADS_BANK0_GPIO0_ISO_BITS, 1<<8
.set SIO_BASE, 0xd0000000
.set SIO_GPIO_OE_SET_OFFSET, 0x38
.set SIO_GPIO_OUT_XOR_OFFSET, 0x28
.set PIN, 25

.option norvc
  j      prog
.option rvc

.word 0xffffded3
.word 0x11010142
.word 0x000001ff
.word 0x00000000
.word 0xab123579

prog:

li t0, RESETS_BASE + RESETS_RESET_OFFSET + REG_ALIAS_CLR_BITS
li t1, RESETS_RESET_IO_BANK0_BITS | RESETS_RESET_PADS_BANK0_BITS
sw t1, (t0)

li t2, RESETS_RESET_DONE_IO_BANK0_BITS | RESETS_RESET_DONE_PADS_BANK0_BITS
li t0, RESETS_BASE + RESETS_RESET_DONE_OFFSET
0:
lw t1, (t0)
not t1, t1
and t1, t1, t2
bnez t1, 0b

li t0, IO_BANK0_BASE + IO_BANK0_GPIO25_CTRL_OFFSET
li t1, GPIO_FUNC_SIO
sw t1, (t0)

li t0, PADS_BANK0_BASE + PADS_BANK0_GPIO25_OFFSET + REG_ALIAS_CLR_BITS
li t1, PADS_BANK0_GPIO0_ISO_BITS
sw t1, (t0)

li t0, SIO_BASE + SIO_GPIO_OE_SET_OFFSET
li t1, 1<<PIN
sw t1, (t0)

li t0, SIO_BASE + SIO_GPIO_OUT_XOR_OFFSET
0:
li t1, 1<<PIN
sw t1, (t0)

li t2, 1000000
1:
add t2, t2, -1
bnez t2, 1b

j 0b
