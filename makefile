# --- Настройки кросс-компилятора ---
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Linux)
    DISTRO := $(shell grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"')
    ifeq ($(DISTRO),arch)
        CROSS ?=
				OVMF_CODE  ?= /usr/share/edk2/x64/OVMF_CODE.4m.fd
        OVMF_VARS  ?= /usr/share/edk2/x64/OVMF_VARS.4m.fd
    else
        CROSS ?= x86_64-elf-
				OVMF_CODE  ?= /usr/share/OVMF/OVMF_CODE_4M.fd
        OVMF_VARS  ?= /usr/share/OVMF/OVMF_VARS_4M.fd
    endif
endif

ifeq ($(UNAME_S),Darwin)
	  BREW_PREFIX := $(shell brew --prefix)
    CROSS       ?= x86_64-elf-
    LIMINE_DIR  ?= $(BREW_PREFIX)/share/limine
    LIMINE_BIN  ?= $(BREW_PREFIX)/bin/limine
    OVMF_CODE   ?= $(BREW_PREFIX)/share/qemu/edk2-x86_64-code.fd
    OVMF_VARS   ?= $(BREW_PREFIX)/share/qemu/edk2-i386-vars.fd
endif

NASM := nasm

# --- Директории и файлы ---
BUILD      := build
KERNEL     := $(BUILD)/kernel.elf
ISO_ROOT   := $(BUILD)/iso_root
LIMINE_DIR := /usr/share/limine

# --- Флаги компиляции ---
CFLAGS   := -std=c11 -ffreestanding -fno-stack-protector -fno-pic \
	-mno-red-zone -mcmodel=kernel -Wall -Wextra -O2 -Iinclude

USER_CXXFLAGS := -std=c++20 -ffreestanding -fno-exceptions -fno-rtti \
	-fno-stack-protector -fno-pic -fno-pie -mcmodel=small \
	-mno-red-zone -O2 -Iuser

CXXFLAGS := -std=c++20 -ffreestanding -fno-exceptions -fno-rtti \
	-fno-threadsafe-statics -fno-stack-protector -fno-pic \
	-mno-red-zone -mcmodel=kernel -mno-sse -mno-sse2 -mno-mmx -mno-avx \
	-Wall -Wextra -O2 -Iinclude

LDFLAGS  := -T linker.ld -nostdlib -static -no-pie -Wl,--build-id=none

# --- Пользовательские программы (Ring 3) ---
USER_BINS := $(BUILD)/user/prog_a.bin $(BUILD)/user/prog_b.bin $(BUILD)/user/shell.bin

# --- Исходные файлы ядра ---
C_SRCS   := 
CXX_SRCS := $(shell find kernel arch drivers kernlang -name '*.cpp')

ASM_SRCS := arch/x86_64/entry.asm \
	arch/x86_64/interrupts/interrupts.asm \
	user/user_bins.asm

C_OBJS   := $(patsubst %.c,  $(BUILD)/%.o, $(C_SRCS))
CXX_OBJS := $(patsubst %.cpp,$(BUILD)/%.o, $(CXX_SRCS))
ASM_OBJS := $(patsubst %.asm,$(BUILD)/%.asm.o, $(ASM_SRCS))

OBJS := $(ASM_OBJS) $(C_OBJS) $(CXX_OBJS)

TARGET := testix.iso

# --- Основные цели ---
all: $(TARGET)

# --- Сборка user-программ в плоские бинарники ---
$(BUILD)/user/%.bin: user/%.cpp user/user.ld
	@mkdir -p $(dir $@)
	$(CXX) $(USER_CXXFLAGS) -c $< -o $(BUILD)/user/$*.o
	$(LD) -T user/user.ld --oformat=binary -nostdlib $(BUILD)/user/$*.o -o $@

$(BUILD)/user/user_bins.asm.o: $(USER_BINS)

# --- Сборка объектных файлов ядра ---
$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@

# --- Линковка ядра ---
$(KERNEL): $(OBJS) linker.ld
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

# --- Создание ISO-образа через Limine ---
$(TARGET): $(KERNEL) limine.conf
	@rm -rf $(ISO_ROOT)
	@mkdir -p $(ISO_ROOT)/boot
	@mkdir -p $(ISO_ROOT)/EFI/BOOT
	
	cp $(KERNEL) $(ISO_ROOT)/boot/kernel.elf
	cp limine.conf $(ISO_ROOT)/limine.conf
	cp limine.conf $(ISO_ROOT)/boot/limine.conf
	
	cp $(LIMINE_DIR)/limine-bios.sys $(ISO_ROOT)/boot/
	cp $(LIMINE_DIR)/limine-bios-cd.bin $(ISO_ROOT)/boot/
	cp $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_ROOT)/boot/
	cp $(LIMINE_DIR)/BOOTX64.EFI $(ISO_ROOT)/EFI/BOOT/

	xorriso -as mkisofs \
		-o $@ \
		-b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		$(ISO_ROOT)
		
	limine bios-install $@

# --- Запуск в QEMU ---
run-efi: $(TARGET)
	cp $(OVMF_VARS) $(BUILD)/OVMF_VARS.fd
	cp $(OVMF_CODE) $(BUILD)/OVMF_CODE.fd
	qemu-system-x86_64 \
		-machine q35 \
		-global ICH9-LPC.disable_s3=1 \
		-drive if=pflash,format=raw,readonly=on,file=$(BUILD)/OVMF_CODE.fd \
		-drive if=pflash,format=raw,file=$(BUILD)/OVMF_VARS.fd \
		-boot d -cdrom $(TARGET) \
		-serial stdio -net none

run-bios: $(TARGET)
	qemu-system-x86_64 -boot d -cdrom $(TARGET) -serial stdio

clean:
	rm -rf $(BUILD) $(TARGET)

context:
	@mkdir -p $(BUILD)
	@python3 -c 'import pathlib; \
	exts={".hpp",".h",".cpp",".c",".asm",".ld",".conf"}; \
	skip={"build",".git",".vscode","font8x8.cpp","limine.hpp","limine.h"}; \
	files=[p for p in sorted(pathlib.Path(".").rglob("*")) if p.is_file() and p.suffix in exts and not any(s in p.parts for s in skip)]; \
	out="\n\n".join(f"=== FILE: {p} ===\n" + p.read_text(errors="ignore") for p in files); \
	pathlib.Path("project_context.txt").write_text(out); \
	print(f"Packed {len(files)} files (~{len(out)//4} tokens) -> project_context.txt")'

.PHONY: all clean run run-iso-bios run-efi context
