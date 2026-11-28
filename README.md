<div align="center">

# Testix OS

**A 64-bit x86_64 hobby operating system written from scratch in C++20 and NASM**

[![Language](https://shields.io)](https://isocpp.org)
[![Architecture](https://shields.io)](https://wikipedia.org)
[![Bootloader](https://shields.io)](https://limine-bootloader.org)
[![License](https://shields.io)](LICENSE)
[![Build](https://github.com)](https://github.com)

**[English](README.md)** | **[Русский](README.ru.md)**

</div>

The system supports booting in both **UEFI** and **Legacy BIOS** modes, preemptive multitasking with address space isolation (Paging + HHDM), system calls (`int 0x80`), an interactive userspace shell (**Ring 3**), and a built-in scripting language called **kernlang**.

---

## Features & Architecture

* **Boot & Memory Management:**
  * **Limine** protocol (Higher Half Kernel mapped at virtual address `0xffffffff80000000`, physical load address `0x200000`).
  * **PMM (Physical Memory Manager):** Bitmap page frame allocator (4 KB pages) that parses the UEFI/Limine memory map and reserves system regions.
  * **VMM (Virtual Memory Manager):** 4-level paging (`PML4` → `PDPT` → `PD` → `PT`), isolated address spaces per Ring 3 process, and HHDM (Higher Half Direct Map) support.
* **Interrupts & Protection:**
  * Configured **GDT**, **IDT** (CPU exceptions `0..31`, hardware interrupts `IRQ 0..15`, software interrupt gate `0x80` with `DPL=3`), and **TSS** (kernel stack `rsp0` switching on Ring 3 → Ring 0 transitions).
  * Programmable Interval Timer (**PIT**, 100 Hz) and **PIC 8259** interrupt controller.
* **Multitasking & Userspace (Ring 3):**
  * Preemptive Round-Robin scheduler with cooperative yielding (`sys_yield`).
  * Flat binary process loader (`kernel/loader`) mapping user code at `0x400000` and user stack at `0x7FFF0000`.
  * Interactive shell (`user/shell.cpp`) and script interpreter (`user/kernlang.hpp`) running completely in **Ring 3**.
* **Drivers:**
  * Linear **Framebuffer** (32/24/16 bpp support, software boundary clipping) and an adaptive console with dynamic `8x8` font scaling based on monitor resolution.
  * **PS/2** keyboard controller driver with `Shift` key support and a ring buffer.
  * **COM1 (`0x3F8`)** serial port driver for debug logging.

---

## Project Structure

```text
Testix/
├── arch/x86_64/            # Architecture-specific code (entry.asm, GDT, IDT, TSS, PIC, Paging)
├── drivers/                # Modular device drivers
│   ├── keyboard/           # PS/2 keyboard driver
│   ├── serial/             # COM1 (0x3F8) serial driver
│   └── video/              # Framebuffer, adaptive console, 8x8 font, and VGA
├── include/                # Kernel header files
├── kernel/                 # Core OS kernel
│   ├── loader/             # Userspace process loader (Ring 3)
│   ├── mm/                 # Physical Memory Manager (PMM)
│   ├── panic/              # Kernel Panic handler (Serial + Framebuffer)
│   ├── syscall/            # System call dispatcher (int 0x80)
│   ├── task/               # Task scheduler and context switching
│   ├── timer/              # PIT timer driver
│   └── main.cpp            # Kernel entry point kmain()
├── user/                   # Userspace programs (Ring 3)
│   ├── kernlang.hpp        # Built-in kernlang script interpreter
│   ├── prog_a.cpp          # Test userspace task A
│   ├── prog_b.cpp          # Test userspace task B
│   ├── shell.cpp           # Userspace command shell
│   ├── syscall.hpp         # System call wrappers for Ring 3
│   ├── user_bins.asm       # Embeds user binaries into the kernel image
│   └── user.ld             # Linker script for Ring 3 binaries
├── limine.conf             # Limine bootloader configuration
├── linker.ld               # Kernel linker script (Higher Half)
└── makefile                # Cross-platform build system (Linux / macOS)
```

---

## System Calls (`int 0x80` ABI)

The system call number is passed in `RAX`, the first argument in `RDI`, and the return value is placed in `RAX`.

| `RAX` | Name | Arguments | Description |
| :---: | :--- | :--- | :--- |
| `1` | `sys_putchar` | `RDI = char c` | Print a single character to the console |
| `2` | `sys_yield` | — | Voluntarily yield the remaining time slice to the next task |
| `3` | `sys_exit` | — | Terminate the current userspace task |
| `4` | `sys_getchar` | — | Non-blocking read of a character from the keyboard buffer (`0` if empty) |
| `5` | `sys_clear` | — | Clear the console screen |
| `6` | `sys_spawn` | `RDI = uint64_t id` | Spawn a built-in user program (`1` — `prog_a`, `2` — `prog_b`) |
| `7` | `sys_panic` | — | Trigger a test Kernel Panic from Ring 3 |
| `8` | `sys_active_tasks` | — | Return the number of active Ring 3 tasks |

---

## Shell Commands (`Ring 3 Shell`)

After initialization, the kernel launches `user/shell.cpp` in user mode:

* `help` — Show available commands.
* `clear` — Clear the screen.
* `uname` — Display OS name and version.
* `ring3` — Spawn two concurrent user processes (`prog_a` and `prog_b`) to demonstrate preemptive and cooperative multitasking.
* `edit` — Enter the line-by-line script editor (`.save` to save, `.cancel` to exit).
* `list` — Display the currently saved script.
* `run` — Execute the saved script using **kernlang**.
* `del` — Delete the saved script from the buffer.
* `panictest` — Test the Kernel Panic screen.

---

## `kernlang` Scripting Language

The built-in shell interpreter supports variables, text output, and delays:

```text
# Example kernlang script
var os = Testix
var ver = 0.1
print "Running " os " v" ver
sleep 5
print "Done!"
```

**Syntax Rules:**
* Spaces around `=` are required (`var NAME = VALUE`).
* Prefix variable names with `$` (without quotes) to expand them in `print` or `var` (`print $os`).
* Wrap strings containing spaces in double quotes (`"Hello world"`).

---

## Building & Running

The `makefile` automatically detects your Linux distribution or macOS, locates the UEFI firmware (`OVMF`), and clones/builds `limine` into `build/limine` if no system package is installed.

### Dependencies

* **Arch Linux:**
  ```bash
  sudo pacman -S base-devel gcc nasm xorriso limine edk2-ovmf qemu-full
  ```
* **Ubuntu / Debian / Linux Mint:**
  ```bash
  sudo apt update
  sudo apt install build-essential g++ nasm xorriso ovmf qemu-system-x86 git
  ```
* **Fedora:**
  ```bash
  sudo dnf install gcc-c++ make nasm xorriso edk2-ovmf qemu-system-x86 git
  ```
* **openSUSE (Tumbleweed / Leap):**
  ```bash
  sudo zypper install gcc-c++ make nasm xorriso qemu-ovmf-x86_64 qemu-x86 git
  ```
* **macOS (Homebrew):**
  ```bash
  brew install x86_64-elf-gcc x86_64-elf-binutils nasm xorriso limine qemu
  ```

### Build Commands

```bash
# Build the bootable testix.iso image
make

# Run in QEMU using UEFI (recommended)
make run-efi

# Run in QEMU using Legacy BIOS
make run-bios

# Clean build artifacts
make clean
```
