# --- Определение ОС, дистрибутива и архитектуры ---
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Linux)
    DISTRO := $(shell . /etc/os-release 2>/dev/null && echo $$ID)
    DISTRO_LIKE := $(shell . /etc/os-release 2>/dev/null && echo $$ID_LIKE)

    ifeq ($(UNAME_M),x86_64)
        CROSS ?=
    else
        CROSS ?= x86_64-linux-gnu-
    endif

    ifeq ($(DISTRO),arch)
        OVMF_CODE ?= /usr/share/edk2/x64/OVMF_CODE.4m.fd
        OVMF_VARS ?= /usr/share/edk2/x64/OVMF_VARS.4m.fd
    else ifneq ($(filter debian ubuntu linuxmint pop,$(DISTRO) $(DISTRO_LIKE)),)
        OVMF_CODE ?= /usr/share/OVMF/OVMF_CODE_4M.fd
        OVMF_VARS ?= /usr/share/OVMF/OVMF_VARS_4M.fd
    else ifneq ($(filter fedora rhel centos,$(DISTRO) $(DISTRO_LIKE)),)
        OVMF_CODE ?= /usr/share/edk2/ovmf/OVMF_CODE.fd
        OVMF_VARS ?= /usr/share/edk2/ovmf/OVMF_VARS.fd
    else ifneq ($(filter opensuse-tumbleweed opensuse-leap suse,$(DISTRO) $(DISTRO_LIKE)),)
        OVMF_CODE ?= /usr/share/qemu/ovmf-x86_64-code.bin
        OVMF_VARS ?= /usr/share/qemu/ovmf-x86_64-vars.bin
    else
        OVMF_CODE ?= /usr/share/OVMF/OVMF_CODE.fd
        OVMF_VARS ?= /usr/share/OVMF/OVMF_VARS.fd
    endif

    ifneq ($(wildcard /usr/share/limine/limine-bios.sys),)
        LIMINE_DIR ?= /usr/share/limine
        LIMINE_BIN ?= limine
    else
        LIMINE_DIR ?= $(BUILD)/limine
        LIMINE_BIN ?= $(BUILD)/limine/limine
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

# --- Инструменты ---
CC      := $(CROSS)gcc
CXX     := $(CROSS)g++
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy
NASM    := nasm

# --- Директории и файлы ---
BUILD    := build
KERNEL   := $(BUILD)/kernel.elf
ISO_ROOT := $(BUILD)/iso_root
TARGET   := testix.iso

# --- Флаги компиляции ---
CFLAGS := -std=c11 -ffreestanding -fno-stack-protector -fno-pic \
	-mno-red-zone -mcmodel=kernel -Wall -Wextra -O2 -Ikernel/include

USER_CXXFLAGS := -std=c++20 -ffreestanding -fno-exceptions -fno-rtti \
	-fno-stack-protector -fno-pic -fno-pie -mcmodel=small \
	-mno-red-zone -mno-sse -mno-sse2 -mno-mmx -mno-avx -O2 -Ilibsys/include

CXXFLAGS := -std=c++20 -ffreestanding -fno-exceptions -fno-rtti \
	-fno-threadsafe-statics -fno-stack-protector -fno-pic \
	-mno-red-zone -mcmodel=kernel -mno-sse -mno-sse2 -mno-mmx -mno-avx \
	-Wall -Wextra -O2 -Ikernel/include

LDFLAGS := -T kernel/linker.ld -nostdlib -static -no-pie -Wl,--build-id=none

# --- Пользовательские программы (Ring 3): init, shell и все утилиты из binutils/ ---
BINUTIL_SRCS := $(wildcard binutils/*.cpp)
BINUTIL_BINS := $(patsubst binutils/%.cpp, initrd/bin/%, $(BINUTIL_SRCS))
USER_BINS    := initrd/bin/init initrd/bin/shell $(BINUTIL_BINS)
INITRD_FILES := $(shell find initrd -type f 2>/dev/null)

# --- Исходные файлы ядра ---
C_SRCS   := 
CXX_SRCS := $(shell find kernel -name '*.cpp')

ASM_SRCS := kernel/arch/x86_64/entry.asm \
	kernel/arch/x86_64/interrupts/interrupts.asm

C_OBJS   := $(patsubst %.c,  $(BUILD)/%.o, $(C_SRCS))
CXX_OBJS := $(patsubst %.cpp,$(BUILD)/%.o, $(CXX_SRCS))
ASM_OBJS := $(patsubst %.asm,$(BUILD)/%.asm.o, $(ASM_SRCS))

OBJS := $(ASM_OBJS) $(C_OBJS) $(CXX_OBJS)

# --- Основные цели ---
all: $(TARGET)

$(LIMINE_DIR)/limine-bios.sys:
	@mkdir -p $(BUILD)
	@printf "  %-8s %s\n" "GIT" "limine"
	@git clone https://github.com --branch=v8.x-binary --depth=1 $(BUILD)/limine
	@$(MAKE) -C $(BUILD)/limine

# --- Сборка init, shell и утилит binutils в плоские бинарники ---
initrd/bin/init: $(wildcard init/*.cpp) libsys/user.ld
	@mkdir -p $(dir $@) $(BUILD)/user/init
	@printf "  %-8s %s\n" "CXX" "init/main.cpp"
	@$(CXX) $(USER_CXXFLAGS) -c init/main.cpp -o $(BUILD)/user/init/main.o
	@printf "  %-8s %s\n" "BIN" "$@"
	@$(LD) -T libsys/user.ld --oformat=binary -nostdlib $(BUILD)/user/init/main.o -o $@

initrd/bin/shell: $(wildcard shell/*.cpp) libsys/user.ld
	@mkdir -p $(dir $@) $(BUILD)/user/shell
	@printf "  %-8s %s\n" "CXX" "shell/main.cpp"
	@$(CXX) $(USER_CXXFLAGS) -c shell/main.cpp -o $(BUILD)/user/shell/main.o
	@printf "  %-8s %s\n" "BIN" "$@"
	@$(LD) -T libsys/user.ld --oformat=binary -nostdlib $(BUILD)/user/shell/main.o -o $@

initrd/bin/%: binutils/%.cpp libsys/user.ld
	@mkdir -p $(dir $@) $(BUILD)/user/binutils
	@printf "  %-8s %s\n" "CXX" "$<"
	@$(CXX) $(USER_CXXFLAGS) -c $< -o $(BUILD)/user/binutils/$*.o
	@printf "  %-8s %s\n" "BIN" "$@"
	@$(LD) -T libsys/user.ld --oformat=binary -nostdlib $(BUILD)/user/binutils/$*.o -o $@

$(BUILD)/initrd.tar: $(USER_BINS) $(INITRD_FILES)
	@mkdir -p $(BUILD) initrd/bin initrd/etc/man
	@if [ ! -f initrd/readme.txt ]; then \
		printf "Welcome to Testix VFS!\nType 'man' to see installed manual pages.\n" > initrd/readme.txt; \
	fi
	@if [ ! -f initrd/demo.kl ]; then \
		printf "var os = Testix\nprint \"Hello from \" $$os \" VFS!\"\n" > initrd/demo.kl; \
	fi
	@printf "  %-8s %s\n" "TAR" "$@"
	@tar --format=ustar -cf $@ -C initrd .

# --- Сборка объектных файлов ядра ---
$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@printf "  %-8s %s\n" "CXX" "$<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@printf "  %-8s %s\n" "CC" "$<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	@printf "  %-8s %s\n" "AS" "$<"
	@$(NASM) -f elf64 $< -o $@

# --- Линковка ядра ---
$(KERNEL): $(OBJS) kernel/linker.ld
	@printf "  %-8s %s\n" "LD" "$@"
	@$(CXX) $(LDFLAGS) -o $@ $(OBJS)

# --- Создание ISO-образа через Limine ---
$(TARGET): $(KERNEL) limine.conf $(LIMINE_DIR)/limine-bios.sys $(BUILD)/initrd.tar
	@rm -rf $(ISO_ROOT)
	@mkdir -p $(ISO_ROOT)/boot $(ISO_ROOT)/EFI/BOOT
	@cp $(KERNEL) $(ISO_ROOT)/boot/kernel.elf
	@cp limine.conf $(ISO_ROOT)/limine.conf
	@cp limine.conf $(ISO_ROOT)/boot/limine.conf
	@cp $(LIMINE_DIR)/limine-bios.sys $(ISO_ROOT)/boot/
	@cp $(LIMINE_DIR)/limine-bios-cd.bin $(ISO_ROOT)/boot/
	@cp $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_ROOT)/boot/
	@cp $(LIMINE_DIR)/BOOTX64.EFI $(ISO_ROOT)/EFI/BOOT/
	@cp $(BUILD)/initrd.tar $(ISO_ROOT)/boot/initrd.tar
	@printf "  %-8s %s\n" "ISO" "$@"
	@xorriso -as mkisofs \
		-o $@ \
		-b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		$(ISO_ROOT) 2>/dev/null
	@$(LIMINE_BIN) bios-install $@ 2>/dev/null
	@printf "  %-8s %s\n" "READY" "$@"

# --- Запуск в QEMU ---
run-efi: $(TARGET)
	@mkdir -p $(BUILD)
	@cp $(OVMF_VARS) $(BUILD)/OVMF_VARS.fd
	@cp $(OVMF_CODE) $(BUILD)/OVMF_CODE.fd
	@printf "  %-8s %s\n" "QEMU" "UEFI ($<)"
	@qemu-system-x86_64 \
		-machine q35 \
		-global ICH9-LPC.disable_s3=1 \
		-drive if=pflash,format=raw,readonly=on,file=$(BUILD)/OVMF_CODE.fd \
		-drive if=pflash,format=raw,file=$(BUILD)/OVMF_VARS.fd \
		-boot d -cdrom $(TARGET) \
		-serial stdio -net none

run-bios: $(TARGET)
	@printf "  %-8s %s\n" "QEMU" "BIOS ($<)"
	@qemu-system-x86_64 -boot d -cdrom $(TARGET) -serial stdio

clean:
	@printf "  %-8s %s\n" "CLEAN" "build/ $(TARGET) initrd/bin/"
	@rm -rf $(BUILD) $(TARGET) initrd/bin/*

context:
	@mkdir -p $(BUILD)
	@python3 -c 'import pathlib; \
	exts={".hpp",".h",".cpp",".c",".asm",".ld",".conf"}; \
	skip={"build",".git",".vscode","font8x8.cpp","limine.hpp","limine.h"}; \
	files=[p for p in sorted(pathlib.Path(".").rglob("*")) if p.is_file() and p.suffix in exts and not any(s in p.parts for s in skip)]; \
	out="\n\n".join(f"=== FILE: {p} ===\n" + p.read_text(errors="ignore") for p in files); \
	pathlib.Path("project_context.txt").write_text(out); \
	print(f"Packed {len(files)} files (~{len(out)//4} tokens) -> project_context.txt")'

.PHONY: all clean run-bios run-efi context
