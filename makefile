# --- Настройки кросс-компилятора ---
CROSS    ?=
CC       := $(CROSS)gcc
CXX      := $(CROSS)g++
LD       := $(CROSS)ld
AS       := nasm
OBJCOPY  := $(CROSS)objcopy

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
USER_BINS := $(BUILD)/user/prog_a.bin $(BUILD)/user/prog_b.bin

# --- Исходные файлы ядра ---
C_SRCS   := 
CXX_SRCS := kernel/main.cpp \
	kernel/panic/panic.cpp \
	kernel/mm/pmm.cpp \
	kernel/syscall/syscall.cpp \
	kernel/task/task.cpp \
	kernel/timer/pit.cpp \
	kernel/loader/loader.cpp \
	arch/x86_64/interrupts/gdt.cpp \
	arch/x86_64/interrupts/idt.cpp \
	arch/x86_64/interrupts/pic.cpp \
	arch/x86_64/interrupts/tss.cpp \
	arch/x86_64/paging/paging.cpp \
	drivers/keyboard/keyboard.cpp \
	drivers/serial/serial.cpp \
	drivers/video/vga/vga.cpp \
	drivers/video/framebuffer/framebuffer.cpp \
	drivers/video/font/font8x8.cpp \
	drivers/video/console/console.cpp \
	kernlang/kernlang.cpp \
	shell/shell.cpp

ASM_SRCS := arch/x86_64/entry.asm \
	arch/x86_64/interrupts/interrupts.asm \
	user/user_bins.asm

C_OBJS   := $(patsubst %.c,  $(BUILD)/%.o, $(C_SRCS))
CXX_OBJS := $(patsubst %.cpp,$(BUILD)/%.o, $(CXX_SRCS))
ASM_OBJS := $(patsubst %.asm,$(BUILD)/%.asm.o, $(ASM_SRCS))

OBJS := $(ASM_OBJS) $(C_OBJS) $(CXX_OBJS)

# --- Основные цели ---
all: $(BUILD)/testix.iso

# --- Сборка user-программ в плоские бинарники ---
$(BUILD)/user/%.bin: user/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(USER_CXXFLAGS) -c $< -o $(BUILD)/user/$*.o
	$(LD) -Ttext=0x400000 --oformat=binary -nostdlib $(BUILD)/user/$*.o -o $@

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
	$(AS) -f elf64 $< -o $@

# --- Линковка ядра ---
$(KERNEL): $(OBJS) linker.ld
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

# --- Создание ISO-образа через Limine ---
$(BUILD)/testix.iso: $(KERNEL) limine.conf
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
run-efi: $(BUILD)/testix.iso
	cp /usr/share/OVMF/OVMF_VARS_4M.fd $(BUILD)/OVMF_VARS.fd
	qemu-system-x86_64 \
		-machine q35 \
		-global ICH9-LPC.disable_s3=1 \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
		-drive if=pflash,format=raw,file=$(BUILD)/OVMF_VARS.fd \
		-boot d -cdrom $(BUILD)/testix.iso \
		-serial stdio -net none

run-bios: $(BUILD)/testix.iso
	qemu-system-x86_64 -boot d -cdrom $(BUILD)/testix.iso -serial stdio

run: $(BUILD)/testix.iso
	qemu-system-x86_64 -cdrom $(BUILD)/testix.iso -serial stdio

clean:
	rm -rf $(BUILD)

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
