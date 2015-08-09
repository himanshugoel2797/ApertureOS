Virtual Memory Map
============
0x00000000 -> 0x02800000 Kernel Low Memory
0x02800000 -> 0xF0000000 User Memory
0xF0000000 -> 0xFFFFFFFF Kernel High Memory (Paging structures, ...)

Physical Memory Map
============
0x00000000 -> 0x02800000 Kernel Low Memory
0x02800000 -> 0xFFFFFFFF User Memory
0xFFFFFFFF+              Kernel Managed page heap
