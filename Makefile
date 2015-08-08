# Environment

INCLUDES=-I.

SOURCES=utils/native.o utils/common.o \
				Graphics/graphics.o	\
				memorymanager/bootstrap_mem_manager.o memorymanager/memorymanager.o memorymanager/paging.o \
				boot.o crt0.o gdt.o idt.o pic.o pit.o fpu.o cpuid.o interruptmanager.o cmos.o \



PLATFORM=/opt/cross/bin/i686

SDA=sdb

QEMU_OPTS=-m 1024 -cpu SandyBridge,+xsave,+osxsave -soundhw all -d guest_errors,int

CURRENT_YEAR=$(shell date +"%Y")













MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
GCC=clang -target i986-none-elf
CFLAGS=-ffreestanding -O0 -Wall -Wextra -DDEBUG  -DCURRENT_YEAR=$(CURRENT_YEAR) $(INCLUDES)
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

.clean-pre:
# Add your pre 'clean' code here...

.clean-post:
# Add your post 'clean' code here...


# clobber
clobber:

.clobber-pre:
# Add your pre 'clobber' code here...

.clobber-post: .clobber-impl
# Add your post 'clobber' code here...


# all
all:test
.all-pre:
# Add your pre 'all' code here...

.all-post: .all-impl
# Add your post 'all' code here...


# build tests
build-tests:build
	mkdir -p ISO
	mkdir -p ISO/isodir
	mkdir -p ISO/isodir/boot
	cp "build/$(CONF)/kernel.bin" ISO/isodir/boot/kernel.bin
	mkdir -p ISO/isodir/boot/grub
	cp grub.cfg ISO/isodir/boot/grub/grub.cfg
	grub2-mkrescue -o ISO/os.iso ISO/isodir

.build-tests-pre:
# Add your pre 'build-tests' code here...

.build-tests-post:
# Add your post 'build-tests' code here...


# run tests
test:.test-pre

.test-pre: build-tests
# Add your pre 'test' code here...
	$(TEST_CMD) -cdrom "ISO/os.iso"
.test-post:
# Add your post 'test' code here...

install:build-tests
	sudo dd if="ISO/os.iso" of=/dev/$(SDA) && sync

# help
help:

.help-pre:
# Add your pre 'help' code here...

.help-post:
# Add your post 'help' code here...
