#!/bin/bash

echo "Building Hoplite OS..."

# Assemble bootloader
nasm -f elf32 boot.asm -o boot.o

# Compile kernel
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# Link
gcc -m32 -T linker.ld -o kernel.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

# Create ISO structure
mkdir -p isodir/boot/grub
cp kernel.bin isodir/boot/kernel.bin
cp boot/grub/grub.cfg isodir/boot/grub/grub.cfg

# Create ISO
grub-mkrescue -o hoplite-os-x86.iso isodir

echo "Build complete! ISO created: hoplite-os-x86.iso"
