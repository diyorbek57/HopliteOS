#!/bin/bash

echo "Building Hoplite OS..."

# Assemble bootloader
nasm -f elf32 boot.asm -o boot.o

# Compile kernel
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

#VGA
gcc -m32  -c vga.c -o vga.o

#Keyboard
gcc -m32  -c keyboard.c -o keyboard.o

# Link
gcc -m32 -T linker.ld -o kernel.bin boot.o kernel.o vga.o keyboard.o -ffreestanding -nostdlib -lgcc


# Create ISO structure
mkdir -p isodir/boot/grub
cp kernel.bin isodir/boot/kernel.bin
cp boot/grub/grub.cfg isodir/boot/grub/grub.cfg

# Create ISO
grub-mkrescue -o hoplite-os-x86.iso isodir

echo "Build complete! ISO created: hoplite-os-x86.iso"
