.set MIE, 1<<3
.set MEIE, 1<<11
.set meiea, 0xbe0
.set IO_IRQ_BANK0_IRQn, 21
.set mask, ((1<<(IO_IRQ_BANK0_IRQn%16 + 16)) | (IO_IRQ_BANK0_IRQn/16))

.section .init

.option norvc
  la sp, _estack

  li t0, MIE
  csrw mstatus, t0

  li t0, MEIE
  csrw mie, t0

  li t0, mask
  csrw meiea, t0

  la t0, IO_IRQ_BANK0_Handler
  csrw mtvec, t0

  j      prog
.option rvc

.word 0xffffded3
.word 0x11010142
.word 0x000001ff
.word 0x00000000
.word 0xab123579
