![Hoplite OS Logo](https://github.com/diyorbek57/HopliteOS/blob/master/Hoplite%20OS%20(1280×640).png?raw=true)
# 🛡️ Hoplite OS
**Hoplite OS** is an educational 32-bit operating system written in **C** and **Assembly**.  
It is designed to demonstrate how low-level systems work: bootloading, kernel development, memory management, drivers, interrupts, and hardware interaction.

The project is simple, clean, and easy to extend — perfect for learning OS development from scratch.

---

## ✨ Features

- 🚀 Multiboot-compatible loading with **GRUB**
- 🖥️ Basic **VGA text console** (printing, scrolling, color output)
- ⌨️ Simple **keyboard driver** (scancode set 1)
- 🧮 Minimal **memory allocator** (`kmalloc` / `kfree`)
- 📁 In-memory **mini file system** (create, read, delete)
- 🕒 Basic timer tick counter and uptime calculation
- 🧱 Custom build and linking process

Hoplite OS is intended as a foundation for deeper systems work such as IDT, interrupts, paging, drivers, multitasking, and more.

---

## 📁 Project Structure

```

root/
├── boot.asm        # Bootstrap code + Multiboot header
├── kernel.c        # Main OS kernel
├── linker.ld       # Linker script for kernel layout
├── grub.cfg        # GRUB bootloader configuration
├── build.sh        # Build script (creates bootable ISO)
└── README.md

````

---

## 🔧 Requirements

You need the following tools installed:

### On Linux (Debian/Ubuntu):

```bash
sudo apt install build-essential nasm xorriso grub-pc-bin grub-common gcc-multilib
````

### Tools required:

* **nasm** — assembler
* **gcc (with -m32 support)** — for building 32-bit binaries
* **xorriso** — ISO creation tool
* **grub-mkrescue** — builds a GRUB bootable image

---

## 🏗️ Build Instructions

Run the build script from the project root:

```bash
chmod +x build.sh
./build.sh
```

After a successful build, you will get:

```
hoplite-os-x86.iso
```

This ISO is fully bootable.

---

## ▶️ Running Hoplite OS

### Run using QEMU (recommended):

```bash
qemu-system-i386 -cdrom hoplite-os-x86.iso
```

### Run using VirtualBox:

1. Create a new VM.
2. OS Type: **Other**
3. Version: **Other/Unknown (32-bit)**
4. Attach *hoplite-os-x86.iso* as a virtual CD-ROM.
5. Start the VM.

### Run using VMware:

1. Create a new Virtual Machine → Custom.
2. Choose **hoplite-os-x86.iso** as installation media.
3. Set OS type to **Other (32-bit)**.

---

## 📌 Notes for Developers

* The kernel is built freestanding (`-ffreestanding`) and without any standard library.
* The stack and Multiboot environment are initialized in `boot.asm`.
* VGA output, keyboard input, and a simple memory allocator are implemented for early development.

### Recommended next steps (if you want to expand the OS):

* Implement **GDT** and **IDT**
* Add real **interrupt handlers (ISR/IRQ)**
* Enable **Programmable Interrupt Timer (PIT)**
* Implement **paging** and virtual memory
* Create a **task scheduler**
* Add **drivers** for more hardware

---

## 🤝 Contributing

Pull requests are welcome!
You can contribute by improving:

* Code structure
* Drivers and subsystems
* Documentation
* Testing
* Build tools

---

## 📜 License

This project is licensed under the **MIT License** — feel free to use, modify, and distribute.

---

## 🛡️ Author
Hamdamov Diyorbek 

Email: hamdamovd@proton.me

Hoplite OS was created as a learning-oriented operating system project focused on low-level x86 architecture.

```

Just tell me!
```
