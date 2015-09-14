# ApertureOS
A 32bit operating system aiming to be able to perform most general purpose tasks. Developed to learn about all the details in the operation of a modern x86 computer. The OS plans to be able to optimize usage of multiple cores by using a GPU style job scheduler for the other cores with asynchronous system calls.

# Design
The core design of ApertureOS:

## Message Pump
Similar to the windows message pump, each application is required to have a message pump in order to be able to communicate with the kernel. The application consumes and provides services by connecting to and creating sockets. The application recieves all its notifications from this message pump. Message pumps are thread specific, encouraging design of applications to make effective use of parallelism with minimal synchronization.

## Sockets
The kernel treats everything as a 'socket'. Although similar to the UNIX abstraction of treating everything as a file, the socket doesn't attempt to generalize into providing any functionality at all. A socket can specify the number of simultaneous connections it can take. Each connection requests the features it desires access to. All actions on sockets are asynchronous and completion is reported to the application through the application message pump. Connections are thread specific.

Documentation at http://himanshugoel2797.github.io/ApertureOS/docs/html/

# Planned Memory Map
## Virtual Memory Map

Address Range            | Use
------------------------ | ---------------------
0x00000000 -> 0x00100000 | VM86 mode code
0x00100000 -> 0x10000000 | Kernel Code Memory
0x10000000 -> 0x20000000 | MMIO
0x10000000 -> 0x40000000 | Kernel Data Memory
0x40000000 -> 0x40004000 | Kernel Bootstrap code
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
-Mouse/Keyboard input (Almost done)
-2D hardware acceleration
-Audio
-Networking
-Elf loading
-Hard disk access, ext2 support (Halfway through)
-Multicore initialization
-User mode thread scheduler
-ACPI support
-USB support
-Fix UEFI boot issues
-Window Manager
-User mode desktop environment
