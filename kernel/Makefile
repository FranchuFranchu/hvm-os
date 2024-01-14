ASM = nasm
ASMFLAGS = -f win64

LINKER = clang
LLD-FLAGS = --target=x86_64-unknown-windows -nostdlib -Wl,-entry:_start -Wl,-subsystem:efi_application -fuse-ld=lld

SRC_DIR = src
OBJ_DIR = obj
EFI_DIR = efi
INC_DIR = inc

SRC = $(wildcard $(SRC_DIR)/*.asm)

OBJ = $(SRC:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.obj)

EFI = $(EFI_DIR)/main.efi
BIOS = $(wildcard $(INC_DIR)/*.bios)
BOOT_DRIVE = main.img

.PHONY: clean qemu

efi/main.efi efi/main.debug: $(wildcard src/**/*) $(wildcard src/*)
	fasm src/main.asm $(EFI) -s efi/main.debug

src/kernel.bin: src/kernel.la ../compiler/target/debug/inet-la 
	../compiler/target/debug/inet-la src/kernel.la src/kernel.bin

clean:
	rm -f $(OBJ) $(BOOT_DRIVE) $(EFI)

qemu: main.img
	qemu-system-x86_64 -bios $(BIOS) -drive file=$(BOOT_DRIVE),format=raw -nographic -net none

main.img: efi/main.efi efi/main.debug
ifeq (, $(shell which mkfs.vfat))
	$(error "Can't find mkfs.vfat, consider doing sudo apt install dosfstools")
endif
ifeq (, $(shell which mcopy))
	$(error "Can't find mcopy, consider doing sudo apt install mtools")
endif
	dd if=/dev/zero of=$(BOOT_DRIVE) bs=1M count=1
	mkfs.vfat $(BOOT_DRIVE)
	mmd -i $(BOOT_DRIVE) ::EFI
	mmd -i $(BOOT_DRIVE) ::EFI/BOOT
	mcopy -i $(BOOT_DRIVE) $(EFI) ::EFI/BOOT/BOOTX64.EFI
