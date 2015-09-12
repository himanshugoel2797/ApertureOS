# Environment

PLATFORM=~/opt/cross/bin/i686
INCLUDES=-I. -Idrivers -Imanagers -Iprocessors -Ikmalloc

PRE_FPU_BOOT=crt0.o gdt.o idt.o cpuid.o \
				boot.o \
				drivers/serial/COM.o	\
				drivers/acpi_tables/acpi_tables.o \
				drivers/apic/apic.o drivers/apic/apic_init.o drivers/apic/io_apic/io_apic.o \
				managers/bootstrap_mem_pool/bootstrap_mem_pool.o \
				managers/interrupt/interrupt_manager.o \
				drivers/cmos/cmos.o \
				drivers/fpu/fpu.o \
				managers/msg_manager/msg_manager.o \
				utils/native.o utils/common.o utils/sprintf.o \
				managers/system_manager/system_manager.o \
				drivers/pic/pic.o \

SOURCES=graphics/graphics.o	\
				managers/filesystem/filesystem.o \
				managers/filesystem/ext2/ext2.o managers/filesystem/ext2/ext2_helpers.o \
				managers/keyboard/keyboard.o	\
				managers/phys_mem_manager/phys_mem_manager.o \
				managers/process/process_manager.o	\
				managers/threads/threads.o	\
				managers/timer/timer_manager.o \
				managers/virt_mem_manager/virt_mem_manager.o \
				drivers/hpet/hpet.o \
				drivers/pci/pci.o	drivers/pci/pci_devices.o \
				drivers/pit/pit.o \
				drivers/ps2/ps2.o drivers/ps2/ps2_keyboard.o drivers/ps2/ps2_mouse.o \
				drivers/ahci/ahci.o \
				drivers/ata_pio/ata_pio.o \
				kmalloc/kmalloc.o \

POST_INIT=processors/elf_loader/elf_loader.o \
		  processors/umalloc/umalloc.o \
		  processors/socket/socket.o \



OUTDISK=sdb

QEMU_OPTS=-enable-kvm -m 1024 -cpu host -d cpu_reset,guest_errors -drive id=disk,file=flash.img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 #-serial file:log.txt

BOOT_FS=EXT2

CURRENT_YEAR=$(shell date +"%Y")
COM_ENABLED=1
install:COM_ENABLED=0

debug:CONF=DEBUG
CONF=RELEASE










MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
GCC=clang -target i986-none-elf

CFLAGS_A= -ffreestanding -Wall -Wextra -Wno-trigraphs -D$(CONF) -DBOOT_FS=$(BOOT_FS)  -DCURRENT_YEAR=$(CURRENT_YEAR) -DCOM_ENABLED=$(COM_ENABLED) $(INCLUDES)
CFLAGS=$(CFLAGS_A) -ftree-vectorize -O0

stageA:CFLAGS=-mno-sse -O0 $(CFLAGS_A)
post_init:CFLAGS=-ftree-vectorize -O3 $(CFLAGS_A)
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

makefs:
	qemu-img create flash.img 512M
	mke2fs -L "HDD0" flash.img

debug: build build-tests

# build
build:stageA $(SOURCES) post_init
	mkdir -p build/$(CONF)
	$(PLATFORM)-elf-gcc -T linker.ld -o "build/$(CONF)/kernel.bin" -ffreestanding -O2 -nostdlib $(PRE_FPU_BOOT) $(SOURCES) $(POST_INIT) -lgcc

stageA:$(PRE_FPU_BOOT)

post_init:$(POST_INIT)

# clean
clean:
	rm -f $(SOURCES)
	rm -f $(PRE_FPU_BOOT)
	rm -f $(POST_INIT)
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
