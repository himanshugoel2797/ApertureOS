# Environment

INCLUDES=-I. -Idrivers -Imanagers

SOURCES=utils/native.o utils/common.o utils/sprintf.o \
				graphics/graphics.o	\
				managers/system_manager/system_manager.o \
				managers/bootstrap_mem_pool/bootstrap_mem_pool.o \
				managers/keyboard/keyboard.o	\
				managers/msg_manager/msg_manager.o \
				managers/interrupt/interrupt_manager.o \
				managers/phys_mem_manager/phys_mem_manager.o \
				managers/process/process_manager.o	\
				managers/threads/threads.o	\
				managers/timer/timer_manager.o \
				managers/virt_mem_manager/virt_mem_manager.o \
				drivers/acpi_tables/acpi_tables.o \
				drivers/apic/apic.o drivers/apic/apic_init.o drivers/apic/io_apic/io_apic.o \
				drivers/cmos/cmos.o \
				drivers/fpu/fpu.o \
				drivers/hpet/hpet.o \
				drivers/pci/pci.o	drivers/pci/pci_devices.o \
				drivers/pic/pic.o \
				drivers/pit/pit.o \
				drivers/ps2/ps2.o drivers/ps2/ps2_keyboard.o \
				drivers/serial/COM.o	\
				kmalloc.o \
				crt0.o gdt.o idt.o cpuid.o \
				boot.o \



PLATFORM=~/Documents/i686-elf/bin/i686

OUTDISK=sdb

QEMU_OPTS=-m 1024 -cpu SandyBridge -d guest_errors,int #-serial file:log.txt

CURRENT_YEAR=$(shell date +"%Y")
COM_ENABLED=1
install:COM_ENABLED=0

debug:CONF=DEBUG
CONF=RELEASE










MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
GCC=clang -target i986-none-elf
CFLAGS= -ffreestanding -O1 -Wall -Wextra -Wno-trigraphs -D$(CONF)  -DCURRENT_YEAR=$(CURRENT_YEAR) -DCOM_ENABLED=$(COM_ENABLED) $(INCLUDES)
ASM=$(PLATFORM)-elf-gcc -DDEBUG -ffreestanding -march=i686
TEST_CMD=qemu-system-x86_64 $(QEMU_OPTS)

ANALYZE=clang-check -analyze

.c.o:
	$(GCC) $(CFLAGS) -S $? -o $(?:.c=.s)
	$(ASM) $(?:.c=.s) -c -o $(?:.c=.o)
	rm -f $(?:.c=.s)

.s.o:
	$(ASM) $? -c -o $(?:.s=.o)

.S.o:
	$(ASM) $? -c -o $(?:.S=.o)

.PHONY:debug

run: all

debug: build build-tests

# build
build:$(SOURCES)
	mkdir -p build/$(CONF)
	$(PLATFORM)-elf-gcc -T linker.ld -o "build/$(CONF)/kernel.bin" -ffreestanding -O2 -nostdlib $(SOURCES) -lgcc

# clean
clean:
	rm -f $(SOURCES)
	rm -f *.plist
	rm -rf build/*
	rm -rf ISO/*

analyze:
	$(ANALYZE) $(SOURCES:.o=.c) -- $(CFLAGS)

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

install:clean build-tests
	sudo dd if="ISO/os.iso" of=/dev/$(OUTDISK) && sync
