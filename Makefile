# Environment

INCLUDES=-I.

SOURCES=utils/native.o utils/common.o \
				Graphics/graphics.o	\
				memorymanager/bootstrap_mem_manager.o memorymanager/memorymanager.o memorymanager/paging.o \
				drivers/COM.o	\
				drivers/ps2/ps2.o drivers/ps2/ps2_keyboard.o \
				boot.o crt0.o gdt.o idt.o pic.o pit.o fpu.o cpuid.o interruptmanager.o cmos.o \



PLATFORM=/opt/cross/bin/i686

SDA=sdb

QEMU_OPTS=-m 1024 -cpu SandyBridge,+xsave,+osxsave -soundhw all -d guest_errors,int

CURRENT_YEAR=$(shell date +"%Y")
COM_ENABLED=1












MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
GCC=clang -target i986-none-elf
CFLAGS=-ffreestanding -O0 -Wall -Wextra -DDEBUG  -DCURRENT_YEAR=$(CURRENT_YEAR) -DCOM_ENABLED=$(COM_ENABLED) $(INCLUDES)
ASM=$(PLATFORM)-elf-gcc -DDEBUG -ffreestanding -march=i686
TEST_CMD=qemu-kvm $(QEMU_OPTS)
CONF=Debug

.c.o:
	$(GCC) $(CFLAGS) -S $? -o $(?:.c=.s)
	$(ASM) $(?:.c=.s) -c -o $(?:.c=.o)
	rm -f $(?:.c=.s)

.s.o:
	$(ASM) $? -c -o $(?:.s=.o)

.S.o:
	$(ASM) $? -c -o $(?:.S=.o)

run: all

# build
build:$(SOURCES)
	mkdir -p build/$(CONF)
	$(PLATFORM)-elf-gcc -T linker.ld -o "build/$(CONF)/kernel.bin" -ffreestanding -O2 -nostdlib $(SOURCES) -lgcc

# clean
clean:
	rm -f $(SOURCES)
	rm -rf build/*
	rm -rf ISO/*


# all
all:build-tests

# build tests
build-tests:build
	mkdir -p ISO
	mkdir -p ISO/isodir
	mkdir -p ISO/isodir/boot
	cp "build/$(CONF)/kernel.bin" ISO/isodir/boot/kernel.bin
	mkdir -p ISO/isodir/boot/grub
	cp grub.cfg ISO/isodir/boot/grub/grub.cfg
	grub2-mkrescue -o ISO/os.iso ISO/isodir

# run tests
test: build-tests
# Add your pre 'test' code here...
	$(TEST_CMD) -cdrom "ISO/os.iso"

install:build-tests
	sudo dd if="ISO/os.iso" of=/dev/$(SDA) && sync
