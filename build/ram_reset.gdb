# ram_reset —— RAM 版软件复位:gdb_ram 在 load 后自动执行
define ram_reset
  set $msp = &_estack
  set $pc = Reset_Handler
  set {unsigned int}0xE000ED08 = &_sisr
end
document ram_reset
设 MSP=_estack、PC=Reset_Handler、VTOR=_sisr(RAM 向量表)。
end
