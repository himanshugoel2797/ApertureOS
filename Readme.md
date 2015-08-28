# ApertureOS
A 32bit operating system aiming to be able to perform most general purpose tasks. Developed to learn about all the details in the operation of a modern x86 computer. The OS plans to be able to optimize usage of multiple cores by using a GPU style job scheduler for the other cores with asynchronous system calls.

# Planned Memory Map
## Virtual Memory Map

Address Range            | Use
------------------------ | ---------------------
0x00000000 -> 0x00100000 | VM86 mode code
0x00100000 -> 0x10000000 | Kernel Code Memory
0x10000000 -> 0x20000000 | MMIO
0x10000000 -> 0x40000000 | Kernel Data Memory
0x40000000 -> 0xFFFFBFFF | User Memory
0xFFFFBFFF -> 0xFFFFFFFF | User Stack

## Physical Memory Map

Address Range            | Use
------------------------ | ------------------------
0x00000000 -> 0x00100000 | Kernel Managed page heap
0x00100000 -> 0x40000000 | Kernel Low Memory
0x40000000 -> 0xEFFFFFFF | User Memory
0xF0000000 -> 0xFFFFFFFF | MMIO
0xFFFFFFFF+              | Kernel Managed page heap

# TODO
-PCI/PCIe support
-Mouse/Keyboard input
-2D hardware acceleration
-Audio
-Networking
-Elf loading
-Hard disk access, ext2 support
-Multicore initialization
-User mode thread scheduler
-ACPI support
-USB support
-Fix UEFI boot issues
-Window Manager
-User mode desktop environment
