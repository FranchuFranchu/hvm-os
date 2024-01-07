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

EFI = $(EFI_DIR)/uefi.efi
BIOS = $(wildcard $(INC_DIR)/*.bios)
BOOT_DRIVE = uefi.img

.PHONY: clean qemu

efi/uefi.efi efi/uefi.debug: $(wildcard src/**/*) $(wildcard src/*)
	fasm src/uefi.asm $(EFI) -s efi/uefi.debug

clean:
	rm -f $(OBJ) $(BOOT_DRIVE) $(EFI)

qemu: uefi.img
	qemu-system-x86_64 -bios $(BIOS) -drive file=$(BOOT_DRIVE),format=raw -nographic -net none

uefi.img: efi/uefi.efi efi/uefi.debug
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
